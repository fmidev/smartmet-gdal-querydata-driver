// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiStringTools
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4309)  // poistaa VC++ kääntäjän varoituksia (256 kpl) kun alustetaan
                                 // iso_8859_1_upper-taulukkoa (warning C4309: 'initializing' :
                                 // truncation of constant value)
#endif

#include "NFmiStringTools.h"
#include <macgyver/Exception.h>
#include <iostream>

#include <cctype>
#include <cstdlib>
#include <locale>

using namespace std;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief ISO-8859-1 lower case conversion table
 */
// ----------------------------------------------------------------------

const unsigned char iso_8859_1_lower[256] = {0,
                                             1,
                                             2,
                                             3,
                                             4,
                                             5,
                                             6,
                                             7,
                                             8,
                                             9,
                                             10,
                                             11,
                                             12,
                                             13,
                                             14,
                                             15,
                                             16,
                                             17,
                                             18,
                                             19,
                                             20,
                                             21,
                                             22,
                                             23,
                                             24,
                                             25,
                                             26,
                                             27,
                                             28,
                                             29,
                                             30,
                                             31,
                                             32,
                                             33,
                                             34,
                                             35,
                                             36,
                                             37,
                                             38,
                                             39,
                                             40,
                                             41,
                                             42,
                                             43,
                                             44,
                                             45,
                                             46,
                                             47,
                                             48,
                                             49,
                                             50,
                                             51,
                                             52,
                                             53,
                                             54,
                                             55,
                                             56,
                                             57,
                                             58,
                                             59,
                                             60,
                                             61,
                                             62,
                                             63,
                                             64,
                                             97,
                                             98,
                                             99,
                                             100,
                                             101,  // 65-90 = A-Z --> 97-122
                                             102,
                                             103,
                                             104,
                                             105,
                                             106,
                                             107,
                                             108,
                                             109,
                                             110,
                                             111,
                                             112,
                                             113,
                                             114,
                                             115,
                                             116,
                                             117,
                                             118,
                                             119,
                                             120,
                                             121,
                                             122,
                                             91,
                                             92,
                                             93,
                                             94,
                                             95,
                                             96,
                                             97,
                                             98,
                                             99,
                                             100,
                                             101,
                                             102,
                                             103,
                                             104,
                                             105,
                                             106,
                                             107,
                                             108,
                                             109,
                                             110,
                                             111,
                                             112,
                                             113,
                                             114,
                                             115,
                                             116,
                                             117,
                                             118,
                                             119,
                                             120,
                                             121,
                                             122,
                                             123,
                                             124,
                                             125,
                                             126,
                                             127,
                                             128,
                                             129,
                                             130,
                                             131,
                                             132,
                                             133,
                                             134,
                                             135,
                                             136,
                                             137,
                                             138,
                                             139,
                                             140,
                                             141,
                                             142,
                                             143,
                                             144,
                                             145,
                                             146,
                                             147,
                                             148,
                                             149,
                                             150,
                                             151,
                                             152,
                                             153,
                                             154,
                                             155,
                                             156,
                                             157,
                                             158,
                                             159,
                                             160,
                                             161,
                                             162,
                                             163,
                                             164,
                                             165,
                                             166,
                                             167,
                                             168,
                                             169,
                                             170,
                                             171,
                                             172,
                                             173,
                                             174,
                                             175,
                                             176,
                                             177,
                                             178,
                                             179,
                                             180,
                                             181,
                                             182,
                                             183,
                                             184,
                                             185,
                                             186,
                                             187,
                                             188,
                                             189,
                                             190,
                                             191,
                                             224,
                                             225,
                                             226,
                                             227,
                                             228,
                                             229,
                                             230,
                                             231,  // 192-222 -->
                                                   // 224-254
                                             232,
                                             233,
                                             234,
                                             235,
                                             236,
                                             237,
                                             238,
                                             239,
                                             240,
                                             241,  // except 215 (times)
                                             242,
                                             243,
                                             244,
                                             245,
                                             246,
                                             215,
                                             248,
                                             249,
                                             250,
                                             251,
                                             252,
                                             253,
                                             254,
                                             223,
                                             224,
                                             225,
                                             226,
                                             227,
                                             228,
                                             229,
                                             230,
                                             231,
                                             232,
                                             233,
                                             234,
                                             235,
                                             236,
                                             237,
                                             238,
                                             239,
                                             240,
                                             241,
                                             242,
                                             243,
                                             244,
                                             245,
                                             246,
                                             247,
                                             248,
                                             249,
                                             250,
                                             251,
                                             252,
                                             253,
                                             254,
                                             255};

// ----------------------------------------------------------------------
/*!
 * \brief ISO-8859-1 upper case conversion table
 */
// ----------------------------------------------------------------------

const unsigned char iso_8859_1_upper[256] = {0,
                                             1,
                                             2,
                                             3,
                                             4,
                                             5,
                                             6,
                                             7,
                                             8,
                                             9,
                                             10,
                                             11,
                                             12,
                                             13,
                                             14,
                                             15,
                                             16,
                                             17,
                                             18,
                                             19,
                                             20,
                                             21,
                                             22,
                                             23,
                                             24,
                                             25,
                                             26,
                                             27,
                                             28,
                                             29,
                                             30,
                                             31,
                                             32,
                                             33,
                                             34,
                                             35,
                                             36,
                                             37,
                                             38,
                                             39,
                                             40,
                                             41,
                                             42,
                                             43,
                                             44,
                                             45,
                                             46,
                                             47,
                                             48,
                                             49,
                                             50,
                                             51,
                                             52,
                                             53,
                                             54,
                                             55,
                                             56,
                                             57,
                                             58,
                                             59,
                                             60,
                                             61,
                                             62,
                                             63,
                                             64,
                                             65,
                                             66,
                                             67,
                                             68,
                                             69,
                                             70,
                                             71,
                                             72,
                                             73,
                                             74,
                                             75,
                                             76,
                                             77,
                                             78,
                                             79,
                                             80,
                                             81,
                                             82,
                                             83,
                                             84,
                                             85,
                                             86,
                                             87,
                                             88,
                                             89,
                                             90,
                                             91,
                                             92,
                                             93,
                                             94,
                                             95,
                                             96,
                                             65,
                                             66,
                                             67,  // 97-122 = a-z --> 65-90
                                             68,
                                             69,
                                             70,
                                             71,
                                             72,
                                             73,
                                             74,
                                             75,
                                             76,
                                             77,
                                             78,
                                             79,
                                             80,
                                             81,
                                             82,
                                             83,
                                             84,
                                             85,
                                             86,
                                             87,
                                             88,
                                             89,
                                             90,
                                             123,
                                             124,
                                             125,
                                             126,
                                             127,
                                             128,
                                             129,
                                             130,
                                             131,
                                             132,
                                             133,
                                             134,
                                             135,
                                             136,
                                             137,
                                             138,
                                             139,
                                             140,
                                             141,
                                             142,
                                             143,
                                             144,
                                             145,
                                             146,
                                             147,
                                             148,
                                             149,
                                             150,
                                             151,
                                             152,
                                             153,
                                             154,
                                             155,
                                             156,
                                             157,
                                             158,
                                             159,
                                             160,
                                             161,
                                             162,
                                             163,
                                             164,
                                             165,
                                             166,
                                             167,
                                             168,
                                             169,
                                             170,
                                             171,
                                             172,
                                             173,
                                             174,
                                             175,
                                             176,
                                             177,
                                             178,
                                             179,
                                             180,
                                             181,
                                             182,
                                             183,
                                             184,
                                             185,
                                             186,
                                             187,
                                             188,
                                             189,
                                             190,
                                             191,
                                             192,
                                             193,
                                             194,
                                             195,
                                             196,
                                             197,
                                             198,
                                             199,
                                             200,
                                             201,
                                             202,
                                             203,
                                             204,
                                             205,
                                             206,
                                             207,
                                             208,
                                             209,
                                             210,
                                             211,
                                             212,
                                             213,
                                             214,
                                             215,
                                             216,
                                             217,
                                             218,
                                             219,
                                             220,
                                             221,
                                             222,
                                             223,
                                             192,
                                             193,
                                             194,
                                             195,
                                             196,
                                             197,  // 224-254 -->
                                                   // 192-222
                                             198,
                                             199,
                                             200,
                                             201,
                                             202,
                                             203,
                                             204,
                                             205,
                                             206,
                                             207,  // except 247 (divide)
                                             208,
                                             209,
                                             210,
                                             211,
                                             212,
                                             213,
                                             214,
                                             247,
                                             216,
                                             217,
                                             218,
                                             219,
                                             220,
                                             221,
                                             222,
                                             255};

// ----------------------------------------------------------------------
/*!
 * \brief Convert a hexadecimal character to a decimal value
 *
 * The value is undefined if the character is not hexadecimal
 * \param theHex The hexadecimal character
 * \return The decimal value
 */
// ----------------------------------------------------------------------

int todec(unsigned char theHex)
{
  try
  {
    if (theHex >= '0' && theHex <= '9')
      return (theHex - '0');
    else if (theHex >= 'a' && theHex <= 'f')
      return (theHex - 'a' + 10);
    else
      return (theHex - 'A' + 10);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace NFmiStringTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Platform independent toupper function in Finnish
 */
// ----------------------------------------------------------------------

char toupperfi(char theChar)
{
  try
  {
    return iso_8859_1_upper[static_cast<unsigned char>(theChar)];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Platform independent tolower function in Finnish
 */
// ----------------------------------------------------------------------

char tolowerfi(char theChar)
{
  try
  {
    return iso_8859_1_lower[static_cast<unsigned char>(theChar)];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Convert the first character to upper case. The code is stolen from
 * NFmiString.
 *
 * \param theString The string to be converted.
 * \return Reference to the converted string.
 */
// ----------------------------------------------------------------------

std::string &FirstCharToUpper(std::string &theString)
{
  try
  {
    if (!theString.empty())
      theString[0] = toupperfi(theString[0]);

    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------.
/*!
 * Convert the given string to lower case.
 *
 * \param theString The string to be converted.
 * \return Reference to the converted string.
 */
// ----------------------------------------------------------------------

std::string &LowerCase(std::string &theString)
{
  try
  {
    for (char &i : theString)
      i = tolowerfi(i);

    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------.
/*!
 * Convert the given string to upper case.
 *
 * \param theString The string to be converted.
 * \return Reference to the converted string.
 */
// ----------------------------------------------------------------------

std::string &UpperCase(std::string &theString)
{
  try
  {
    for (char &i : theString)
      i = toupperfi(i);

    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Replace characters.
 *
 */
// ----------------------------------------------------------------------

std::string &ReplaceChars(std::string &theString, char fromChar, char toChar)
{
  try
  {
    for (char &i : theString)
    {
      if (i == fromChar)
        i = toChar;
    }
    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Code take from http://www.codeguru.com/cpp/tic/tic0180.shtml
std::string &ReplaceAll(std::string &theString,
                        const std::string &fromStr,
                        const std::string &toStr)
{
  try
  {
    // Jos fromStr on tyhjä ja toStr ei, lisää replace-komento stringin alkuun aina toStr:n.
    // Sitten found-indeksi kasvaa yhdellä ja indeksin 1 kohdalle lisätään toStr. Ja tämä jatkuu
    // ikuisesti...
    // Siksi jos fromStr on tyhjä, ei tehdä mitään.
    if (fromStr.empty() == false)
    {
      string::size_type advanceThis = toStr.length();  // etsinnässä pitää edetä korvaus stringin
                                                       // verran eteenpäin, muuten voi jäädä
                                                       // ikilooppiin (esim. '%' -> '%C4%')
      if (advanceThis == 0)
        advanceThis++;  // paitsi jos korvaus stringi oli tyhjä, pitää edetä 1:llä, muuten voi jäädä
                        // ikilooppiin
      string::size_type found = theString.find(fromStr);
      while (found != string::npos)
      {
        theString.replace(found, fromStr.length(), toStr);
        found = theString.find(fromStr, found + advanceThis);
      }
    }
    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * For some reason NFmiString IsValue is very stupid. This replicates
 * the behaviour.
 */
// ----------------------------------------------------------------------

bool IsValue(const std::string &theString)
{
  try
  {
    return (!theString.empty());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Trim the string from the left
 */
// ----------------------------------------------------------------------

std::string &TrimL(std::string &theString, char theChar)
{
  try
  {
    unsigned int pos = 0;
    if (theChar == ' ')
    {
#ifdef PERKELEEN_296
      while (pos < theString.size() && isspace(theString[pos]))
#else
      while (pos < theString.size() && ::isspace(static_cast<unsigned char>(theString[pos])))
      // while(pos<theString.size() && ::isspace(theString[pos]))
#endif
        pos++;
    }
    else
    {
      while (pos < theString.size() && theString[pos] == theChar)
        pos++;
    }
    if (pos > 0)
      theString = theString.substr(pos);

    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Trim the string from the right
 */
// ----------------------------------------------------------------------

std::string &TrimR(std::string &theString, char theChar)
{
  try
  {
    int pos = theString.size() - 1;
    if (theChar == ' ')
    {
#ifdef PERKELEEN_296
      while (pos >= 0 && isspace(theString[pos]))
#else
      // while(pos>=0 && ::isspace(theString[pos]))
      while (pos >= 0 && ::isspace(static_cast<unsigned char>(theString[pos])))
#endif
        pos--;
    }
    else
    {
      while (pos >= 0 && theString[pos] == theChar)
        pos--;
    }
    if (pos != static_cast<int>(theString.size() - 1))
      theString.resize(pos + 1);

    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Trim the string from left and right
 */
// ----------------------------------------------------------------------

std::string &Trim(std::string &theString, char theChar)
{
  try
  {
    TrimL(theString, theChar);
    TrimR(theString, theChar);
    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Poistaa stringistä alku+loppu whitespacet.
 * Sanojen välistä poistetaan spacet ja tabit paitsi jättää/lisää yhden spacen.
 * Jos sanojen välillä on ainoastaan tabulaattoreita, korvaa ne yhdellä spacella.
 * <cr> ja <lf> ei poisteta sanojen välistä (paitsi alusta ja lopusta).
 */
// ----------------------------------------------------------------------

std::string TrimAll(std::string &theString, bool replaceInsideNewlinesWithSpace)
{
  try
  {
    std::string tmp(theString);

    // trimmataan stringin alusta ja lopusta whitespacet pois
    NFmiStringTools::Trim(tmp, ' ');
    NFmiStringTools::Trim(tmp, '\t');
    NFmiStringTools::Trim(tmp, '\r');
    NFmiStringTools::Trim(tmp, '\n');
    std::string result;
    bool wordBreak = false;
    for (char i : tmp)
    {
      if (replaceInsideNewlinesWithSpace ? (i == ' ' || i == '\t' || i == '\r' || i == '\n')
                                         : (i == ' ' || i == '\t'))
      {
        if (wordBreak == false)
        {
          wordBreak = true;
          result.push_back(' ');  // laitetaan aina sanan perään yksi space
        }
      }
      else
      {
        wordBreak = false;
        result.push_back(i);
      }
    }
    theString.swap(result);
    return theString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Read an input stream into a string
 *
 * \param is The input stream to read from
 * \return The read string
 */
// ----------------------------------------------------------------------

const std::string ReadFile(std::istream &is)
{
  try
  {
    string ret;
    const int bufsize = 1024;
    char buffer[bufsize];
    while (!is.eof() && !is.fail())
    {
      is.read(buffer, bufsize);
      ret.append(buffer, is.gcount());
    }
    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Encode a string
 *
 * The encoder converts all non-alphanumeric characters
 * except '_' to RFC 1738 form "%AB". Strictly speaking
 * the RFC allows characters "$-_.+!*'()," to be unencoded
 * in addresses.
 *
 * \param theString The string to encode
 * \return The encoded string
 */
// ----------------------------------------------------------------------

const std::string UrlEncode(const std::string &theString)
{
  try
  {
    static const char *decimals = "0123456789ABCDEF";
    string ret;
    for (char ch : theString)
    {
      if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
          (ch == '_'))
      {
        ret += ch;
      }
      else
      {
        ret += '%';
        ret += decimals[(ch >> 4) & 0xF];
        ret += decimals[ch & 0xF];
      }
    }

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Decode a string
 *
 * Performs RFC 1738 decoding, \e plus decodes the '+' character
 * as a space for historical reasons. The space really should
 * be encoded as "%20" instead according to the standards.
 *
 * \param theString The string to decode
 * \return The encoded string
 */
// ----------------------------------------------------------------------

const std::string UrlDecode(const std::string &theString)
{
  try
  {
    string ret;
    for (string::size_type i = 0; i < theString.size(); i++)
    {
      const unsigned char ch = theString[i];
      if (ch == '+')
        ret += ' ';
      else if (ch != '%')
        ret += ch;
      else if (i + 2 < theString.size())
      {
        const unsigned char ch1 = theString[++i];
        const unsigned char ch2 = theString[++i];
        ret += static_cast<char>(((todec(ch1) << 4) | todec(ch2)));
      }
      else
      {
        // The string runs out without enough hexadecimals - we ignore
        // the encoding and return the string as is
        ret += ch;
      }
    }

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse QUERY_STRING
 *
 * Throws if QUERY_STRING is not set
 *
 * \return Map of variable name-value pairs
 */
// ----------------------------------------------------------------------

const std::map<std::string, std::string> ParseQueryString()
{
  try
  {
    const char *env = getenv("QUERY_STRING");
    if (env == nullptr)
      throw Fmi::Exception(BCP, "ParseQueryString:: Environment variable QUERY_STRING is not set");

    return ParseQueryString(env);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse cgi-variables from a querystring
 *
 * An optional QUERYSTRING= is stripped from the start before
 * parsing is done.
 *
 * The querystring is allowed to have name-value pairs of the form
 *
 *   - name=value
 *   - name=
 *
 * In the latter case the value of the variable will be an empty string
 *
 * \param theQueryString The query string to parse
 * \return map of variable name-value pairs
 */
// ----------------------------------------------------------------------

const std::map<std::string, std::string> ParseQueryString(const std::string &theQueryString)
{
  try
  {
    // Erase optional leading "QUERYSTRING="
    string str(theQueryString);
    if (str.substr(0, 12) == "QUERYSTRING=")
      str.erase(0, 12);

    // Split at "&"-characters

    const vector<string> vars = Split<vector<string> >(str, "&");

    // Process each assignment

    map<string, string> ret;

    for (const auto &var : vars)
    {
      const vector<string> parts = Split<vector<string> >(var, "=");
      if (parts.size() == 1)
        ret.insert(make_pair(UrlDecode(parts.front()), string("")));
      else if (parts.size() == 2)
        ret.insert(make_pair(UrlDecode(parts.front()), UrlDecode(parts.back())));
      else
      {
      }  // we ignore the invalid setting
    }

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Split string into vector of strings
 */
// ----------------------------------------------------------------------

const std::vector<std::string> Split(const std::string &theString, const std::string &theSeparator)
{
  try
  {
    return Split<std::vector<std::string> >(theString, theSeparator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract basename from filename
 *
 * Note:
 *
 * basename foo/bar  = bar
 * basename foo/     = foo
 *
 */
// ----------------------------------------------------------------------

const std::string Basename(const std::string &theFile)
{
  try
  {
    string::size_type pos = theFile.rfind('/');
    if (pos == string::npos)
      return theFile;

    string base = theFile.substr(pos + 1);
    if (!base.empty())
      return base;

    return theFile.substr(0, pos);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract suffix from filename
 */
// ----------------------------------------------------------------------

const std::string Suffix(const std::string &theFile)
{
  try
  {
    string::size_type pos = theFile.rfind('.');
    if (pos == string::npos)
      return "";
    else
      return theFile.substr(pos + 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiStringTools

// ======================================================================
