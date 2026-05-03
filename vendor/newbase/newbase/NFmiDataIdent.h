// ======================================================================
/*!
 * \file NFmiDataIdent.h
 * \brief Interface of class NFmiDataIdent
 */
// ======================================================================

#pragma once

#include "NFmiParam.h"
#include "NFmiParameterName.h"
#include "NFmiProducer.h"

class NFmiVoidPtrList;
class NFmiVoidPtrIterator;
class NFmiVoidPtrData;

//! Uhhuh, a nameless enum???
enum
{
  kSynopProducer
};

enum FmiParamType
{
  kUndefinedParam = 0,
  kContinuousParam = 1,
  kSteppingParam = 2,
  kNumberParam = 3,
  kCharacterParam = 4,
  kSymbolicParam = 5,
  kSimpleSymbolicParam = 6,
  kIntervalParam = 7,
  kWindBarbParam = 8,
  kIncrementalParam = 100

  // parametri voi olla yhtäaikaa jatkuva ja kasvava (100 + 1 = 101)
  // Käyttö: Normaalit param tyypit kysytään edelleen Type()-metodilla
  // mutta se ei palauta 100-osaa. IsIncremental() palauttaa true:n jos
  // inkrementaalinen parametri ja SetIncrementalType(bool)-metodilla asetetaan
  // parametri myös inkrementaaliseksi, edellisen arvon lisäksi.

};

class NFmiParamBag;

//! Undocumented
class NFmiDataIdent
{
 public:
  virtual ~NFmiDataIdent();
  NFmiDataIdent();
  NFmiDataIdent(const NFmiDataIdent &theDataIdent);

  // 14.5.2002/Marko Tee kontruktori, jolle annetaan FmiParameterName parametrina.
  // explicit NFmiDataIdent (FmiParameterName theParam); // ks. mallia seuraavasta

  NFmiDataIdent(const NFmiParam &theParam,
                const NFmiProducer &theProducer = NFmiProducer(kSynopProducer),
                FmiParamType theType = kContinuousParam,
                bool isGroup = true,
                bool isActive = true,
                bool containsIndividualParams = true,
                bool isDataParam = true,
                bool hasDataParam = false,
                NFmiParamBag *theSubParamBag = 0,
                NFmiVoidPtrList *theSecondaryProducerList = 0);

  void Destroy();

  NFmiParam *GetParam() const;
  NFmiProducer *GetProducer() const;

  void SetParam(const NFmiParam &theParam);
  void SetProducer(const NFmiProducer &theProducer);

  const NFmiString &GetParamName() const;
  unsigned long GetParamIdent() const;
  unsigned long Type() const;
  void Type(unsigned long theType) { itsType = static_cast<FmiParamType>(theType); };
  bool IsIncremental() const;
  void SetIncrementalType(bool newState);

  bool IsActive() const;
  bool IsGroup() const;

  bool ContainsIndividualParams() const;

  bool IsDataParam() const;
  bool IsDataProducer() const;
  bool HasDataParams() const;
  NFmiParamBag *GetDataParams() const;

  void SetActive(bool isActive = true);

  bool NextSecondaryProducer();
  NFmiProducer *CurrentSecondaryProducer() const;
  void ResetSecondaryProducer();
  bool IsSecondaryProducerList() const;
  bool IsDataParam(const FmiParameterName &theParam);
  bool IsDataParam(const NFmiDataIdent &theDataIdent);

  const NFmiDataIdent &FirstDataParam() const;
  void ResetDataParams();
  NFmiDataIdent &CurrentDataParam();
  bool NextDataParam();
  bool ResetLastDataParams();
  bool PreviousDataParam();

  bool NextActiveDataParam();
  void SetActiveDataParam(const NFmiParam &theParam, bool isActive);
  NFmiDataIdent &CurrentActiveDataParam();

  void SetProducers(const NFmiProducer &theProducer);

  NFmiDataIdent &operator=(const NFmiDataIdent &theDataIdent);
  bool operator==(const NFmiDataIdent &theDataIdent) const;
  bool operator==(const NFmiParam &theParam) const;
  bool operator==(const NFmiProducer &theProducer) const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName() const;

 private:
  NFmiParam *itsParam;
  NFmiProducer *itsProducer;
  unsigned long itsType;
  bool fIsActive;
  bool fIsGroup;
  bool fContainsIndividualParams;
  bool fIsDataParam;
  bool fHasDataParams;
  NFmiParamBag *itsDataParams;

  NFmiVoidPtrList *itsSecondaryProducers;
  NFmiVoidPtrIterator *itsSecondaryProducerIterator;
  NFmiVoidPtrData *itsCurrentSecondaryProducer;

};  // class NFmiDataIdent

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiDataIdent::~NFmiDataIdent()
{
  Destroy();
}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiDataIdent::NFmiDataIdent()
    : itsParam(new NFmiParam),
      itsProducer(new NFmiProducer),
      itsType(kContinuousParam),
      fIsActive(false),
      fIsGroup(true),
      fContainsIndividualParams(true),
      fIsDataParam(false),
      fHasDataParams(false),
      itsDataParams(0),
      itsSecondaryProducers(0),
      itsSecondaryProducerIterator(0),
      itsCurrentSecondaryProducer(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::IsSecondaryProducerList() const
{
  return itsSecondaryProducers ? true : false;
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theDataIdent The object to compara against
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::operator==(const NFmiDataIdent &theDataIdent) const
{
  return (*itsParam == *(theDataIdent.itsParam) && *itsProducer == *(theDataIdent.itsProducer));
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::operator==(const NFmiParam &theParam) const
{
  return (*itsParam == theParam);
}

// ----------------------------------------------------------------------
/*!
 * \param theProducer Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::operator==(const NFmiProducer &theProducer) const
{
  return *itsProducer == theProducer;
}

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiDataIdent
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiDataIdent &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiDataIdent
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiDataIdent &ob)
{
  return ob.Read(file);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiParam *NFmiDataIdent::GetParam() const
{
  return itsParam;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiProducer *NFmiDataIdent::GetProducer() const
{
  return itsProducer;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString &NFmiDataIdent::GetParamName() const
{
  return itsParam->GetName();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataIdent::GetParamIdent() const
{
  return itsParam->GetIdent();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataIdent::Type() const
{
  return itsType % kIncrementalParam;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::IsIncremental() const
{
  return itsType / kIncrementalParam == 1;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::IsActive() const
{
  return fIsActive;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::IsGroup() const
{
  return fIsGroup;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::ContainsIndividualParams() const
{
  return fContainsIndividualParams;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::IsDataParam() const
{
  return fIsDataParam;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::IsDataProducer() const
{
  return itsProducer != 0;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataIdent::HasDataParams() const
{
  return fHasDataParams;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiParamBag *NFmiDataIdent::GetDataParams() const
{
  return itsDataParams;
}
// ----------------------------------------------------------------------
/*!
 * \param isActive Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataIdent::SetActive(bool isActive)
{
  fIsActive = isActive;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiDataIdent::ClassName() const
{
  return "NFmiDataIdent";
}

// ======================================================================
