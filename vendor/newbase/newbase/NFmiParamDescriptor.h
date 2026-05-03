// ======================================================================
/*!
 * \file NFmiParamDescriptor.h
 * \brief Interface of class NFmiParamDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiParamBag.h"
#include "NFmiSaveBaseFactory.h"

//! Undocumented
class NFmiParamDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiParamDescriptor();
  NFmiParamDescriptor();
  NFmiParamDescriptor(const NFmiParamDescriptor &theParamDescriptor);
  NFmiParamDescriptor(const NFmiParamBag &theParamBag, bool interpolate = false);

  bool Next(bool fIgnoreSubParam = true);
  bool Previous(bool fIgnoreSubParam = true);
  void Reset();

  NFmiDataIdent &Param(bool fIgnoreSubParam = true) const;
  NFmiDataIdent &Param(unsigned long theIndex, bool fIgnoreSubParam = true) const;

  NFmiDataIdent &EditParam(bool fIgnoreSubParam = true);
  NFmiDataIdent &EditParam(unsigned long theIndex, bool fIgnoreSubParam = true);
  bool Param(const NFmiDataIdent &theDataIdent);
  bool Param(const NFmiParam &theParam);
  bool Param(FmiParameterName theParam);

  virtual unsigned long Index() const;
  bool Index(unsigned long theIndex);
  virtual unsigned long Size() const;

  virtual bool IsActive() const;
  virtual bool IsActive(bool fIgnoreSubParam) const;
  virtual bool IsActive(unsigned long index, bool fIgnoreSubParam = true) const;
  virtual bool SetActivity(bool theActivityState);
  virtual bool SetActivity(bool theActivityState, bool fIgnoreSubParam);
  virtual bool SetActivity(bool theActivityState,
                           unsigned long theIndex,
                           bool fIgnoreSubParam = true);
  virtual bool NextActive();
  virtual bool NextActive(bool fIgnoreSubParam);
  virtual unsigned long SizeActive() const;
  bool FindSubParam(const NFmiParam &theParam);
  bool FindSubParam(const NFmiDataIdent &theDataIdent);
  bool IsSubParamUsed() const;

  NFmiParamBag *ParamBag() const;
  bool Interpolate() const;
  void SetProducer(const NFmiProducer &newProducer);

  virtual const NFmiParamDescriptor Combine(const NFmiParamDescriptor &theCombine);
  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;

  NFmiParamDescriptor &operator=(const NFmiParamDescriptor &theParamDescriptor);
  bool operator==(const NFmiParamDescriptor &theParamDescriptor) const;

 protected:
  void Destroy();

 private:
  NFmiParamBag *itsParamBag;
  bool *itsActivity;  // käyttö lopetettu, NFmiDataIdent hoitaa nyt homman
  bool fInterpolate;

};  // class NFmiParamDescriptor

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiParamDescriptor::~NFmiParamDescriptor()
{
  Destroy();
}
// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::Next(bool fIgnoreSubParam)
{
  return itsParamBag->Next(fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::Previous(bool fIgnoreSubParam)
{
  return itsParamBag->Previous(fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiParamDescriptor::Reset()
{
  itsParamBag->Reset();
}
// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataIdent &NFmiParamDescriptor::Param(bool fIgnoreSubParam) const
{
  return *(itsParamBag->Current(fIgnoreSubParam == true));
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataIdent &NFmiParamDescriptor::EditParam(bool fIgnoreSubParam)
{
  return *(itsParamBag->Current(fIgnoreSubParam == true));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiParamBag *NFmiParamDescriptor::ParamBag() const
{
  return itsParamBag;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiParamDescriptor::Index() const
{
  return itsParamBag->CurrentIndex();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiParamDescriptor::Size() const
{
  return itsParamBag->GetSize();
}
// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::IsActive(bool fIgnoreSubParam) const
{
  return itsParamBag->IsActive(fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::IsActive(unsigned long theIndex, bool fIgnoreSubParam) const
{
  return itsParamBag->IsActive(theIndex, fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::IsActive() const
{
  return itsActivity[Index()];
}
// ----------------------------------------------------------------------
/*!
 * Vaihtaa kaikille parametreille tuottajan
 *
 * \param newProducer Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiParamDescriptor::SetProducer(const NFmiProducer &newProducer)
{
  itsParamBag->SetProducer(newProducer);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::IsSubParamUsed() const
{
  return itsParamBag->IsSubParamUsed();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiParamDescriptor::Interpolate() const
{
  return fInterpolate;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiParamDescriptor::ClassId() const
{
  return kNFmiParamDescriptor;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiParamDescriptor::ClassName() const
{
  return "NFmiParamDescriptor";
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiParamDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiParamDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiParamDescriptor
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiParamDescriptor &ob)
{
  return ob.Read(file);
}

// ======================================================================
