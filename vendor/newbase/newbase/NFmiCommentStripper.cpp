// ======================================================================
/*!
 * \file NFmiCommentStripper.cpp
 * \brief Implementation of class NFmiCommentStripper
 */
// ======================================================================
/*!
 * \class NFmiCommentStripper
 *
 * Luokka poistaa C++ ja #-kommenttimerkit datasta ja palauttaa
 * riisutun merkkijonon. C++ -kommentit löydetään ja poistetaan myös
 * keskeltä riviä,  #-kommentit vain rivin alusta
 *
 */
// ======================================================================

#include "NFmiCommentStripper.h"

#include <macgyver/Exception.h>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theFileName Undocumented
 * \param stripPound Undocumented
 * \param stripDoubleSlash Undocumented
 * \param stripSlashAst Undocumented
 * \param stripSlashAstNested Undocumented
 */
// ----------------------------------------------------------------------

NFmiCommentStripper::NFmiCommentStripper(const std::string& theFileName,
                                         bool stripPound,
                                         bool stripDoubleSlash,
                                         bool stripSlashAst,
                                         bool stripSlashAstNested,
                                         bool stripEndOfLineSpaces)
    : itsMessage(),
      itsString(),
      itsFileName(""),
      fStripPound(stripPound),
      fStripNested(stripSlashAstNested),
      fStripDoubleSlash(stripDoubleSlash),
      fStripSlashAst(stripSlashAst),
      fStripEndOfLineSpaces(stripEndOfLineSpaces)
{
  ReadAndStripFile(theFileName);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param stripPound Undocumented
 * \param stripDoubleSlash Undocumented
 * \param stripSlashAst Undocumented
 * \param stripSlashAstNested Undocumented
 */
// ----------------------------------------------------------------------

NFmiCommentStripper::NFmiCommentStripper(bool stripPound,
                                         bool stripDoubleSlash,
                                         bool stripSlashAst,
                                         bool stripSlashAstNested,
                                         bool stripEndOfLineSpaces)
    : itsMessage(),
      itsString(),
      itsFileName(""),
      fStripPound(stripPound),
      fStripNested(stripSlashAstNested),
      fStripDoubleSlash(stripDoubleSlash)  // oli aiemmin asetettu arvo stripSlashAst
      ,
      fStripSlashAst(stripSlashAst),
      fStripEndOfLineSpaces(stripEndOfLineSpaces)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStripper The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCommentStripper::NFmiCommentStripper(const NFmiCommentStripper& theStripper)
    : itsMessage(theStripper.itsMessage),
      itsString(theStripper.itsString),
      itsFileName(theStripper.itsFileName),
      fStripPound(theStripper.fStripPound),
      fStripNested(theStripper.fStripNested),
      fStripDoubleSlash(theStripper.fStripDoubleSlash),
      fStripSlashAst(theStripper.fStripSlashAst)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::ReadFile(const string& theFileName)
{
  try
  {
    ifstream in(theFileName.c_str(), std::ios::binary);
#ifndef UNIX
    itsFileName = theFileName.substr(theFileName.rfind("\\") + 1);
#else
    itsFileName = theFileName;
#endif
    if (in)
    {
      itsString = "";  // nollaa lopullinen stringi

      string filt_elem1("//");
      string rowbuffer, bigstring;

      // luetaan tiedostoa rivi kerrallaan ja testataan löytyykö yhden rivin kommentteja

      while (getline(in, rowbuffer))
      {
        if (!rowbuffer.empty())
        {
          if (fStripPound && rowbuffer[0] == '#')
            continue;
          if (rowbuffer[rowbuffer.size() - 1] == '\r')
            rowbuffer.resize(rowbuffer.size() - 1);
        }
        bigstring += rowbuffer + "\n";
      }
      itsString = bigstring;
      in.close();
    }
    else
    {
      itsMessage = "file not found = " + theFileName;
      return false;
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
 * Kuten ReadFile mutta tarkistaa samalla onko tekstissä käytetty theOptionText-
 * alkuisia sanoja, joita lehtiohjelmassa käytetään ehdollisina direktiiveinä,
 * esim #ifConditionHour13. Tästä kun saa tiedon mitä käytössä on, ei tarvitse rasittaa
 * ohjelmaa raskaammilla varsinaisilla srippauksilla, joita tuntitapauksessa jo
 * saattaisi olla 24.
 *
 * \param theFileName Undocumented
 *
 * \param theOptionText
 * tarkistettava sana, lehtiohjelman tapauksessa #ifCondition
 *
 * \param places
 * tähän annettuun settiin ne kerätään, lehtiohjelma aluksi #ifConditionHour1 -> 23
 *
 * \return
 * false: tiedosto ei löydy
 *        itsMessage käytettävissä virheilmoituksiin
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::ReadFileCheckingOptions(const string& theFileName,
                                                  const string& theOptionText,
                                                  set<string>& theOptionTexts)
{
  try
  {
    ifstream in(theFileName.c_str(), std::ios::binary);
#ifndef UNIX
    itsFileName = theFileName.substr(theFileName.rfind("\\") + 1);
#else
    itsFileName = theFileName;
#endif

    if (in)
    {
      itsString = "";  // nollaa lopullinen stringi

      string filt_elem1("//");
      string rowbuffer, bigstring;

      // luetaan tiedostoa rivi kerrallaan ja testataan löytyykö yhden rivin kommentteja

      while (getline(in, rowbuffer))
      {
        if (!rowbuffer.empty())
        {
          string::size_type pos = rowbuffer.find(theOptionText);
          if (pos != string::npos)
          {
            string::size_type lastPos = rowbuffer.find_first_of(" ", pos);
            string option = rowbuffer.substr(pos, lastPos - pos);
            theOptionTexts.insert(option);
          }
          if (fStripPound && rowbuffer[0] == '#')
            continue;
          if (rowbuffer[rowbuffer.size() - 1] == '\r')
            rowbuffer.resize(rowbuffer.size() - 1);
        }
        bigstring += rowbuffer + "\n";
      }
      itsString = bigstring;
      in.close();
    }
    else
    {
      itsMessage = "file not found = " + theFileName;
      return false;
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
 * \param theFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::ReadAndStripFile(const string& theFileName)
{
  try
  {
    if (ReadFile(theFileName))
      return Strip();

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Feature/minor bug: Jos rivin lopussa on space ennen rivinvaihtoa, se
 * siivotaan pois. En muuta toimintaa ainakaan nyt kun sain korjattua tästä
 * aiheutuneen ongelman toisaalla (SmartMetin viewMacrojen tallennus ja luku).
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::Strip()
{
  try
  {
    StripBomMarkersFromStart();
    string filt_elem2("/*"), filt_elem3("*/");
    if (fStripSlashAst)
    {
      if (fStripNested)
      {
        if (!CollectAndStripNested(filt_elem2, filt_elem3))
          return false;
      }
      else
      {
        if (!StripBlocks(filt_elem2, filt_elem3))
          return false;
      }
    }
    if (fStripPound)
    {
      if (!StripPounds())
        return false;
    }
    if (fStripDoubleSlash)
    {
      if (!StripDoubleSlashes())
        return false;
    }

    if (fStripEndOfLineSpaces)
    {
      // Mika: 07.02.2002:
      // Lopuksi poistetaan tarpeeton whitespace:
      // Kun tulee \n vastaan, peruutetaan spacet pois ja jätetään korkeintaan
      // 3 peräkkäistä \n merkkiä
      unsigned int j = 2;

      for (unsigned int i = 2; i < itsString.size(); i++)
      {
        if (itsString[i] != '\n')
          itsString[j++] = itsString[i];
        else
        {
          for (; j > 1 && (itsString[j - 1] == ' ' || itsString[j - 1] == '\t'); j--)
            ;
          if ((j >= 1 && itsString[j - 1] != '\n') || (j >= 2 && itsString[j - 2] != '\n'))
            itsString[j++] = itsString[i];
        }
      }
      itsString.resize(j);
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiCommentStripper::StripBomMarkersFromStart()
{
  try
  {
    const std::string bomMarkers =
        "\xEF\xBB\xBF";  // BOM characters ï»¿ must be given with hexa escape format because this
                         // cpp file is Utf-8 encoded
    auto pos = itsString.find(bomMarkers);
    if (pos == 0)
      itsString = std::string(itsString.begin() + bomMarkers.size(), itsString.end());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theString Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::Strip(const std::string& theString)
{
  try
  {
    itsString = theString;
    return Strip();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theBeginDirective Undocumented
 * \param theEndDirective Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::CollectAndStripNested(const string& theBeginDirective,
                                                const string& theEndDirective)
{
  try
  {
    std::vector<unsigned long> posStartFilts;
    std::vector<unsigned long> posEndFilts;

    if (CollectStringPositions(theBeginDirective, posStartFilts) &&
        CollectStringPositions(theEndDirective, posEndFilts))
    {
      return StripNested(posStartFilts, posEndFilts);
    }
    return false;
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

bool NFmiCommentStripper::StripDoubleSlashes()  // from to endline
{
  try
  {
    string oldString(itsString);
    string newString = "";
    string doubleSlash = "//";
    string::size_type posbigalku = oldString.find(doubleSlash);
    string::size_type posbigloppu;
    //	unsigned int len = theEndFilt.length();

    if (posbigalku == string::npos)
      itsString = oldString;
    else
    {
      while (posbigalku != string::npos)
      {
        posbigalku = oldString.find(doubleSlash);
        posbigloppu = oldString.find('\n', posbigalku);
        newString += oldString.substr(0, posbigalku);
        if (posbigloppu != string::npos)
          oldString = oldString.substr(posbigloppu);
        else
          break;  // This prevents an eternal loop
      }
      itsString = newString;
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
 * \return Undocumented
 * BUG This should strip only if pound-char is first letter in line. Now
 * method works so that pound can be anywhere in line and rest of line
 * is stripped just like with //-comments
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::StripPounds()  // from pound to end of line
{
  try
  {
    string oldString(itsString);
    string newString = "";
    string pound = "#";
    string::size_type posbigalku = oldString.find(pound);
    string::size_type posbigloppu;
    //	unsigned int len = theEndFilt.length();

    if (posbigalku == string::npos)
      itsString = oldString;
    else
    {
      while (posbigalku != string::npos)
      {
        posbigalku = oldString.find(pound);
        posbigloppu = oldString.find('\n', posbigalku);
        newString += oldString.substr(0, posbigalku);
        if (posbigloppu != string::npos)
          oldString = oldString.substr(posbigloppu);
        else
          break;  // This prevents an eternal loop
      }
      itsString = newString;
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
 * \param theBeginDirective Undocumented
 * \param theEndDirective Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::StripBlocks(const string& theBeginDirective,
                                      const string& theEndDirective)
{
  try
  {
    string oldString(itsString);
    string newString = "";
    string::size_type posbigalku = oldString.find(theBeginDirective);
    string::size_type posbigloppu;
    string::size_type len = theEndDirective.length();

    if (posbigalku == string::npos)
      itsString = oldString;
    else
    {
      while (posbigalku != string::npos)
      {
        posbigalku = oldString.find(theBeginDirective);

        if (posbigalku != string::npos)
          posbigloppu = oldString.find(theEndDirective, posbigalku);
        else
          posbigloppu = string::npos;

        if (posbigalku != string::npos && posbigloppu == string::npos)
        {
          itsMessage = "ERROR in " + itsFileName + ": Missing */";
          return false;
        }
        newString += oldString.substr(0, posbigalku);
        if (posbigloppu != string::npos)
          oldString = oldString.substr(posbigloppu + len);
      }
      itsString = newString;
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
 * \param theString Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::StripSubStrings(const string& theString)
{
  try
  {
    string oldString(itsString);
    string newString = "";
    string::size_type posbigalku = oldString.find(theString);
    string::size_type posbigloppu;
    string::size_type len = theString.length();

    if (posbigalku == string::npos)
      itsString = oldString;
    else
    {
      while (posbigalku != string::npos)
      {
        posbigalku = oldString.find(theString);
        posbigloppu = posbigalku + len;
        newString += oldString.substr(0, posbigalku);
        oldString = oldString.substr(posbigloppu);
        posbigalku = oldString.find(theString);
      }
      newString += oldString;
      itsString = newString;
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
 * \param theBeginPositions Undocumented
 * \param theEndPositions Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::StripNested(std::vector<unsigned long> theBeginPositions,
                                      std::vector<unsigned long> theEndPositions)
{
  try
  {
    auto startFiltsInd = theBeginPositions.begin();
    auto endFiltsInd = theEndPositions.begin();
    int posStart, posEnd, startOfErase = 0;
    int eraseSum = 0;
    int level = 0;
    while (startFiltsInd != theBeginPositions.end() || endFiltsInd != theEndPositions.end())
    {
      posStart = startFiltsInd == theBeginPositions.end() ? kShortMissing : *startFiltsInd;
      posEnd = endFiltsInd == theEndPositions.end() ? kShortMissing : *endFiltsInd;

      if (posStart < posEnd)
      {
        level++;
        if (level == 1)
          startOfErase = posStart;
        startFiltsInd++;
      }
      else
      {
        level--;
        if (level < 0)
        {
          itsMessage = "ERROR in " + itsFileName +
                       ": Missing pair to */ after: " + itsString.substr(max(posEnd - 22, 0), 21);
          return false;
        }
        else if (level == 0)
        {
          int len = posEnd + 2 - startOfErase;
          itsString.erase(startOfErase - eraseSum, len);
          eraseSum += len;
        }
        endFiltsInd++;
      }
    }
    if (level > 0)
    {
      itsMessage = "ERROR in " + itsFileName + ": the /*'s exceed the */'s";
      return false;
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
 * \param theSearchString Undocumented
 * \param theResVector Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCommentStripper::CollectStringPositions(const string& theSearchString,
                                                 std::vector<unsigned long>& theResVector)
{
  try
  {
    string aString(itsString);
    string::size_type filtLen = theSearchString.length();
    string::size_type pos = aString.find(theSearchString);
    if (pos == string::npos)
      return true;

    string::size_type sum = 0;
    while (pos != string::npos)
    {
      sum += pos;
      theResVector.push_back(static_cast<unsigned long>(sum));
      aString = aString.substr(pos + filtLen);
      sum += filtLen;
      pos = aString.find(theSearchString);
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
