// ======================================================================
/*!
 * \file NFmiString.cpp
 * \brief Implementation of class NFmiString
 */
// ======================================================================
/*!
 * \class NFmiString
 *
 * A deprecated string class - always prefer std::string if possible.
 *
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // poistaa MSVC++2008 kääntäjän tekemiä varoituksia: "warning
                                 // C4996: 'sprintf': This function or variable may be unsafe.
                                 // Consider using sprintf_s instead"
#endif

#include "NFmiString.h"
#include "NFmiStringTools.h"

#include <macgyver/Hash.h>
#include <macgyver/Exception.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef UNIX
#include <clocale>
#endif

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param aText Undocumented
 * \param len Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(const unsigned char *aText, unsigned long len)
    : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(aText, len);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param aText Undocumented
 * \param len Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(const char *aText, unsigned long len)
    : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(reinterpret_cast<const unsigned char *>(aText), len);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString() : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(reinterpret_cast<const unsigned char *>(""), 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param len Undocumented
 * \param fSetLengthToRealStringLength Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(unsigned long len, bool fSetLengthToRealStringLength)
    : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(reinterpret_cast<const unsigned char *>(""),
        len,
        fSetLengthToRealStringLength);  // 8.10.1997/Marko
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param aText Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(const unsigned char *aText)
    : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(aText, strlen(reinterpret_cast<const char *>(aText)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param aText Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(const char *aText) : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(reinterpret_cast<const unsigned char *>(aText), strlen(aText));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param aString Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(const NFmiString &aString)
    : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(aString.fChar, aString.fLength);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param str Undocumented
 */
// ----------------------------------------------------------------------

NFmiString::NFmiString(const std::string &str)
    : NFmiSortable(), fChar(), fLength(), fReservedLength(-1)
{
  try
  {
    Set(reinterpret_cast<unsigned char *>(const_cast<char *>(str.c_str())), str.size());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiString::~NFmiString()
{
  try
  {
    delete[] fChar;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fFmi Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiString::IsEqual(const NFmiSortable &fFmi) const
{
  try
  {
    return strcmp(reinterpret_cast<char *>(fChar),
                  reinterpret_cast<const char *>((static_cast<const NFmiString &>(fFmi)).fChar)) ==
           0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aText Undocumented
 * \param len Undocumented
 * \param fSetLengthToRealStringLength Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::Set(const unsigned char *aText,
                     unsigned long len,
                     bool fSetLengthToRealStringLength)
{
  try
  {
    if (fReservedLength <= static_cast<int>(len))
    {
      if (fReservedLength > -1)
      {
        delete[] fChar;
      }

      fReservedLength = (len + 1);                 // add 1 for safety in case of a null string
      fChar = new unsigned char[fReservedLength];  // add 1 for terminator null
    }

    if (fSetLengthToRealStringLength)
    {
      fLength = strlen(reinterpret_cast<const char *>(aText));
      fChar[fLength] = '\0';  // Put end character after string
    }
    else
    {
      fLength = len;
      fReservedLength = fLength + 1;
    }
    strncpy(reinterpret_cast<char *>(fChar), reinterpret_cast<const char *>(aText), fLength);
    fChar[fReservedLength - 1] = '\0';  // Kun riviä ei ollut jätti joskus roskaa
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aChar Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiString &NFmiString::Add(const unsigned char *aChar)
{
  try
  {
    if (fReservedLength <=
        static_cast<long>(fLength + strlen(reinterpret_cast<const char *>(aChar))))
    {
      fReservedLength = fLength + strlen(reinterpret_cast<const char *>(aChar)) + 1;
      auto *aHelp = new unsigned char[fReservedLength];
      strcpy(reinterpret_cast<char *>(aHelp), reinterpret_cast<char *>(fChar));
      delete[] static_cast<unsigned char *>(fChar);
      fChar = aHelp;
    }
    strcat(reinterpret_cast<char *>(fChar), reinterpret_cast<const char *>(aChar));
    fLength += strlen(reinterpret_cast<const char *>(aChar));

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theChar Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::TrimR(unsigned char theChar)
{
  try
  {
    int len = strlen(reinterpret_cast<char *>(fChar));

    /*
     * Workaround for MSVC bug:
     *
     * If len is 0, after 'while' loop we get calculation -1 + 1
     * which for some reason has the result 4294967295, which
     * then causes the the calling program to crash.
     *
     * This happens only with 64bit MSVC 2008, release build.
     */

    if (len == 0)
      return;

    while (fChar[--len] == theChar)
      ;

    fChar[len + 1] = '\0';
    fLength = len + 1;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theChar Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::TrimL(unsigned char theChar)
{
  try
  {
    int len = 0;

    while (fChar[len] == theChar)
      len++;

    if (len)
    {
      Set(&fChar[len], (fLength - len));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theSize Undocumented
 * \param theChar Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::FillR(long theSize, unsigned char theChar)
{
  try
  {
    // Mika: Jostain syystä g++ kääntää väärin ilman castia
    if (theSize > static_cast<long>(fLength))
    {
      unsigned char *theCharBuffer;

      theCharBuffer = new unsigned char[(theSize - fLength) + 1];

      for (unsigned long i = 0; i < (theSize - fLength); i++)
      {
        theCharBuffer[i] = theChar;
        theCharBuffer[i + 1] = '\0';
      }

      Add(theCharBuffer);

      delete[] theCharBuffer;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aChar Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::SetCharPtr(unsigned char *aChar)
{
  try
  {
    Set(aChar, strlen(reinterpret_cast<char *>(aChar)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fFmi Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiString::IsLessThan(const NFmiSortable &fFmi) const
{
  try
  {
    return (strcmp(reinterpret_cast<char *>(fChar),
                   (reinterpret_cast<char *>((static_cast<const NFmiString &>(fFmi)).fChar))) < 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param len Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::SizeCheck(unsigned long len) const
{
  try
  {
    if ((len > GetLen()) || (len <= 0))
    {
      throw Fmi::Exception(BCP, "NFmiString::SizeCheck - out-of-bounds");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param firstChar Undocumented
 * \param length Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiString::GetChars(unsigned long firstChar, unsigned long length) const
{
  try
  {
    NFmiString back(length);

    firstChar--;

    unsigned long test = (back.fLength < GetLen()) ? back.fLength : GetLen();

    // for (unsigned long i=0; i<test; i++)
    for (unsigned long i = 0; i + firstChar < fLength && i < length;
         i++)  // 14.10.2002/Viljo, ylivuodon korjaus
      back.fChar[i] = fChar[i + firstChar];

    for (unsigned long j = test; j < length; j++)
      back.fChar[j] = ' ';

    back.fChar[back.fLength] = '\0';

    return back;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param firstChar Undocumented
 * \param length Undocumented
 * \return Undocumented
 * \todo Should return std::shared_ptr
 */
// ----------------------------------------------------------------------

char *NFmiString::GetCharsPtr(unsigned long firstChar, unsigned long length) const
{
  try
  {
    SizeCheck(firstChar);
    SizeCheck(firstChar + length - 1);

    auto *back = new NFmiString(length);

    firstChar--;
    for (unsigned long i = 0; i < back->fLength; i++)
      back->fChar[i] = fChar[i + firstChar];
    back->fChar[back->fLength] = '\0';

    return reinterpret_cast<char *>(back->fChar);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Searches the wanted longer string (fromChar), but replaces only the first char!
 * \param fromChar Undocumented
 * \param toChar Undocumented
 * \returning false, if fromChar not found, others atleast one mark replaced and returning true
 */
// ----------------------------------------------------------------------

bool NFmiString::ReplaceChars(const unsigned char *fromChar, const unsigned char *toChar)
{
  try
  {
    unsigned long pos = 1;
    pos = Search(fromChar, pos);
    bool replaceableCharFound = pos > 0;
    while (pos > 0)
    {
      fChar[pos - 1] = *toChar;
      pos = Search(fromChar, ++pos);
    }
    return replaceableCharFound;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Searches the wanted string (fromChars) and replaces it to the given string (toChars).
 * - Replacement done in maximum to the given number of instances (see third parameter
 * replacementsMaxCount)
 * By default replaces all the occurences of 'fromChar' to 'toChars'.
 *
 * \param fromChar: characters to replace
 * \param toChar: replacement characters
 * \param replacementsMaxCount: maximum number of replacement, by default kUnsignedLongMissing
 * \returning false in case fromChar not found or empty or this NFmiString is empty, others true
 */
// ----------------------------------------------------------------------

bool NFmiString::ReplaceChars2(const unsigned char *fromChars,
                               const unsigned char *toChars,
                               unsigned long replacementsMaxCount)
{
  try
  {
    NFmiString fromStr(fromChars), toStr(toChars);
    if (fromStr.GetLen() == 0)
      return false;
    unsigned long pos1 = 1, pos = 1, counter = 0;
    if (replacementsMaxCount == 0)
      return true;
    counter++;
    bool returnBool = false;
    NFmiString str;

    for (pos = Search(fromChars, pos1); pos >= pos1; counter++)
    {
      if (pos < 1)
        break;
      returnBool = true;
      str += GetChars(pos1, pos - pos1);
      str += toStr;
      pos1 = pos + fromStr.GetLen();
      if (counter >= replacementsMaxCount)
        break;
      pos = Search(fromChars, pos1);
    }

    if (returnBool)
    {
      if (pos1 <= GetLen())
        str += GetChars(pos1, GetLen() - pos1 + 1);
      *this = str;
    }
    return returnBool;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param searChar Undocumented
 * \param fromPos Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiString::Search(const unsigned char *searChar, unsigned long fromPos) const
{
  try
  {
    if (fromPos < 1 || fromPos > fLength)
      return 0;

    unsigned char *findChar;
    findChar = reinterpret_cast<unsigned char *>(strstr(
        reinterpret_cast<char *>(fChar + fromPos - 1), reinterpret_cast<const char *>(searChar)));

    if (findChar == nullptr)
      return 0;

    return static_cast<unsigned long>(findChar - fChar + 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param searChar Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiString::SearchLast(const unsigned char *searChar) const
{
  try
  {
    unsigned char *findChar;
    unsigned char *lastChar = nullptr;

    findChar = reinterpret_cast<unsigned char *>(
        strstr(reinterpret_cast<char *>(fChar), reinterpret_cast<const char *>(searChar)));

    while (findChar)
    {
      lastChar = findChar;
      findChar = reinterpret_cast<unsigned char *>(
          strstr(reinterpret_cast<char *>(findChar + 1), reinterpret_cast<const char *>(searChar)));
    }

    if (lastChar == nullptr)
    {
      return 0;
    }

    return static_cast<unsigned long>(lastChar - fChar + 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Toimii kuten SearchLast(unsigned char*), mutta käyttäjä voi rajoittaa
 * haun antamalla limit-arvon, jota pidemmälle hakua ei uloteta.
 *
 * \param unsigned char *searchChar, haettava merkki
 * \param unsigned long limit, tutkittavien merkkien määrä eli rajoitus,
 *  jota pitemmälle hakua ei uloteta
 * \return löydetyn merkin indeksi, 0 ellei annettua merkkiä löydetä
 */
// ----------------------------------------------------------------------
unsigned long NFmiString::SearchLast(const unsigned char *searchChar, unsigned long limit) const
{
  try
  {
    NFmiString str(*this);
    if (GetLen() > limit)
      str = str.GetChars(1, limit);

    return str.SearchLast(searchChar);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param FmiChar The character string to be copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiString &NFmiString::operator=(const char *FmiChar)
{
  try
  {
    if (FmiChar)
      Set(reinterpret_cast<const unsigned char *>(FmiChar),
          static_cast<unsigned long>(strlen(static_cast<const char *>(FmiChar))));
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param FmiChar The character string to be copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiString &NFmiString::operator=(const unsigned char *FmiChar)
{
  try
  {
    if (FmiChar)
      Set(static_cast<const unsigned char *>(FmiChar),
          static_cast<unsigned long>(strlen(reinterpret_cast<const char *>(FmiChar))));
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param aChar Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiString &NFmiString::operator+=(const char aChar)
{
  try
  {
    char table[2];
    table[0] = aChar;
    table[1] = '\0';
    NFmiString str(table);
    *this += str;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the string to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiString::Write(std::ostream &file) const
{
  try
  {
    file << fLength << " " << reinterpret_cast<char *>(fChar) << std::endl;

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiString::Read(std::istream &file)
{
  try
  {
    unsigned long theLength;
    char space;

    file >> theLength;
    // Safety against an obsene string size
    if (file.fail())
      throw Fmi::Exception(BCP,
                           "NFmiString::Read string-length read failed, don't read the string");
    file.get(space);

    Set(reinterpret_cast<const unsigned char *>(""), theLength);

    file.read(reinterpret_cast<char *>(fChar), theLength);
    fChar[fReservedLength - 1] = '\0';

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------
#ifndef UNIX
void NFmiString::UpperCase()
{
  try
  {
    setlocale(LC_ALL, "Finnish");
    ::_strupr((char *)fChar);
    setlocale(LC_ALL, "C");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#else
void NFmiString::UpperCase()
{
  try
  {
    char *p;
    p = reinterpret_cast<char *>(fChar);
    while (*p != '\0')
    {
      *p = NFmiStringTools::toupperfi(*p);
      p++;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#endif

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------
#ifndef UNIX
void NFmiString::LowerCase()
{
  try
  {
    setlocale(LC_ALL, "Finnish");
    ::_strlwr((char *)fChar);
    setlocale(LC_ALL, "C");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#else
void NFmiString::LowerCase()
{
  try
  {
    char *p;
    p = reinterpret_cast<char *>(fChar);
    while (*p != '\0')
    {
      *p = NFmiStringTools::tolowerfi(*p);
      p++;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
#endif

// ----------------------------------------------------------------------
/*!
 * \param theUpperIndex Undocumented
 */
// ----------------------------------------------------------------------

void NFmiString::FirstCharToUpper(unsigned long theUpperIndex)
{
  try
  {
    if (theUpperIndex < fLength)
    {
      setlocale(LC_ALL, "Finnish");
      NFmiString theLetter(fChar);
#ifndef UNIX
      char *theChar = ::_strupr((char *)theLetter);

#else
      auto *theChar = static_cast<char *>(theLetter);
      *theChar = toupper(*theChar);
#endif
      fChar[theUpperIndex] = theChar[theUpperIndex];
      setlocale(LC_ALL, "C");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiString::FirstCharIsUpper() const
{
  try
  {
    char c = fChar[0];
    if ((c >= 'A' && c <= 'Z') || c == '\304' || c == '\326' || c == '\305')
      return true;

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theChar Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiString::CharCount(unsigned char theChar) const
{
  try
  {
    int count = 0;
    for (unsigned long i = 0; i < fLength; i++)
    {
      if (fChar[i] == theChar)
        count++;
    }
    return count;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Replaces chars with newChars from beginning of the fromIndex.
 * Does not remove characters, i.e. if newChars == "", returns false.
 *
 * Example:
 * \code
 *		NFmiString str("123456789");
 *		str.Replace("abc",2);
 * \endcode
 *
 * Outcome will be:
 * \code
 *		str = "12abc6789"
 * \endcode
 *
 * \param newChars Undocumented
 * \param fromIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiString::Replace(const NFmiString &newChars, unsigned long fromIndex)
{
  try
  {
    if (GetLen() < fromIndex + newChars.GetLen() || newChars.GetLen() == 0)
      return false;

    const unsigned char *replStr = newChars.GetCharPtr();

    for (unsigned int i = 0; i < newChars.GetLen(); i++)
      fChar[fromIndex + i] = replStr[i];

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return void
 */
// ----------------------------------------------------------------------

void NFmiString::RemoveExtraSpaces()
{
  try
  {
    NFmiString tmpStr;
    TrimR();
    TrimL();
    long num = 0;
    for (unsigned long i = 0; i < fLength; i++)
    {
      fChar[i] == ' ' ? num++ : num = 0;
      if (num < 2)
        tmpStr += GetChars(i + 1, 1);
    }
    *this = tmpStr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return void
 */
// ----------------------------------------------------------------------

void NFmiString::FirstInWordToUpper()
{
  try
  {
    LowerCase();
    FirstCharToUpper();
    for (unsigned long i = 2; i < fLength; i++)
    {
      if (fChar[i - 1] == ' ' && fChar[i] >= 'a')
        FirstCharToUpper(i);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the string
 */
// ----------------------------------------------------------------------

std::size_t NFmiString::HashValue() const
{
  try
  {
    std::string name(CharPtr());
    return Fmi::hash_value(name);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
