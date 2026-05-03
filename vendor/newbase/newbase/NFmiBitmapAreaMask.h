// ======================================================================
/*!
 * \file NFmiBitmapAreaMask.h
 * \brief Interface of class NFmiBitmapAreaMask
 */
// ======================================================================

#pragma once

#include "NFmiAreaMaskImpl.h"
#include "NFmiDataIdent.h"
#include "NFmiDataMatrix.h"

class NFmiArea;
class NFmiGrid;

//! Undocumented
class NFmiBitmapAreaMask : public NFmiAreaMaskImpl
{
 public:
  virtual ~NFmiBitmapAreaMask();

  NFmiBitmapAreaMask();

  NFmiBitmapAreaMask(const NFmiBitmapAreaMask& theMask);

  // tätä konstruktoria käytettäessä asetetaan maskien arvot käsipelillä ulkoa päin
  NFmiBitmapAreaMask(int theXSize,
                     int theYSize,
                     const NFmiArea* theArea,
                     const NFmiDataIdent* theDataIdent,
                     const NFmiLevel* theLevel,
                     BinaryOperator thePostBinaryOperator);

  NFmiBitmapAreaMask(const NFmiGrid& theGrid,
                     const NFmiDataIdent* theDataIdent,
                     const NFmiLevel* theLevel,
                     BinaryOperator thePostBinaryOperator);

  NFmiAreaMask* Clone() const;
#ifdef UNIX
  // RHEL6 compiler bug fails the reference
  void Init(std::shared_ptr<NFmiFastQueryInfo> theInfo,
            const NFmiCalculationCondition& theOperation);
#else
  void Init(std::shared_ptr<NFmiFastQueryInfo>& theInfo,
            const NFmiCalculationCondition& theOperation);
#endif

  bool IsMasked(const NFmiPoint& theLatLon) const;
  bool IsMasked(int theIndex)
      const;  // erikoistapaus optimoituun käyttöön (ei voi käyttää kaikille luokille!!!!)
  void Mask(int theIndex, bool newStatus);
  double MaskValue(const NFmiPoint& theLatLon) const;  // palauttaa kertoimen 0:n ja 1:n välilt
  void SetAll(bool theNewState);  // HUOM! toimii vain NFmiBinaryMask:in kanssa. Asettaa koko maskin
                                  // kaikki arvot halutuksi.

  const NFmiDataIdent* DataIdent() const { return itsDataIdent; }
  const NFmiParam* Param() const { return itsDataIdent ? itsDataIdent->GetParam() : 0; }
  using NFmiAreaMaskImpl::Level;
  const NFmiLevel* Level() const { return itsLevel; }

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString(
      void) const;  // tekee  param nimi tai vastaavan stringin pätkän

 private:
  NFmiBitmapAreaMask& operator=(const NFmiBitmapAreaMask& theMask);

  void Destroy();
  int LatLon2Index(const NFmiPoint& theLatLon) const;

  bool CheckIndex(int theIndex) const
  {
    return (theIndex > 0 && theIndex < (itsGridXSize * itsGridYSize));
  }

  std::vector<bool> itsBitmask;
  NFmiArea* itsArea;  // omistaa, tuhoaa
  int itsGridXSize;
  int itsGridYSize;

  NFmiDataIdent* itsDataIdent;  // omistaa, tuhoaa
  NFmiLevel* itsLevel;          // omistaa, tuhoaa

};  // class NFmiBitmapAreaMask

// ======================================================================
