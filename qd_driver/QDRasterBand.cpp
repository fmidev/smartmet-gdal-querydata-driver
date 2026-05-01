#include "QDRasterBand.h"
#include "QDDataset.h"

#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiGlobals.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiParameterName.h>

#include <cpl_error.h>

#include <mutex>

namespace SmartMet
{
namespace GdalQueryData
{
QDRasterBand::QDRasterBand(QDDataset* parent, int nBandIn, unsigned long timeIdx)
    : itsTimeIndex(timeIdx)
{
  poDS = parent;
  nBand = nBandIn;
  eDataType = GDT_Float32;
  // One block = the whole 2D plane. QD grids are typically small enough that this
  // is more efficient than strip-by-strip, since each Values() call materializes
  // the full slab anyway.
  nBlockXSize = parent->GetRasterXSize();
  nBlockYSize = parent->GetRasterYSize();
  nRasterXSize = nBlockXSize;
  nRasterYSize = nBlockYSize;
}

double QDRasterBand::GetNoDataValue(int* pbSuccess)
{
  if (pbSuccess != nullptr) *pbSuccess = TRUE;
  return static_cast<double>(kFloatMissing);
}

CPLErr QDRasterBand::IReadBlock(int /*nBlockXOff*/, int /*nBlockYOff*/, void* pImage)
{
  auto* ds = static_cast<QDDataset*>(poDS);
  std::lock_guard<std::mutex> lock(ds->mutex());
  auto& qi = ds->info();

  bool ok;
  if (ds->subParamId() != 0)
  {
    // Activates the sub-param and the parent index in one call. ParamIndex()
    // would clear fUseSubParam, so we must NOT call it here.
    ok = qi.Param(static_cast<FmiParameterName>(ds->subParamId())) && qi.IsSubParamUsed();
  }
  else
  {
    ok = qi.ParamIndex(ds->paramIndex());
  }
  ok = ok && qi.LevelIndex(ds->levelIndex()) && qi.TimeIndex(itsTimeIndex);
  if (!ok)
  {
    CPLError(CE_Failure, CPLE_AppDefined, "querydata: failed to position iterator");
    return CE_Failure;
  }

  const NFmiDataMatrix<float> matrix = qi.Values();

  const int nx = nBlockXSize;
  const int ny = nBlockYSize;
  if (static_cast<unsigned long>(nx) != matrix.NX() ||
      static_cast<unsigned long>(ny) != matrix.NY())
  {
    CPLError(CE_Failure, CPLE_AppDefined,
             "querydata: matrix dimensions (%lux%lu) do not match band size (%dx%d)",
             matrix.NX(), matrix.NY(), nx, ny);
    return CE_Failure;
  }

  // QD grid origin: kBottomLeft (default) → matrix[x][0] is the south row, so we
  // must flip Y to write GDAL's top-down rows. kTopLeft → no flip.
  const NFmiGrid* grid = qi.Grid();
  const bool flipY = (grid == nullptr) || (grid->Origo() != kTopLeft);

  auto* out = static_cast<float*>(pImage);
  for (int row = 0; row < ny; ++row)
  {
    const int qdRow = flipY ? (ny - 1 - row) : row;
    for (int col = 0; col < nx; ++col) out[row * nx + col] = matrix[col][qdRow];
  }
  return CE_None;
}

}  // namespace GdalQueryData
}  // namespace SmartMet
