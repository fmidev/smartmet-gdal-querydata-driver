#include "QDMultidim.h"
#include "QDDataset.h"
#include "QDStrings.h"

#include <gdal_priv.h>
#include <gdal_multidim.h>

#include <newbase/NFmiArea.h>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiGlobals.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiParam.h>
#include <newbase/NFmiParamBag.h>
#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiRect.h>

#include <ogr_spatialref.h>

#include <fmt/format.h>

#include <cctype>
#include <ctime>
#include <mutex>
#include <unordered_set>

namespace SmartMet
{
namespace GdalQueryData
{
namespace
{
// Map Finnish/Scandinavian accented Latin-1 chars to their ASCII equivalents
// before sanitisation, so "Lämpötila" → "Lampotila" instead of "L__mp__tila".
// Operates on the raw Latin-1 byte (not UTF-8).
char transliterateLatin1(unsigned char c)
{
  switch (c)
  {
    case 0xc4: case 0xe4: return 'a';  // Ä, ä
    case 0xc5: case 0xe5: return 'a';  // Å, å
    case 0xd6: case 0xf6: return 'o';  // Ö, ö
    case 0xdc: case 0xfc: return 'u';  // Ü, ü
    case 0xc9: case 0xe9: return 'e';  // É, é
    case 0xc0: case 0xe0: return 'a';  // À, à
    case 0xc8: case 0xe8: return 'e';  // È, è
    case 0xd1: case 0xf1: return 'n';  // Ñ, ñ
    default:              return '_';
  }
}

// Make a string usable as a netCDF/Zarr/HDF5 variable identifier. Input is
// Latin-1 (the format newbase uses internally). Output is ASCII letters, digits,
// and underscore; first char must not be a digit.
std::string sanitizeName(std::string_view latin1)
{
  std::string out;
  out.reserve(latin1.size());
  for (size_t i = 0; i < latin1.size(); ++i)
  {
    unsigned char c = static_cast<unsigned char>(latin1[i]);
    char keep;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
      keep = static_cast<char>(c);
    else if (c >= '0' && c <= '9')
      keep = (i > 0) ? static_cast<char>(c) : '_';
    else if (c >= 0x80)
      keep = transliterateLatin1(c);
    else
      keep = '_';

    // Collapse runs of underscores.
    if (keep == '_' && !out.empty() && out.back() == '_') continue;
    out.push_back(keep);
  }
  // Trim trailing underscore.
  while (!out.empty() && out.back() == '_') out.pop_back();
  if (out.empty()) out = "var";
  return out;
}

// Make `name` unique within `seen` by appending _2, _3, ...
std::string uniquify(std::string name, std::unordered_set<std::string>& seen)
{
  if (seen.insert(name).second) return name;
  for (int i = 2;; ++i)
  {
    std::string candidate = name + "_" + std::to_string(i);
    if (seen.insert(candidate).second) return candidate;
  }
}

// Shared cells of work that a data array needs from the parent dataset.
struct ArraySpec
{
  unsigned long paramIdx = 0;
  unsigned long subParamId = 0;  // 0 if the data array is the parent param itself
  std::string longName;
  long paramId = 0;
};

// ---------- CF attribute helper ---------------------------------------------
//
// Read-only string-valued GDALAttribute used for CF metadata (axis,
// standard_name, etc.) on coordinate variables and data arrays. Without
// these, gdalmdimtranslate writes a NetCDF that CF-aware tools (Panoply,
// NetCDF-Java, ncview) can't parse correctly — the dimension assignments
// in the consumer end up wrong (time → x, etc.).
class CFStringAttribute : public GDALAttribute
{
 public:
  static std::shared_ptr<CFStringAttribute> Create(const std::string& parent,
                                                   const std::string& name,
                                                   std::string value)
  {
    auto p = std::shared_ptr<CFStringAttribute>(
        new CFStringAttribute(parent, name, std::move(value)));
    p->SetSelf(p);
    return p;
  }

 private:
  CFStringAttribute(const std::string& parent, const std::string& name, std::string value)
      : GDALAbstractMDArray(parent, name),
        GDALAttribute(parent, name),
        itsValue(std::move(value)),
        itsType(GDALExtendedDataType::CreateString())
  {
  }

 public:
  const std::vector<std::shared_ptr<GDALDimension>>& GetDimensions() const override
  {
    return itsEmptyDims;
  }
  const GDALExtendedDataType& GetDataType() const override { return itsType; }

 protected:
  bool IRead(const GUInt64* /*arrayStartIdx*/, const size_t* /*count*/,
             const GInt64* /*arrayStep*/, const GPtrDiff_t* /*bufferStride*/,
             const GDALExtendedDataType& bufferDataType, void* pDstBuffer) const override
  {
    char* tmp = CPLStrdup(itsValue.c_str());
    GDALExtendedDataType::CopyValue(&tmp, itsType, pDstBuffer, bufferDataType);
    CPLFree(tmp);
    return true;
  }

 private:
  std::string itsValue;
  GDALExtendedDataType itsType;
  std::vector<std::shared_ptr<GDALDimension>> itsEmptyDims;
};

// ---------- coordinate variables --------------------------------------------

class RegularDoubleArray : public GDALMDArray
{
 public:
  static std::shared_ptr<RegularDoubleArray> Create(const std::string& parent,
                                                    const std::string& name,
                                                    std::shared_ptr<GDALDimension> dim,
                                                    double start, double step)
  {
    auto p = std::shared_ptr<RegularDoubleArray>(
        new RegularDoubleArray(parent, name, std::move(dim), start, step));
    p->SetSelf(p);
    return p;
  }

  void addAttr(const std::string& name, const std::string& value)
  {
    itsAttrs.push_back(CFStringAttribute::Create(GetFullName(), name, value));
  }

 private:
  RegularDoubleArray(const std::string& parent, const std::string& name,
                     std::shared_ptr<GDALDimension> dim, double start, double step)
      : GDALAbstractMDArray(parent, name),
        GDALMDArray(parent, name),
        itsDim(std::move(dim)),
        itsDims{itsDim},
        itsType(GDALExtendedDataType::Create(GDT_Float64)),
        itsStart(start),
        itsStep(step)
  {
  }

 public:

  bool IsWritable() const override { return false; }
  const std::string& GetFilename() const override { return itsEmpty; }
  const std::vector<std::shared_ptr<GDALDimension>>& GetDimensions() const override
  {
    return itsDims;
  }
  const GDALExtendedDataType& GetDataType() const override { return itsType; }
  std::shared_ptr<GDALAttribute> GetAttribute(const std::string& osName) const override
  {
    for (const auto& a : itsAttrs)
      if (a->GetName() == osName) return a;
    return nullptr;
  }
  std::vector<std::shared_ptr<GDALAttribute>> GetAttributes(CSLConstList) const override
  {
    return itsAttrs;
  }

 protected:
  bool IRead(const GUInt64* arrayStartIdx, const size_t* count, const GInt64* arrayStep,
             const GPtrDiff_t* bufferStride, const GDALExtendedDataType& bufferDataType,
             void* pDstBuffer) const override
  {
    const size_t n = count[0];
    const GInt64 step = arrayStep[0];
    const GUInt64 start = arrayStartIdx[0];
    std::vector<double> tmp(n);
    for (size_t i = 0; i < n; ++i)
      tmp[i] = itsStart + (start + step * static_cast<GInt64>(i)) * itsStep;

    auto* dst = static_cast<GByte*>(pDstBuffer);
    const GPtrDiff_t stride = bufferStride[0] * static_cast<GPtrDiff_t>(bufferDataType.GetSize());
    for (size_t i = 0; i < n; ++i)
    {
      GDALExtendedDataType::CopyValue(&tmp[i], itsType, dst + i * stride, bufferDataType);
    }
    return true;
  }

 private:
  std::shared_ptr<GDALDimension> itsDim;
  std::vector<std::shared_ptr<GDALDimension>> itsDims;
  GDALExtendedDataType itsType;
  double itsStart;
  double itsStep;
  std::string itsEmpty;
  std::vector<std::shared_ptr<GDALAttribute>> itsAttrs;
};

class FloatVectorArray : public GDALMDArray
{
 public:
  static std::shared_ptr<FloatVectorArray> Create(const std::string& parent,
                                                  const std::string& name,
                                                  std::shared_ptr<GDALDimension> dim,
                                                  std::vector<float> values)
  {
    auto p = std::shared_ptr<FloatVectorArray>(
        new FloatVectorArray(parent, name, std::move(dim), std::move(values)));
    p->SetSelf(p);
    return p;
  }

  void addAttr(const std::string& name, const std::string& value)
  {
    itsAttrs.push_back(CFStringAttribute::Create(GetFullName(), name, value));
  }

 private:
  FloatVectorArray(const std::string& parent, const std::string& name,
                   std::shared_ptr<GDALDimension> dim, std::vector<float> values)
      : GDALAbstractMDArray(parent, name),
        GDALMDArray(parent, name),
        itsDim(std::move(dim)),
        itsDims{itsDim},
        itsType(GDALExtendedDataType::Create(GDT_Float32)),
        itsValues(std::move(values))
  {
  }

 public:

  bool IsWritable() const override { return false; }
  const std::string& GetFilename() const override { return itsEmpty; }
  const std::vector<std::shared_ptr<GDALDimension>>& GetDimensions() const override
  {
    return itsDims;
  }
  const GDALExtendedDataType& GetDataType() const override { return itsType; }
  std::shared_ptr<GDALAttribute> GetAttribute(const std::string& osName) const override
  {
    for (const auto& a : itsAttrs)
      if (a->GetName() == osName) return a;
    return nullptr;
  }
  std::vector<std::shared_ptr<GDALAttribute>> GetAttributes(CSLConstList) const override
  {
    return itsAttrs;
  }

 protected:
  bool IRead(const GUInt64* arrayStartIdx, const size_t* count, const GInt64* arrayStep,
             const GPtrDiff_t* bufferStride, const GDALExtendedDataType& bufferDataType,
             void* pDstBuffer) const override
  {
    auto* dst = static_cast<GByte*>(pDstBuffer);
    const GPtrDiff_t stride = bufferStride[0] * static_cast<GPtrDiff_t>(bufferDataType.GetSize());
    for (size_t i = 0; i < count[0]; ++i)
    {
      const size_t srcIdx = arrayStartIdx[0] + arrayStep[0] * static_cast<GInt64>(i);
      if (srcIdx >= itsValues.size()) return false;
      GDALExtendedDataType::CopyValue(&itsValues[srcIdx], itsType, dst + i * stride,
                                      bufferDataType);
    }
    return true;
  }

 private:
  std::shared_ptr<GDALDimension> itsDim;
  std::vector<std::shared_ptr<GDALDimension>> itsDims;
  GDALExtendedDataType itsType;
  std::vector<float> itsValues;
  std::string itsEmpty;
  std::vector<std::shared_ptr<GDALAttribute>> itsAttrs;
};

// Time as int64 seconds since UNIX epoch — the convention xarray/CF understand.
class Int64VectorArray : public GDALMDArray
{
 public:
  static std::shared_ptr<Int64VectorArray> Create(const std::string& parent,
                                                  const std::string& name,
                                                  std::shared_ptr<GDALDimension> dim,
                                                  std::vector<int64_t> values,
                                                  std::string unitAttr)
  {
    auto p = std::shared_ptr<Int64VectorArray>(new Int64VectorArray(
        parent, name, std::move(dim), std::move(values), std::move(unitAttr)));
    p->SetSelf(p);
    return p;
  }

  void addAttr(const std::string& name, const std::string& value)
  {
    itsAttrs.push_back(CFStringAttribute::Create(GetFullName(), name, value));
  }

 private:
  Int64VectorArray(const std::string& parent, const std::string& name,
                   std::shared_ptr<GDALDimension> dim, std::vector<int64_t> values,
                   std::string unitAttr)
      : GDALAbstractMDArray(parent, name),
        GDALMDArray(parent, name),
        itsDim(std::move(dim)),
        itsDims{itsDim},
        itsType(GDALExtendedDataType::Create(GDT_Int64)),
        itsValues(std::move(values)),
        itsUnit(std::move(unitAttr))
  {
  }

 public:

  bool IsWritable() const override { return false; }
  const std::string& GetUnit() const override { return itsUnit; }
  const std::string& GetFilename() const override { return itsEmpty; }
  const std::vector<std::shared_ptr<GDALDimension>>& GetDimensions() const override
  {
    return itsDims;
  }
  const GDALExtendedDataType& GetDataType() const override { return itsType; }
  std::shared_ptr<GDALAttribute> GetAttribute(const std::string& osName) const override
  {
    for (const auto& a : itsAttrs)
      if (a->GetName() == osName) return a;
    return nullptr;
  }
  std::vector<std::shared_ptr<GDALAttribute>> GetAttributes(CSLConstList) const override
  {
    return itsAttrs;
  }

 protected:
  bool IRead(const GUInt64* arrayStartIdx, const size_t* count, const GInt64* arrayStep,
             const GPtrDiff_t* bufferStride, const GDALExtendedDataType& bufferDataType,
             void* pDstBuffer) const override
  {
    auto* dst = static_cast<GByte*>(pDstBuffer);
    const GPtrDiff_t stride = bufferStride[0] * static_cast<GPtrDiff_t>(bufferDataType.GetSize());
    for (size_t i = 0; i < count[0]; ++i)
    {
      const size_t srcIdx = arrayStartIdx[0] + arrayStep[0] * static_cast<GInt64>(i);
      if (srcIdx >= itsValues.size()) return false;
      GDALExtendedDataType::CopyValue(&itsValues[srcIdx], itsType, dst + i * stride,
                                      bufferDataType);
    }
    return true;
  }

 private:
  std::shared_ptr<GDALDimension> itsDim;
  std::vector<std::shared_ptr<GDALDimension>> itsDims;
  GDALExtendedDataType itsType;
  std::vector<int64_t> itsValues;
  std::string itsUnit;
  std::string itsEmpty;
  std::vector<std::shared_ptr<GDALAttribute>> itsAttrs;
};

// ---------- data arrays ------------------------------------------------------

class QDDataMDArray : public GDALMDArray
{
 public:
  static std::shared_ptr<QDDataMDArray> Create(
      QDDataset& ds, const std::string& parent, const std::string& name, ArraySpec spec,
      std::vector<std::shared_ptr<GDALDimension>> dims,
      std::shared_ptr<OGRSpatialReference> srs, bool flipY, unsigned long gridY)
  {
    auto p = std::shared_ptr<QDDataMDArray>(new QDDataMDArray(
        ds, parent, name, std::move(spec), std::move(dims), std::move(srs), flipY, gridY));
    p->SetSelf(p);
    return p;
  }

 private:
  QDDataMDArray(QDDataset& ds, const std::string& parent, const std::string& name,
                ArraySpec spec, std::vector<std::shared_ptr<GDALDimension>> dims,
                std::shared_ptr<OGRSpatialReference> srs, bool flipY, unsigned long gridY)
      : GDALAbstractMDArray(parent, name),
        GDALMDArray(parent, name),
        itsDataset(ds),
        itsSpec(std::move(spec)),
        itsDims(std::move(dims)),
        itsType(GDALExtendedDataType::Create(GDT_Float32)),
        itsSRS(std::move(srs)),
        itsFlipY(flipY),
        itsGridY(gridY)
  {
  }

 public:

  bool IsWritable() const override { return false; }
  const std::string& GetFilename() const override { return itsEmpty; }
  const std::vector<std::shared_ptr<GDALDimension>>& GetDimensions() const override
  {
    return itsDims;
  }
  const GDALExtendedDataType& GetDataType() const override { return itsType; }

  std::shared_ptr<OGRSpatialReference> GetSpatialRef() const override { return itsSRS; }

  const void* GetRawNoDataValue() const override
  {
    static const float kNoData = static_cast<float>(kFloatMissing);
    return &kNoData;
  }

 protected:
  bool IRead(const GUInt64* arrayStartIdx, const size_t* count, const GInt64* arrayStep,
             const GPtrDiff_t* bufferStride, const GDALExtendedDataType& bufferDataType,
             void* pDstBuffer) const override
  {
    // itsDims is always (time, level, y, x). Pull the per-dim parameters out
    // by name so the indexing stays readable.
    const size_t nT = count[0];
    const size_t nL = count[1];
    const size_t nY = count[2];
    const size_t nX = count[3];
    const GUInt64 t0 = arrayStartIdx[0];
    const GUInt64 l0 = arrayStartIdx[1];
    const GUInt64 y0 = arrayStartIdx[2];
    const GUInt64 x0 = arrayStartIdx[3];
    const GInt64 tStep = arrayStep[0];
    const GInt64 lStep = arrayStep[1];
    const GInt64 yStep = arrayStep[2];
    const GInt64 xStep = arrayStep[3];

    const GPtrDiff_t elemSize = static_cast<GPtrDiff_t>(bufferDataType.GetSize());

    std::lock_guard<std::mutex> lock(itsDataset.mutex());
    auto& qi = itsDataset.info();

    bool ok;
    if (itsSpec.subParamId != 0)
      ok = qi.Param(static_cast<FmiParameterName>(itsSpec.subParamId)) && qi.IsSubParamUsed();
    else
      ok = qi.ParamIndex(itsSpec.paramIdx);
    if (!ok) return false;


    const unsigned long gridY = itsGridY;

    // Iterate slowest dim first so each (time, level) plane is read once.
    for (size_t it = 0; it < nT; ++it)
    {
      const unsigned long timeIdx = static_cast<unsigned long>(t0 + tStep * static_cast<GInt64>(it));
      if (!qi.TimeIndex(timeIdx)) return false;

      for (size_t il = 0; il < nL; ++il)
      {
        const unsigned long levelIdx =
            static_cast<unsigned long>(l0 + lStep * static_cast<GInt64>(il));
        if (!qi.LevelIndex(levelIdx)) return false;

        // One full 2D plane per (time, level). Then strided copy into the dst.
        const NFmiDataMatrix<float> matrix = qi.Values();

        for (size_t iy = 0; iy < nY; ++iy)
        {
          const unsigned long yIdx =
              static_cast<unsigned long>(y0 + yStep * static_cast<GInt64>(iy));
          // GDAL row 0 is the top; QD bottom-left grids store row 0 at the bottom.
          const unsigned long qdY = itsFlipY ? (gridY - 1 - yIdx) : yIdx;

          for (size_t ix = 0; ix < nX; ++ix)
          {
            const unsigned long xIdx =
                static_cast<unsigned long>(x0 + xStep * static_cast<GInt64>(ix));
            const float val = matrix[xIdx][qdY];

            const GPtrDiff_t off = (bufferStride[0] * static_cast<GPtrDiff_t>(it) +
                                    bufferStride[1] * static_cast<GPtrDiff_t>(il) +
                                    bufferStride[2] * static_cast<GPtrDiff_t>(iy) +
                                    bufferStride[3] * static_cast<GPtrDiff_t>(ix)) *
                                   elemSize;
            GByte* dst = static_cast<GByte*>(pDstBuffer) + off;
            GDALExtendedDataType::CopyValue(&val, itsType, dst, bufferDataType);
          }
        }
      }
    }
    return true;
  }

 private:
  QDDataset& itsDataset;
  ArraySpec itsSpec;
  std::vector<std::shared_ptr<GDALDimension>> itsDims;
  GDALExtendedDataType itsType;
  std::shared_ptr<OGRSpatialReference> itsSRS;
  bool itsFlipY;
  unsigned long itsGridY;
  std::string itsEmpty;
};

// ---------- root group -------------------------------------------------------

class QDRootGroup : public GDALGroup
{
 public:
  QDRootGroup(const std::string& name, QDDataset& ds)
      : GDALGroup("/", name), itsDataset(ds)
  {
  }

  std::vector<std::string> GetMDArrayNames(CSLConstList = nullptr) const override
  {
    std::vector<std::string> names;
    names.reserve(itsArrayOrder.size() + itsCoordOrder.size());
    for (const auto& n : itsCoordOrder) names.push_back(n);
    for (const auto& n : itsArrayOrder) names.push_back(n);
    return names;
  }

  std::shared_ptr<GDALMDArray> OpenMDArray(const std::string& name,
                                           CSLConstList = nullptr) const override
  {
    auto it = itsArrays.find(name);
    if (it != itsArrays.end()) return it->second;
    auto cit = itsCoords.find(name);
    if (cit != itsCoords.end()) return cit->second;
    return nullptr;
  }

  std::vector<std::shared_ptr<GDALDimension>> GetDimensions(
      CSLConstList = nullptr) const override
  {
    return itsDimList;
  }

  // Mutators used by the builder (not part of the GDAL public API).
  void addDimension(std::shared_ptr<GDALDimension> dim)
  {
    itsDimList.push_back(dim);
    itsDims[dim->GetName()] = std::move(dim);
  }
  void addArray(const std::string& name, std::shared_ptr<GDALMDArray> arr)
  {
    itsArrays[name] = std::move(arr);
    itsArrayOrder.push_back(name);
  }
  void addCoord(const std::string& name, std::shared_ptr<GDALMDArray> arr)
  {
    itsCoords[name] = std::move(arr);
    itsCoordOrder.push_back(name);
  }
  std::shared_ptr<GDALDimension> dim(const std::string& name) const
  {
    auto it = itsDims.find(name);
    return it == itsDims.end() ? nullptr : it->second;
  }

 private:
  QDDataset& itsDataset;
  std::vector<std::shared_ptr<GDALDimension>> itsDimList;
  std::map<std::string, std::shared_ptr<GDALDimension>> itsDims;
  std::map<std::string, std::shared_ptr<GDALMDArray>> itsArrays;
  std::map<std::string, std::shared_ptr<GDALMDArray>> itsCoords;
  std::vector<std::string> itsArrayOrder;
  std::vector<std::string> itsCoordOrder;
};

}  // namespace

std::shared_ptr<GDALGroup> buildRootGroup(QDDataset& ds)
{
  auto& qi = ds.info();
  if (!qi.IsGrid() || qi.Grid() == nullptr) return nullptr;
  const NFmiGrid* grid = qi.Grid();
  const unsigned long nx = grid->XNumber();
  const unsigned long ny = grid->YNumber();
  const unsigned long nT = qi.SizeTimes();
  const unsigned long nL = qi.SizeLevels();

  // ---- Y flip detection ----
  const bool flipY = (grid->Origo() != kTopLeft);

  // ---- SRS shared by all data arrays ----
  std::shared_ptr<OGRSpatialReference> srs;
  if (auto* dsSrs = ds.GetSpatialRef())
  {
    srs = std::shared_ptr<OGRSpatialReference>(dsSrs->Clone());
  }

  // ---- geotransform → projected x and y axis values ----
  const NFmiArea* area = qi.Area();
  std::vector<double> xValues, yValues;
  double xStart = 0, xStep = 1, yStart = 0, yStep = -1;
  if (area != nullptr)
  {
    const NFmiRect rect = area->WorldRect();
    const double pixelW = (rect.Right() - rect.Left()) / static_cast<double>(nx);
    const double pixelH = (rect.Top() - rect.Bottom()) / static_cast<double>(ny);
    xStart = rect.Left() + 0.5 * pixelW;
    xStep = pixelW;
    yStart = rect.Top() - 0.5 * pixelH;  // top-down (matches geotransform)
    yStep = -pixelH;
  }

  auto root = std::make_shared<QDRootGroup>("/", ds);

  // ---- dimensions ----
  auto dimT =
      std::make_shared<GDALDimensionWeakIndexingVar>("/", "time", "TEMPORAL", "FUTURE", static_cast<GUInt64>(nT));
  auto dimL =
      std::make_shared<GDALDimensionWeakIndexingVar>("/", "level", "VERTICAL", "UP", static_cast<GUInt64>(nL));
  auto dimY = std::make_shared<GDALDimensionWeakIndexingVar>("/", "y", "HORIZONTAL_Y", "SOUTH",
                                              static_cast<GUInt64>(ny));
  auto dimX = std::make_shared<GDALDimensionWeakIndexingVar>("/", "x", "HORIZONTAL_X", "EAST",
                                              static_cast<GUInt64>(nx));
  root->addDimension(dimT);
  root->addDimension(dimL);
  root->addDimension(dimY);
  root->addDimension(dimX);

  // ---- coordinate variables ----
  // Each dimension MUST have its indexing variable wired up via
  // SetIndexingVariable. Without that, GDAL's NetCDF writer drops the
  // coordinate variables on translate (warning: "No 1D variable is indexed
  // by dimension <name>") and CF-aware tools like Panoply / NetCDF-Java
  // can't recognise time / x / y at all.
  //
  // The coord arrays also carry CF attributes (axis, standard_name, units)
  // so that consumers like Panoply identify which dim is which without
  // having to fall back to "first dim is X" heuristics.

  // x, y. Attributes depend on whether the SRS is geographic (degrees) or
  // projected (metres). qi.Area()->WKT() is empty for some files so default
  // to projected/metres if we can't tell.
  bool isGeographic = false;
  if (const NFmiArea* area = qi.Area())
  {
    OGRSpatialReference srs;
    srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    if (!area->WKT().empty() && srs.importFromWkt(area->WKT().c_str()) == OGRERR_NONE)
      isGeographic = srs.IsGeographic() != 0;
  }
  auto xArr = RegularDoubleArray::Create("/", "x", dimX, xStart, xStep);
  auto yArr = RegularDoubleArray::Create("/", "y", dimY, yStart, yStep);
  xArr->addAttr("axis", "X");
  yArr->addAttr("axis", "Y");
  if (isGeographic)
  {
    xArr->addAttr("standard_name", "longitude");
    xArr->addAttr("units", "degrees_east");
    xArr->addAttr("long_name", "longitude");
    yArr->addAttr("standard_name", "latitude");
    yArr->addAttr("units", "degrees_north");
    yArr->addAttr("long_name", "latitude");
  }
  else
  {
    xArr->addAttr("standard_name", "projection_x_coordinate");
    xArr->addAttr("units", "m");
    xArr->addAttr("long_name", "x coordinate of projection");
    yArr->addAttr("standard_name", "projection_y_coordinate");
    yArr->addAttr("units", "m");
    yArr->addAttr("long_name", "y coordinate of projection");
  }
  root->addCoord("x", xArr);
  root->addCoord("y", yArr);
  dimX->SetIndexingVariable(xArr);
  dimY->SetIndexingVariable(yArr);

  // levels
  std::vector<float> levelValues(nL, 0.f);
  for (unsigned long l = 0; l < nL; ++l)
  {
    qi.LevelIndex(l);
    if (auto* lv = qi.Level()) levelValues[l] = lv->LevelValue();
  }
  auto levelArr = FloatVectorArray::Create("/", "level", dimL, std::move(levelValues));
  levelArr->addAttr("axis", "Z");
  levelArr->addAttr("long_name", "vertical level");
  root->addCoord("level", levelArr);
  dimL->SetIndexingVariable(levelArr);

  // times: epoch seconds
  std::vector<int64_t> timeValues(nT, 0);
  for (unsigned long t = 0; t < nT; ++t)
  {
    qi.TimeIndex(t);
    const NFmiMetTime& tm = qi.Time();
    std::tm tt{};
    tt.tm_year = tm.GetYear() - 1900;
    tt.tm_mon = tm.GetMonth() - 1;
    tt.tm_mday = tm.GetDay();
    tt.tm_hour = tm.GetHour();
    tt.tm_min = tm.GetMin();
    tt.tm_sec = 0;
    timeValues[t] = static_cast<int64_t>(timegm(&tt));
  }
  auto timeArr = Int64VectorArray::Create("/", "time", dimT, std::move(timeValues),
                                          "seconds since 1970-01-01T00:00:00Z");
  timeArr->addAttr("axis", "T");
  timeArr->addAttr("standard_name", "time");
  timeArr->addAttr("long_name", "time");
  timeArr->addAttr("calendar", "gregorian");
  root->addCoord("time", timeArr);
  dimT->SetIndexingVariable(timeArr);

  // ---- data arrays: one per (param, sub-param) ----
  std::vector<std::shared_ptr<GDALDimension>> dataDims = {dimT, dimL, dimY, dimX};
  std::unordered_set<std::string> usedNames;
  // Reserve coord-var names to avoid collisions.
  for (const auto& n : {"x", "y", "level", "time"}) usedNames.insert(n);

  for (unsigned long p = 0; p < qi.SizeParams(); ++p)
  {
    qi.ParamIndex(p);
    const auto& dataIdent = qi.Param();
    const std::string parentName = toUtf8(dataIdent.GetParamName().CharPtr());
    const long parentId = dataIdent.GetParamIdent();

    if (dataIdent.HasDataParams() && dataIdent.GetDataParams() != nullptr)
    {
      for (const auto& sub : dataIdent.GetDataParams()->ParamsVector())
      {
        const char* subRaw = sub.GetParamName().CharPtr();
        const std::string subName = toUtf8(subRaw);
        const long subId = sub.GetParamIdent();
        ArraySpec spec{p, static_cast<unsigned long>(subId), subName, subId};
        const std::string varName = uniquify(sanitizeName(subRaw), usedNames);
        root->addArray(varName,
                       QDDataMDArray::Create(ds, "/", varName, spec, dataDims, srs, flipY, ny));
      }
    }
    else
    {
      const char* parentRaw = dataIdent.GetParamName().CharPtr();
      ArraySpec spec{p, 0, parentName, parentId};
      const std::string varName = uniquify(sanitizeName(parentRaw), usedNames);
      root->addArray(varName,
                     QDDataMDArray::Create(ds, "/", varName, spec, dataDims, srs, flipY, ny));
    }
  }

  return root;
}

}  // namespace GdalQueryData
}  // namespace SmartMet
