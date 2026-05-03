// ======================================================================
/*!
 * \file NFmiAreaMaskList.h
 * \brief Interface for class NFmiAreaMaskList
 */
// ======================================================================

#pragma once

#include "NFmiAreaMask.h"
#include <memory>
#include <vector>

class NFmiAreaMaskList
{
 public:
  NFmiAreaMaskList();
  NFmiAreaMaskList(const NFmiAreaMaskList &theOther);
  virtual ~NFmiAreaMaskList();
  NFmiAreaMaskList& operator = (const NFmiAreaMaskList&);
  static std::shared_ptr<NFmiAreaMaskList> CreateShallowCopy(
      const std::shared_ptr<NFmiAreaMaskList> &theOther);

  void Add(std::shared_ptr<NFmiAreaMask> &theMask);
  bool Remove();
  void Clear();
  unsigned long NumberOfItems();

  bool Reset();
  bool Next();
  std::shared_ptr<NFmiAreaMask> Current();

  bool UseMask() { return fMaskInUse; }
  bool IsMasked(const NFmiPoint &theLatLon);
  double MaskValue(const NFmiPoint &theLatLon);
  bool CheckIfMaskUsed();
  bool SyncronizeMaskTime(const NFmiMetTime &theTime);

  bool Index(unsigned long index);    // 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on
                                      // taas 0:sta alkava
  bool Find(unsigned long theIndex);  // 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on
                                      // taas 0:sta alkava
  bool Find(const NFmiDataIdent &theParam);
  bool Find(const NFmiDataIdent &theParam, const NFmiLevel *theLevel);

 private:
  bool IsValidIndex(int theIndex);

  std::vector<std::shared_ptr<NFmiAreaMask> > itsMaskVector;
  int itsCurrentIndex;  // Reset laittaa tämän -1:ksi, 1. maski löytyy 0:sta ja viimeinen size-1:stä
  bool fMaskInUse;      // Arvo asetetaan kun tarkastetaan onko mikään

};  // class NFmiAreaMaskList

// ======================================================================
