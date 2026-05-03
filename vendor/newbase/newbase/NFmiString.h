// ======================================================================
/*!
 * \file NFmiString.h
 * \brief Interface of class NFmiString
 */
// ======================================================================

#pragma once

#include "NFmiSaveBaseFactory.h"
#include "NFmiSortable.h"
#include <macgyver/Exception.h>
#include <string>

class NFmiString : public NFmiSortable
{
 public:
  virtual ~NFmiString();

  NFmiString();
  NFmiString(const NFmiString &aString);
  explicit NFmiString(unsigned long len, bool fSetLengthToRealStringLength = false);
  NFmiString(const unsigned char *aText);
  NFmiString(const unsigned char *aText, unsigned long len);
  NFmiString(const char *aText);
  NFmiString(const char *, unsigned long len);
  NFmiString(const std::string &str);

  bool IsValue() const;
  void Set(const unsigned char *text, unsigned long len, bool fSetLengthToRealStringLength = false);
  virtual unsigned long ClassId() const;

  NFmiString &Add(const unsigned char *aChar);

  unsigned long GetLen() const;
  const NFmiString GetChars(unsigned long firstChar, unsigned long length) const;

  char *GetCharsPtr(unsigned long firstChar, unsigned long length) const;
  const unsigned char *GetCharPtr() const;
  const char *CharPtr() const;

  void SetCharPtr(unsigned char *);
  void TrimR(unsigned char theChar = ' ');
  void TrimL(unsigned char theChar = ' ');
  void FillR(long theSize, unsigned char theChar = ' ');
  void UpperCase();
  void LowerCase();
  void FirstCharToUpper(unsigned long theUpperIndex = 0);
  void FirstInWordToUpper();

  bool FirstCharIsUpper() const;

  unsigned long Search(const unsigned char *searchChar, unsigned long fromPos = 1) const;
  unsigned long SearchLast(const unsigned char *searchChar) const;
  unsigned long SearchLast(const unsigned char *searchChar, unsigned long limit) const;
  int CharCount(unsigned char theChar) const;
  virtual NFmiString *Clone() const;

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  NFmiString &operator=(const unsigned char *aChar);
  NFmiString &operator=(const char *aChar);
  NFmiString &operator=(const NFmiString &);
  NFmiString &operator+=(const unsigned char *aChar);
  NFmiString &operator+=(const char *aChar);
  NFmiString &operator+=(const char aChar);
  NFmiString &operator+=(const NFmiString &);

  bool operator==(const NFmiString &other) const;
  bool operator==(const char *other) const;

  unsigned char operator[](const unsigned long counter) const;

  operator char *() const;
  operator unsigned char *() const;

  bool ReplaceChars(const unsigned char *fromChar, const unsigned char *toChar);
  bool ReplaceChars2(const unsigned char *fromChars,
                     const unsigned char *toChars,
                     unsigned long replacementsMaxCount = kUnsignedLongMissing);
  bool Replace(const NFmiString &newChars, unsigned long fromIndex);
  void RemoveExtraSpaces();

  bool IsLessThan(const NFmiSortable &aFmiTest) const;
  bool IsEqual(const NFmiSortable &aFmiTest) const;

  std::size_t HashValue() const;

 protected:
  void SizeCheck(unsigned long len) const;

  unsigned char *fChar;
  unsigned long fLength;
  long fReservedLength;

};  // class NFmiString

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiString::GetLen() const
{
  return fLength;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const unsigned char *NFmiString::GetCharPtr() const
{
  return fChar;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiString::CharPtr() const
{
  return reinterpret_cast<const char *>(fChar);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiString::IsValue() const
{
  return bool(fLength >= 1);
}
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param aFmiString The string being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

inline NFmiString &NFmiString::operator=(const NFmiString &aFmiString)
{
  if (this != &aFmiString)
    Set(aFmiString.fChar, aFmiString.fLength);
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param aChar Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString &NFmiString::operator+=(const char *aChar)
{
  Add(reinterpret_cast<const unsigned char *>(aChar));
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param aChar Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString &NFmiString::operator+=(const unsigned char *aChar)
{
  Add(aChar);
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString *NFmiString::Clone() const
{
  return new NFmiString(*this);
}
// ----------------------------------------------------------------------
/*!
 * \param aFmiString Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString &NFmiString::operator+=(const NFmiString &aFmiString)
{
  if (this == &aFmiString)
  {
    NFmiString tmpStr(aFmiString);
    Add(tmpStr.GetCharPtr());
    return *this;
  }
  else
  {
    Add(aFmiString.GetCharPtr());
    return *this;
  }
  //  Add (aFmiString.GetCharPtr());
  //  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param counter Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned char NFmiString::operator[](const unsigned long counter) const
{
  SizeCheck(counter);
  return fChar[counter - 1];
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiString::ClassId() const
{
  return kNFmiString;
}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param other The string to compare to
 * \return True if the strings are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiString::operator==(const NFmiString &other) const
{
  return IsEqual(other) == true;
}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param other The string to compare to
 * \return True if the strings are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiString::operator==(const char *other) const
{
  return IsEqual(NFmiString(other)) == true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString::operator char *() const
{
  return reinterpret_cast<char *>(fChar);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString::operator unsigned char *() const
{
  return fChar;
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiString
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 * \todo NFmiString parameter should be const
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiString &ob)
{
  return ob.Write(file);
}
// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiString
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiString &ob)
{
  return ob.Read(file);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiString operator+(const NFmiString &lhs1, const NFmiString &lhs2)
{
  NFmiString temp(lhs1);
  temp += lhs2;
  return temp;
}

// ======================================================================
