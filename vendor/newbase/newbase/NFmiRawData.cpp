// ======================================================================
/*!
 * \file NFmiRawData.cpp
 * \brief Implementation of class NFmiRawData
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4503)  // VC++ 2012 kääntäjällä tulee paljon ruman näköistä varoitusta
                                 // ilman tätä (decorated name length exceeded (4096 bytes), name
                                 // was truncated)
#pragma warning(disable : 4101)  // VC++ 2012 kääntäjällä tulee paljon 'lock' unreferenced
                                 // -varoitusta ilman tätä.  Scoped variableen ei ole
                                 // tarkoitustakaan viitata, sen tarkoitus on purkaa jotain scopen
                                 // loppuessa.
#endif

#include "NFmiRawData.h"
#include "NFmiVersion.h"

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <macgyver/Exception.h>
#include <macgyver/MappedFile.h>
#include <macgyver/StringConversion.h>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#ifndef _MSC_VER
#include <sys/mman.h>
#endif

#if 0
// paranoid mode
typedef boost::shared_mutex MutexType;
typedef boost::shared_lock<MutexType> ReadLock;
typedef boost::unique_lock<MutexType> WriteLock;
#else
// trust kernel to handle it
struct FooBar
{
};
using MutexType = FooBar;
using ReadLock = FooBar;
using WriteLock = FooBar;
#endif

using MappedFileParams = boost::iostreams::mapped_file_params;
using MappedFile = Fmi::MappedFile;

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Pimple class
 */
// ----------------------------------------------------------------------

class NFmiRawData::Pimple
{
 public:
  ~Pimple();
  Pimple();
  Pimple(const Pimple &other);
  Pimple(istream &file, size_t size, bool endianswap);
  Pimple(const string &filename, istream &file, size_t size);
  bool Init(size_t size);
  bool Init(size_t size,
            const std::string &theHeader,
            const std::string &theFilename,
            bool fInitialize);
  size_t Size() const;
  float GetValue(size_t index) const;

  bool GetValues(size_t startIndex, size_t step, size_t count, std::vector<float> &values) const;
  bool SetValues(size_t startIndex, size_t step, size_t count, const std::vector<float> &values);
  bool GetValuesPartial(size_t startIndex,
                        size_t rowCount,
                        size_t columnCount,
                        size_t step,
                        size_t rowSkip,
                        std::vector<float> &values) const;

  bool SetValue(size_t index, float value);
  void SetBinaryStorage(bool flag) const;
  bool IsBinaryStorageUsed() const;
  ostream &Write(ostream &file) const;
  void Backup(char *ptr) const;
  void Undo(char *ptr);
  bool IsReadOnly() const;

 private:
#ifdef NFMIRAWDATA_ENABLE_UNDO_REDO
  void Unmap() const;
#endif

  mutable MutexType itsMutex;
  mutable float *itsData{nullptr};                               // non-memory mapped data
  mutable boost::scoped_ptr<MappedFile> itsMappedFile{nullptr};  // memory mapped data
  size_t itsOffset{0};                                           // offset to raw data
  size_t itsSize{0};
  mutable bool itsSaveAsBinaryFlag{true};
  bool itsEndianSwapFlag{false};
  std::string itsFileName{};  // to ease examining core dumps
};

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::~Pimple()
{
  try
  {
    delete[] itsData;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple() : itsMutex(), itsMappedFile(nullptr) {}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple(const Pimple &other)
    : itsMutex(), itsSize(other.itsSize), itsSaveAsBinaryFlag(other.itsSaveAsBinaryFlag)

{
  try
  {
    // We assume copied data will be changed so that copying mmapping would
    // be a wasted effort

    WriteLock lock(itsMutex);
    itsData = new float[itsSize];

    if (other.itsData != nullptr)
    {
      memcpy(itsData, other.itsData, itsSize * sizeof(float));
    }
    else
    {
      auto *dst = reinterpret_cast<char *>(itsData);
      const char *src = other.itsMappedFile->const_data() + other.itsOffset;
      memcpy(dst, src, itsSize * sizeof(float));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief File constructor (memory mapping)
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple(const string &filename, istream &file, size_t size)
    : itsMutex(), itsSize(size), itsFileName(filename)

{
  try
  {
    WriteLock lock(itsMutex);

    // Backward compatibility:
    long datatype;
    file >> datatype;

    file >> itsSaveAsBinaryFlag;

    size_t poolsize = 0;
    file >> poolsize;

    if (poolsize != itsSize * sizeof(float))
      throw Fmi::Exception(BCP, "Invalid datapool size in querydata");

    // Skip last newline charater after poolsize
    char ch;
    file.get(ch);

    // Sanity check on the size of the data. Note that old querydata
    // has an extra endl at the end, hence we must permit 2 extra
    // characters at the end for legacy data to work

    std::size_t filesize = std::filesystem::file_size(filename);
    itsOffset = file.tellg();

    if (itsOffset + poolsize > filesize)
      throw Fmi::Exception(BCP,
                           "Querydata file " +
                               Fmi::to_string(itsOffset + poolsize - filesize) +
                               " bytes too short: '" + filename + "'");

    else if (filesize - itsOffset - poolsize > 2)
      throw Fmi::Exception(BCP,
                           "Querydata file " +
                               Fmi::to_string(filesize - itsOffset - poolsize) +
                               " bytes too long: '" + filename + "'");

    // memory map starting from this file position
    // only if itsSaveAsBinaryFlag is true

    if (itsSaveAsBinaryFlag)
    {
      MappedFileParams params(filename.c_str());
      params.flags = boost::iostreams::mapped_file::readonly;
      params.length = filesize;
      itsMappedFile.reset(new MappedFile(params));
      if (!itsMappedFile->is_open())
        throw Fmi::Exception(BCP, "Failed to memory map '" + filename + "' in read only mode");
    }
    else
    {
      itsData = new float[itsSize];

      for (size_t idx = 0; idx < itsSize; ++idx)
        file >> itsData[idx];
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Stream constructor (no memory mapping)
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple(istream &file, size_t size, bool endianswap)
    : itsMutex(),
      itsData(nullptr),
      itsMappedFile(nullptr),
      itsOffset(0),
      itsSize(size),

      itsEndianSwapFlag(endianswap)
{
  try
  {
    WriteLock lock(itsMutex);

    delete itsData;
    itsData = new float[itsSize];

    // Backward compatibility:
    long datatype;
    file >> datatype;

    // We trust everything to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 6)
      file >> itsSaveAsBinaryFlag;
    else
      itsSaveAsBinaryFlag = false;

    size_t poolsize = 0;
    file >> poolsize;

    if (!itsSaveAsBinaryFlag)
    {
      for (size_t idx = 0; idx < itsSize; ++idx)
        file >> itsData[idx];
    }
    else
    {
      if (poolsize != itsSize * sizeof(float))
        throw Fmi::Exception(BCP, "Invalid datapool size in querydata");

      // Skip last newline charater after poolsize
      char ch;
      file.get(ch);

      auto *ptr = reinterpret_cast<char *>(itsData);
      file.read(ptr, poolsize);
    }

    if (file.fail())
      throw Fmi::Exception(BCP, "Failed to read rawdata from input stream");

    if (itsEndianSwapFlag)
    {
      char tmp1, tmp2, tmp3, tmp4;
      auto *ptr = reinterpret_cast<char *>(itsData);
      for (size_t i = 3; i < itsSize * sizeof(float); i += 4)
      {
        tmp1 = ptr[i - 3];
        tmp2 = ptr[i - 2];
        tmp3 = ptr[i - 1];
        tmp4 = ptr[i - 0];

        ptr[i - 3] = tmp4;
        ptr[i - 2] = tmp3;
        ptr[i - 1] = tmp2;
        ptr[i - 0] = tmp1;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize memory mapped file for writing
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::Init(size_t size,
                               const std::string &theHeader,
                               const std::string &theFilename,
                               bool fInitialize)
{
  try
  {
    WriteLock lock(itsMutex);

    // Update number of elements in the data

    itsSize = size;

    // Create info header fully up to start of raw data

    long datatype = 6;  // float??
    bool saveasbinary = true;
    std::ostringstream headerstream;
    headerstream << theHeader << '\n'
                 << datatype << '\n'
                 << saveasbinary << '\n'
                 << itsSize * sizeof(float) << '\n';
    std::string fullheader = headerstream.str();

    // Now we know the offset to the start of raw data

    itsOffset = fullheader.size();

    // Create memory mapped file for writing

    MappedFileParams params(theFilename.c_str());
    params.flags = boost::iostreams::mapped_file::readwrite;
    params.new_file_size = itsOffset + itsSize * sizeof(float);
    itsMappedFile.reset(new MappedFile(params));

    // Initialize the header

    char *headerdata = itsMappedFile->data();
    memcpy(headerdata, fullheader.c_str(), fullheader.size());

    // Initialize the data section to kFloatMissing if so requested.
    // It seems as if this is not requested, the mapped region
    // will be initialized to zero bytes from start to finish.

    if (fInitialize)
    {
      auto *data = reinterpret_cast<float *>(itsMappedFile->data() + itsOffset);
      for (std::size_t i = 0; i < itsSize; i++)
        data[i] = kFloatMissing;
    }

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Init
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::Init(size_t size)
{
  try
  {
    WriteLock lock(itsMutex);

    itsData = nullptr;
    itsSize = size;
    if (itsSize > 0)
    {
      itsData = new float[itsSize];
      for (size_t i = 0; i < itsSize; i++)
        itsData[i] = kFloatMissing;
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Get size
 */
// ----------------------------------------------------------------------

size_t NFmiRawData::Pimple::Size() const
{
  return itsSize;
}
// ----------------------------------------------------------------------
/*!
 * \brief Set storage mode to binary/ascii
 */
// ----------------------------------------------------------------------

void NFmiRawData::Pimple::SetBinaryStorage(bool flag) const
{
  try
  {
    WriteLock lock(itsMutex);
    itsSaveAsBinaryFlag = flag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if binary output is used
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::IsBinaryStorageUsed() const
{
  try
  {
    ReadLock lock(itsMutex);
    return itsSaveAsBinaryFlag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Switch from mmapped data to dynamically allocated array
 */
// ----------------------------------------------------------------------

#ifdef NFMIRAWDATA_ENABLE_UNDO_REDO
void NFmiRawData::Pimple::Unmap() const
{
  try
  {
    if (itsData)
      return;  // oli jo alustettu

    itsData = new float[itsSize];
    char *dst = reinterpret_cast<char *>(itsData);

    char *src = reinterpret_cast<char *>(itsMappedRegion->get_address());
    memcpy(dst, src, itsSize * sizeof(float));

    itsMappedFile.reset();
    itsOffset = 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#endif

// ----------------------------------------------------------------------
/*!
 * \brief Get value
 */
// ----------------------------------------------------------------------

float NFmiRawData::Pimple::GetValue(size_t index) const
{
  try
  {
    ReadLock lock(itsMutex);

    if (index >= itsSize)
      return kFloatMissing;

    if (itsData)
      return itsData[index];

    auto *addr = itsMappedFile->const_data() + itsOffset + index * sizeof(float);
    float value;
    memcpy(&value, addr, sizeof(float));
    return value;

    // ASAN does not like the reinterpret_cast here due to bad float alignment:
    // const auto *ptr = reinterpret_cast<const float *>(itsMappedFile->const_data() + itsOffset);
    // return ptr[index];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiRawData::Pimple::GetValues(size_t startIndex,
                                    size_t step,
                                    size_t count,
                                    std::vector<float> &values) const
{
  try
  {
    if (startIndex + step * (count - 1) >= itsSize)
      return false;

    values.resize(count);

    const float *ptr;

    if (itsData)
      ptr = itsData;
    else
      ptr = reinterpret_cast<const float *>(itsMappedFile->const_data() + itsOffset);

    {
      ReadLock lock(itsMutex);

      size_t i = 0;
      std::generate(values.begin(), values.end(), [&] { return ptr[startIndex + (i++) * step]; });

      // C++17 (not supported yet), might enable additional compiler optimization

      // std::generate(values.begin(), values.end(), [&] { return i++; });
      // std::transform(std::execution::par_unseq,values.begin(),values.end(),
      // [=] (const float &i)
      // {
      // 	return ptr[startIndex + i*step];
      // });
    }

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiRawData::Pimple::SetValues(size_t startIndex,
                                    size_t step,
                                    size_t count,
                                    const std::vector<float> &values)
{
  if (startIndex + step * (count - 1) >= itsSize) return false;

  float *ptr = nullptr;

  if (itsData)
    ptr = itsData;
  else
    ptr = reinterpret_cast<float *>(itsMappedFile->data() + itsOffset);

  {
    WriteLock lock(itsMutex);

    for (size_t i = 0; i < count; i++)
      ptr[startIndex + i * step] = values[i];
  }

  return true;
}

bool NFmiRawData::Pimple::GetValuesPartial(size_t startIndex,
                                           size_t rowCount,
                                           size_t rowStep,
                                           size_t columnCount,
                                           size_t columnStep,
                                           std::vector<float> &values) const
{
  try
  {
    if (startIndex + rowStep * (rowCount - 1) + columnStep * (columnCount - 1) >= itsSize)
      return false;

    values.resize(rowCount * columnCount);

    const float *ptr;

    if (itsData)
      ptr = itsData;
    else
      ptr = reinterpret_cast<const float *>(itsMappedFile->const_data() + itsOffset);

    {
      ReadLock lock(itsMutex);

      size_t i = 0;

      for (size_t row = 0; row < rowCount; row++)
      {
        for (size_t column = 0; column < columnCount; column++)
        {
          values[column + row * columnCount] = ptr[startIndex + i];
          i += columnStep;
        }
        i += rowStep;
      }
    }

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set value
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::SetValue(size_t index, float value)
{
  try
  {
    WriteLock lock(itsMutex);

    if (index >= itsSize)
      return false;

    if (itsData)
    {
      itsData[index] = value;
      return true;
    }
    else if (itsOffset > 0)
    {
      if (itsMappedFile->flags() == boost::iostreams::mapped_file::readonly)
        throw Fmi::Exception(BCP, "Can't modify read-only memory-mapped data");

      // We have mmapped output data
      auto *ptr = reinterpret_cast<float *>(itsMappedFile->data() + itsOffset);
      ptr[index] = value;
      return true;
    }

    // copy-on-write semantics: switch to memory buffer on a write

#if 0
    Unmap();
    itsData[index] = value;
#endif

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Write to output
 */
// ----------------------------------------------------------------------

ostream &NFmiRawData::Pimple::Write(ostream &file) const
{
  try
  {
    ReadLock lock(itsMutex);

    // Backward compatibility when other than floats were supported
    const int kFloat = 6;
    file << kFloat << endl;

    if (DefaultFmiInfoVersion >= 6)
      file << itsSaveAsBinaryFlag << endl;

    file << itsSize * sizeof(float) << endl;

    if (itsSaveAsBinaryFlag && DefaultFmiInfoVersion >= 6)
    {
      if (itsData != nullptr)
      {
        auto *ptr = reinterpret_cast<char *>(itsData);
        file.write(ptr, itsSize * sizeof(float));
      }
      else
      {
        const char *ptr = itsMappedFile->const_data() + itsOffset;
        file.write(ptr, itsSize * sizeof(float));
      }

      // Backward compatibility - not sure if needed:
      file << endl;
    }
    else
    {
      for (size_t idx = 0; idx < itsSize; ++idx)
        file << GetValue(idx) << ' ';
    }
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Backup the raw data
 */
// ----------------------------------------------------------------------

void NFmiRawData::Pimple::Backup(char *ptr) const
{
  try
  {
    if (itsData)
    {
      ReadLock lock(itsMutex);

      // we assume data which is backed up is edited, so might as well unmap
#ifdef NFMIRAWDATA_ENABLE_UNDO_REDO
      Unmap();
#endif

      auto *src = reinterpret_cast<char *>(itsData);
      memcpy(ptr, src, itsSize * sizeof(float));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Undo changes from backup
 */
// ----------------------------------------------------------------------

void NFmiRawData::Pimple::Undo(char *ptr)
{
  try
  {
    if (itsData)
    {
      WriteLock lock(itsMutex);

      // This may be slower than necessary when mmapped, but since Backup
      // unmaps this really should never actually unmap

#ifdef NFMIRAWDATA_ENABLE_UNDO_REDO
      Unmap();
#endif
      auto *src = reinterpret_cast<char *>(itsData);
      memcpy(src, ptr, itsSize * sizeof(float));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiRawData::Pimple::IsReadOnly() const
{
  if (itsMappedFile)
  {
    return itsMappedFile->flags() == boost::iostreams::mapped_file::readonly;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiRawData::~NFmiRawData()
{
  try
  {
    delete itsPimple;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData() : itsPimple(new Pimple()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData(const NFmiRawData &other) : itsPimple(new Pimple(*other.itsPimple)) {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor anticipating a memory mapped file
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData(const string &filename, istream &file, size_t size)
    : itsPimple(new Pimple(filename, file, size))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor not anticipating a memory mapped file
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData(istream &file, size_t size, bool endianswap)
    : itsPimple(new Pimple(file, size, endianswap))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Init
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Init(size_t size)
{
  try
  {
    return itsPimple->Init(size);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Init memory mapped file for writing
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Init(size_t size,
                       const std::string &theHeader,
                       const std::string &theFilename,
                       bool fInitialize)
{
  try
  {
    return itsPimple->Init(size, theHeader, theFilename, fInitialize);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return size of raw data
 */
// ----------------------------------------------------------------------

size_t NFmiRawData::Size() const
{
  try
  {
    return itsPimple->Size();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Get value at given index
 */
// ----------------------------------------------------------------------

float NFmiRawData::GetValue(size_t index) const
{
  try
  {
    return itsPimple->GetValue(index);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Resizes values (invalidates iterators!!) to count and populates it with values at
 * startIndex, startIndex+step, startIndex+step*2, ..., startIndex+step*count. Returns false if
 * out-of-range, true otherwise.
 */
// ----------------------------------------------------------------------

bool NFmiRawData::GetValues(size_t startIndex,
                            size_t step,
                            size_t count,
                            std::vector<float> &values) const
{
  try
  {
    return itsPimple->GetValues(startIndex, step, count, values);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiRawData::SetValues(size_t startIndex,
                            size_t step,
                            size_t count,
                            const std::vector<float> &values)
{
  try
  {
    return itsPimple->SetValues(startIndex, step, count, values);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Resizes values (invalidates iterators!!) to count and populates it with values at
 * startIndex, startIndex+step, startIndex+step*2, ..., startIndex+step*count. Returns false if
 * out-of-range, true otherwise.
 */
// ----------------------------------------------------------------------

bool NFmiRawData::GetValuesPartial(size_t startIndex,
                                   size_t rowCount,
                                   size_t rowStep,
                                   size_t columnCount,
                                   size_t columnStep,
                                   std::vector<float> &values) const
{
  try
  {
    return itsPimple->GetValuesPartial(
        startIndex, rowCount, rowStep, columnCount, columnStep, values);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set value at given index
 */
// ----------------------------------------------------------------------

bool NFmiRawData::SetValue(size_t index, float value)
{
  try
  {
    return itsPimple->SetValue(index, value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Set binary storage mode based on given flag
 */
// ----------------------------------------------------------------------

void NFmiRawData::SetBinaryStorage(bool flag) const
{
  try
  {
    itsPimple->SetBinaryStorage(flag);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Is binary storage mode on?
 */
// ----------------------------------------------------------------------

bool NFmiRawData::IsBinaryStorageUsed() const
{
  try
  {
    return itsPimple->IsBinaryStorageUsed();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Write raw data
 */
// ----------------------------------------------------------------------

ostream &NFmiRawData::Write(ostream &file) const
{
  try
  {
    return itsPimple->Write(file);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Backup data to given pointer
 */
// ----------------------------------------------------------------------

void NFmiRawData::Backup(char *ptr) const
{
  try
  {
    itsPimple->Backup(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Restore data from given pointer
 */
// ----------------------------------------------------------------------

void NFmiRawData::Undo(char *ptr)
{
  try
  {
    itsPimple->Undo(ptr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
