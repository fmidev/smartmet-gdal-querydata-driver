// ======================================================================
/*!
 * \file NFmiIndividual.h
 * \brief Interface of class NFmiIndividual
 */
// ======================================================================

#pragma once

#include "NFmiSortable.h"
#include "NFmiString.h"

//! Undocumented

class NFmiIndividual : public NFmiSortable
{
 public:
  virtual ~NFmiIndividual() {}
  NFmiIndividual() : itsName("None"), itsIdent(0) {}
  NFmiIndividual(long theIdent, const NFmiString &theName = "None")
      : itsName(theName), itsIdent(theIdent)
  {
  }

  NFmiIndividual(const NFmiIndividual &anOtherIndividual)
      : NFmiSortable(), itsName(anOtherIndividual.GetName()), itsIdent(anOtherIndividual.GetIdent())
  {
  }

  long GetIdent() const { return itsIdent; }
  const NFmiString &GetName() const { return itsName; }
  void SetIdent(long theIdent) { itsIdent = theIdent; }
  void SetName(const NFmiString &theName) { itsName = theName; }
  NFmiIndividual &operator=(const NFmiIndividual &theIndividual);

  virtual bool IsEqual(const NFmiSortable &aFmiTest) const;
  virtual bool IsLessThan(const NFmiSortable &aFmiTest) const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual const char *ClassName() const { return "NFmiIndividual"; }
  void SetContents(long ident, NFmiString name);

  std::size_t HashValue() const;

 private:
  NFmiString itsName;
  long itsIdent;

};  // class NFmiIndividual

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiIndividual
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiIndividual &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiIndividual
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiIndividual &ob)
{
  return ob.Read(file);
}
// ----------------------------------------------------------------------
/*!
 * \param theSortable Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiIndividual::IsEqual(const NFmiSortable &theSortable) const
{
  return itsIdent == (static_cast<const NFmiIndividual &>(theSortable)).itsIdent;
}

// ----------------------------------------------------------------------
/*!
 * \param theSortable Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiIndividual::IsLessThan(const NFmiSortable &theSortable) const
{
  return itsIdent > (static_cast<const NFmiIndividual &>(theSortable)).itsIdent;
}

// ======================================================================
