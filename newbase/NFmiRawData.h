// ======================================================================
/*!
 * \file NFmiRawData.h
 * \brief Interface of class NFmiRawData
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <iosfwd>
#include <string>
#include <vector>

class NFmiRawData
{
 private:
  class Pimple;
  Pimple* itsPimple;

  NFmiRawData& operator=(const NFmiRawData& other);

 public:
  ~NFmiRawData();

  NFmiRawData();
  bool Init(size_t size);
  bool Init(size_t size,
            const std::string& theHeader,
            const std::string& theFilename,
            bool fInitialize);

  NFmiRawData(const NFmiRawData& other);

  // no mmap

  NFmiRawData(std::istream& file, size_t size, bool endianswap);

  // possible mmap
  NFmiRawData(const std::string& filename, std::istream& file, size_t size);

  size_t Size() const;
  float GetValue(size_t index) const;
  bool GetValues(size_t startIndex, size_t step, size_t count, std::vector<float>& values) const;
  bool SetValues(size_t startIndex, size_t step, size_t count, const std::vector<float>& values);
  bool GetValuesPartial(size_t startIndex,
                        size_t rowCount,
                        size_t rowStep,
                        size_t columnCount,
                        size_t columnStep,
                        std::vector<float>& values) const;
  bool SetValue(size_t index, float value);

  void SetBinaryStorage(bool flag) const;
  bool IsBinaryStorageUsed() const;

  std::ostream& Write(std::ostream& file) const;

  void Backup(char* ptr) const;
  void Undo(char* ptr);
  bool IsReadOnly() const;
};  // class NFmiRawData

// ======================================================================
