// ======================================================================
/*!
 * \file NFmiPreProcessor.cpp
 * \brief Implementation of class NFmiPreProcessor
 */
// ======================================================================
/*!
 * \class NFmiPreProcessor
 *
 * The class can be used to delete the following substrings from
 * a string and/or to accomplish the operations:
 *
 *  - to strip lines starting with #
 *  - to strip all characters between and including two given substrings
 *  - to strip from and including // to end of line
 *  - to accomplish directives of type: ifCond(ifnotCond)/else/endif
 *  - to replace strings with other strings
 *  - to include files (to which all these operations are accomplished recursively,
 *                     presuming they are set for the base level)
 *
 * The operations are performed in the order listed above unless explicitly called. In files
 * included, the order is, however, always the one above. All operations are optional (see
 * constructors and setting methods).
 *
 * If the methods return false, an error message can (hopefully) be obtained
 * by GetMessage(). Infinite include loops are prevented by checking the number
 * against kMaxNumOfStripperIncludes (which may need to be increased).
 *
 * Example program:
 *
 * \code
 * NFmiPreProcessor stripper;
 * bool res = stripper.SetConditionalStripping(true, "#ifWinter", "#ifNotWinter", "#endif",
 *"#else");
 * res = stripper.SetIncluding("#Include", "D:\\projekti\\lehtiTuote\\include", "strip");
 * res = stripper.AddReplaceString("lineEnd", "thisLineEnds");
 * res = stripper.ReadAndStripFile("D:\\projekti\\projApp\\Press\\demo.strip");
 * stripper.StripBlocks ("AA", "ABC");
 * string RESULT = stripper.GetString();
 * \endcode
 *
 * In the sample files below we use "/_*" and "*_/" instead of the normal delimiters
 * to avoid preprocessor warnings on nested comments in the actual header files.
 *
 * Sample file \c demo.strip
 * =========================
 * \code
 * start
 * /_*
 *    away
 *  *_/ take //take not
 *  line
 *  //away
 *     #notFirst
 *  2222#if/_* now *_/WinterWINTER#elseSUMMER#endif lineEnd
 *  333#ifNotWinterSUMMER#elseWINTER#endif
 * 44#ifNotWinterNOTWINTER#endif
 *   #ifWinterWINTER#elseSUMMER#endif ABCDE
 *  AA stripped ABCD
 *  EE #Include demoInclude1.strip FF
 * \endcode
 *
 * Sample file \c demoInclude1.strip
 * =================================
 * \code
 * INSIDE INCLUDE1 begin
 * #ifWinter Inc1WINTER #else Inc1SUMMER #endif
 * #ifNotWinter Inc1SUMMER #else Inc1WINTER #endif
 * EE/_* skip /_*double skip *_/ *_/DD
 * #Include demoInclude2
 * //Comment
 * INSIDE INCLUDE1 end
 * \endcode
 *
 * Sample file \c demoInclude2.strip
 * =================================
 * \code
 * INSIDE INCLUDE2 begin
 * 22 /_* // *_/take
 * #ifWinter Inc2WINTER #else Inc2SUMMER #endif
 * INSIDE INCLUDE2 end
 * \endcode
 *
 * Resulting stripped file is:
 * ===========================
 * \code
 * start
 * take
 * line
 *
 *     #notFirst
 * 2222WINTER thisLineEnds
 * 333WINTER
 * 44
 *  WINTER ABCDE
 * D
 * EE INSIDE INCLUDE1 begin
 *  Inc1WINTER
 *  Inc1WINTER
 * EEDD
 *   INSIDE INCLUDE2 begin
 * 22 take
 *  Inc2WINTER
 * INSIDE INCLUDE2 end
 *
 * INSIDE INCLUDE1 end
 * FF
 * \endcode
 */
// ======================================================================

#include "NFmiPreProcessor.h"
#include "NFmiStringTools.h"
#include <macgyver/Exception.h>

#include <fstream>
#include <iostream>

using namespace std;

namespace
{
const int kMaxNumOfStripperIncludes = 200;
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

NFmiPreProcessor::NFmiPreProcessor(bool stripPound,
                                   bool stripDoubleSlash,
                                   bool stripSlashAst,
                                   bool stripSlashAstNested,
                                   bool stripEndOfLineSpaces)
    : NFmiCommentStripper(
          stripPound, stripDoubleSlash, stripSlashAst, stripSlashAstNested, stripEndOfLineSpaces),
      itsDefineDirective(""),
      itsIncludeDirective(""),
      itsIncludePath(),
      itsIncludeExtension(),
      itsConditionalBeginDirective(""),
      itsConditionalNotBeginDirective(),
      itsConditionalEndDirective(),
      itsConditionalElseDirective(),
      itsReplaceMap(),
      itsNumOfReplacesDone(0),
      fConditionValue(),
      fNoFilesIncluded(false),
      fUseReplace(false),
      itsCurNumOfIncludes(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theFileName Undocumented
 * \param stripPound Undocumented
 * \param stripDoubleSlash Undocumented
 * \param stripSlashAst Undocumented
 * \param stripSlashAstNested Undocumented
 * \todo This should be in the cpp file
 */
// ----------------------------------------------------------------------

NFmiPreProcessor::NFmiPreProcessor(const std::string &theFileName,
                                   bool stripPound,
                                   bool stripDoubleSlash,
                                   bool stripSlashAst,
                                   bool stripSlashAstNested,
                                   bool stripEndOfLineSpaces)
    : NFmiCommentStripper(theFileName,
                          stripPound,
                          stripDoubleSlash,
                          stripSlashAst,
                          stripSlashAstNested,
                          stripEndOfLineSpaces),
      itsDefineDirective(""),
      itsIncludeDirective(""),
      itsIncludePath(),
      itsIncludeExtension(),
      itsConditionalBeginDirective(""),
      itsConditionalNotBeginDirective(),
      itsConditionalEndDirective(),
      itsConditionalElseDirective(),
      itsReplaceMap(),
      itsNumOfReplacesDone(0),
      fConditionValue(),
      fNoFilesIncluded(false),
      fUseReplace(false),
      itsCurNumOfIncludes(0)
{
  try
  {
    ReadAndStripFile(theFileName);
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
 * \param theStripper The other object being copied
 */
// ----------------------------------------------------------------------

NFmiPreProcessor::NFmiPreProcessor(const NFmiPreProcessor &theStripper)
    : NFmiCommentStripper(theStripper),
      itsDefineDirective(theStripper.itsDefineDirective),
      itsIncludeDirective(theStripper.itsIncludeDirective),
      itsIncludePath(theStripper.itsIncludePath),
      itsIncludeExtension(theStripper.itsIncludeExtension),
      itsConditionalBeginDirective(theStripper.itsConditionalBeginDirective),
      itsConditionalNotBeginDirective(theStripper.itsConditionalNotBeginDirective),
      itsConditionalEndDirective(theStripper.itsConditionalEndDirective),
      itsConditionalElseDirective(theStripper.itsConditionalElseDirective),
      itsReplaceMap(theStripper.itsReplaceMap),
      itsNumOfReplacesDone(theStripper.itsNumOfReplacesDone),
      fConditionValue(theStripper.fConditionValue),
      fNoFilesIncluded(false),
      fUseReplace(theStripper.fUseReplace),
      itsCurNumOfIncludes(theStripper.itsCurNumOfIncludes)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::Strip()
{
  try
  {
    if (!NFmiCommentStripper::Strip())
      return false;

    if (IsConditional())
    {
      if (!StripConditionally())
        return false;
    }
    // Note: In replace mode ReplaceAll() is expected to be called by the calling module
    // after the main file (and thus all include files) is loaded
    //
    if ((!fUseReplace) /* Yes, naming not too clear here. */ && IsReplace())
    {
      if (!Replace())
        return false;
    }
    if (IsInclude())
    {
      if (!Include())
        return false;
    }

    if (IsDefine())
    {
      if (!Define())
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
 * \param theIncludeDirective Undocumented
 * \param theIncludePath Undocumented
 * \param theIncludeExtension Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::IncludeFiles(const string &theIncludeDirective,
                                    const string &theIncludePath,
                                    const string &theIncludeExtension)
{
  try
  {
    SetIncluding(theIncludeDirective, theIncludePath, theIncludeExtension);
    return Include();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tämä poistaa halutun merkin stringin alusta ja lopusta, jos merkki on molemmissa päissä
static void RemovePossibleCharactersFromStartAndEnd(std::string &theString, char theChar)
{
  try
  {
    if (theString.size() < 2)
      return;

    if (theString[0] == theChar && theString[theString.size() - 1] == theChar)
      theString = string(theString.begin() + 1, theString.end() - 1);
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

bool NFmiPreProcessor::Include()
{
  try
  {
    if (itsCurNumOfIncludes >= kMaxNumOfStripperIncludes)
    {
      itsMessage = "ERROR in " + itsFileName + ": max number of Include directives exceeded";
      return false;
    }
    itsCurNumOfIncludes++;
    string newString = "";
    string oldString(itsString);
    string fileContents, fileName;
    std::string::size_type lenDef = itsIncludeDirective.length();
    std::string::size_type posHelp;
    std::string::size_type pos = oldString.find(itsIncludeDirective);
    if (pos == string::npos)
    {
      fNoFilesIncluded = true;
      return true;  // HUOM
    }

    while (pos != string::npos)
    {
      newString += oldString.substr(0, pos);
      posHelp = oldString.find_first_of("\n", pos + lenDef + 1);
      fileName = oldString.substr(pos + lenDef + 1, posHelp - pos - lenDef - 1);
      NFmiStringTools::TrimR(fileName);  // tämä viritys johtuu metkun editorin smarttool
                                         // dialogista, poistetaan mahd. white spacet nimen lopusta
      RemovePossibleCharactersFromStartAndEnd(fileName, '"');  // tämä poistaa nimestä ""-merkit jos
                                                               // include:ssa on laitettu tiedoston
                                                               // nimi heittomerkkeihin
      CompleteFileName(fileName);
      if (IncludeFile(fileName, fileContents))
      {
        newString += fileContents;
        fNoFilesIncluded = false;
      }
      else
      {
        return false;
      }
      if (posHelp == string::npos)
      {
        itsString = newString;
        return true;
        // ollaan tultu jo loppuun, lopetetaan, muuten jää ikilooppiin, jos include-lauseen jälkeen
        // ei ole newlineä
      }
      oldString = oldString.substr(posHelp + 1);
      pos = oldString.find(itsIncludeDirective);
    }
    newString += oldString;
    itsString = newString;
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

bool NFmiPreProcessor::CompleteFileName(string &theFileName)
{
  try
  {
    if (theFileName.find(".") == string::npos)
    {
      if (!itsIncludeExtension.empty())
      {
        theFileName += ".";
        theFileName += itsIncludeExtension;
      }
    }
    if (theFileName.find(":") == string::npos && !itsIncludePath.empty())
    {
      string prefix = itsIncludePath;
      prefix += kFmiDirectorySeparator;
      theFileName = prefix + theFileName;
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
 * \param theFileContentsToInclude Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::IncludeFile(const string &theFileName, string &theFileContentsToInclude)
{
  try
  {
    NFmiPreProcessor stripper(fStripPound, fStripDoubleSlash, fStripSlashAst, fStripNested);
    stripper.SetNumOfIncludes(itsCurNumOfIncludes);
    if (IsConditional())
      stripper.SetConditionalStripping(fConditionValue,
                                       itsConditionalBeginDirective,
                                       itsConditionalNotBeginDirective,
                                       itsConditionalEndDirective,
                                       itsConditionalElseDirective);
    if (IsInclude())
      stripper.SetIncluding(itsIncludeDirective, itsIncludePath, itsIncludeExtension);

    if (fUseReplace)
      stripper.SetDefine(itsDefineDirective, true);

    if (!stripper.ReadAndStripFile(theFileName))
    {
      itsMessage = stripper.GetMessage();
      return false;
    }
    theFileContentsToInclude = stripper.GetString();

    if (fUseReplace)
      // Add included definitions to replace map, overwriting existing values
      //
      for (auto &iterMap : stripper.itsReplaceMap)
        AddReplaceString(iterMap.first, iterMap.second);

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
 * \param theEndDirective2 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::StripDoubleEnds(const string &theBeginDirective,
                                       const string &theEndDirective,
                                       const string &theEndDirective2)
{
  try
  {
    string oldString(itsString);
    string newString = "";
    string::size_type posbigalku = oldString.find(theBeginDirective);
    string::size_type posbigloppu, posbigloppu2;
    string::size_type len = theEndDirective.length();
    string::size_type len2 = theEndDirective2.length();
    string::size_type curLen;

    if (posbigalku == string::npos)
      itsString = oldString;
    else
    {
      while (posbigalku != string::npos)
      {
        posbigalku = oldString.find(theBeginDirective);

        if (posbigalku == string::npos)
          posbigloppu = string::npos;
        else
          posbigloppu = oldString.find(theEndDirective, posbigalku);

        if (posbigloppu == string::npos && posbigalku != string::npos)
        {
          itsMessage = "ERROR in " + itsFileName + ": " + theEndDirective + " missing";
          return false;
        }
        else if (posbigalku == string::npos && posbigloppu != string::npos)
        {
          itsMessage = "ERROR in " + itsFileName + ": " + theBeginDirective + " missing";
          return false;
        }

        if (posbigalku != string::npos)
          posbigloppu2 = oldString.find(theEndDirective2, posbigalku);
        else
          posbigloppu2 = string::npos;

        if (posbigalku == string::npos)
        {
          newString += oldString;
        }
        else
        {
          if (posbigloppu < posbigloppu2 || posbigloppu2 == string::npos)
          {
            curLen = len;
          }
          else
          {
            posbigloppu = posbigloppu2;
            curLen = len2;
          }
          newString += oldString.substr(0, posbigalku);
          oldString = oldString.substr(posbigloppu + curLen);
        }
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
 * \param theCondValue Undocumented
 * \param theConditionalBeginDirective Undocumented
 * \param theConditionalNotBeginDirective Undocumented
 * \param theConditionalEndDirective Undocumented
 * \param theConditionalElseDirective Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::StripConditionally(bool theCondValue,
                                          const string &theConditionalBeginDirective,
                                          const string &theConditionalNotBeginDirective,
                                          const string &theConditionalEndDirective,
                                          const string &theConditionalElseDirective)
{
  try
  {
    SetConditionalStripping(theCondValue,
                            theConditionalBeginDirective,
                            theConditionalNotBeginDirective,
                            theConditionalEndDirective,
                            theConditionalElseDirective);
    return StripConditionally();
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

bool NFmiPreProcessor::StripConditionally()
{
  try
  {
    if (!fConditionValue)
    {
      if (!StripDoubleEnds(itsConditionalBeginDirective,
                           itsConditionalEndDirective,
                           itsConditionalElseDirective))
        return false;

      if (!itsConditionalNotBeginDirective.empty())
      {
        if (!StripSubStrings(itsConditionalNotBeginDirective))
          return false;
      }
    }
    else
    {
      if (!itsConditionalNotBeginDirective.empty())
      {
        if (!StripDoubleEnds(itsConditionalNotBeginDirective,
                             itsConditionalEndDirective,
                             itsConditionalElseDirective))
          return false;
      }
      if (!StripSubStrings(itsConditionalBeginDirective))
        return false;
    }
    if (!StripBlocks(itsConditionalElseDirective, itsConditionalEndDirective))
      return false;

    if (!StripSubStrings(itsConditionalEndDirective))
      return false;

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
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::Replace()
{
  try
  {
    //  itsReplaceMap.insert(pair<string, string>("5vrk", "9vrk"));
    //  itsReplaceMap.insert(pair<string, string>("Summa", "Hop"));
    string newString(itsString);

    map<string, string>::iterator iterMap;
    for (iterMap = itsReplaceMap.begin(); iterMap != itsReplaceMap.end(); iterMap++)
    {
      string::size_type iterString = newString.find(iterMap->first);
      while (iterString != string::npos)
      {
        newString.replace(iterString, iterMap->first.length(), iterMap->second);
        itsNumOfReplacesDone++;
        iterString = newString.find(iterMap->first);
      }
    }
    itsString = newString;
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
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::ReplaceAll()
{
  try
  {
    return Replace();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIncludeDirective Undocumented
 * \param theIncludePath Undocumented
 * \param theIncludeExtension Undocumented
 * \return Undocumented
 * \todo This should be in the cpp file
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::SetIncluding(const std::string &theIncludeDirective,
                                    const std::string &theIncludePath,
                                    const std::string &theIncludeExtension)
{
  try
  {
    itsIncludeDirective = theIncludeDirective;
    itsIncludePath = theIncludePath;
    itsIncludeExtension = theIncludeExtension;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set a define directive on
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::SetDefine(const std::string &theDirective, bool useReplace)
{
  try
  {
    itsDefineDirective = theDirective;
    fUseReplace = useReplace;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theCondValue Undocumented
 * \param theConditionalBeginDirective Undocumented
 * \param theConditionalNotBeginDirective Undocumented
 * \param theConditionalEndDirective Undocumented
 * \param theConditionalElseDirective Undocumented
 * \todo This should be in the cpp file
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::SetConditionalStripping(bool theCondValue,
                                               const std::string &theConditionalBeginDirective,
                                               const std::string &theConditionalNotBeginDirective,
                                               const std::string &theConditionalEndDirective,
                                               const std::string &theConditionalElseDirective)
{
  try
  {
    fConditionValue = theCondValue;
    itsConditionalBeginDirective = theConditionalBeginDirective;
    itsConditionalNotBeginDirective = theConditionalNotBeginDirective;
    itsConditionalEndDirective = theConditionalEndDirective;
    itsConditionalElseDirective = theConditionalElseDirective;
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
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::SetReplaceMap(const std::map<std::string, std::string> &theMap)
{
  try
  {
    itsReplaceMap = theMap;
    itsNumOfReplacesDone = 0;
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
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::AddReplaceString(const std::string fromString, const std::string toString)
{
  try
  {
    auto ret = itsReplaceMap.insert(std::pair<std::string, std::string>(fromString, toString));

    // Overwrite existing values in replace mode

    if (fUseReplace && (!ret.second))
      ret.first->second = toString;

    itsNumOfReplacesDone = 0;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Expand all define directives
 *
 * \return True, if the operation succeeded
 */
// ----------------------------------------------------------------------

bool NFmiPreProcessor::Define()
{
  try
  {
    if (!IsDefine())
      return false;

    typedef map<string, string> Defines;
    Defines defines;
    bool fUseDefine = (!fUseReplace);

    ostringstream output;
    istringstream input(itsString);

    string line;
    while (getline(input, line))
    {
      istringstream lineinput(line);
      string token;
      lineinput >> token;
      if (token != itsDefineDirective)
      {
        // First token on line was not define directive, process the line
        // and then output it

        for (Defines::const_iterator it = defines.begin(); it != defines.end(); ++it)
        {
          int count = 0;
          string::size_type pos;
          while ((pos = line.find(it->first)) != string::npos)
          {
            line.replace(pos, it->first.size(), it->second);

            // Safety against eternal loop
            if (++count > 100)
              return false;
          }
        }

        output << line << endl;
      }
      else
      {
        // Extract the variable name and value
        string name, value;
        lineinput >> name;
        getline(lineinput, value);
        NFmiStringTools::TrimL(value);
        NFmiStringTools::TrimR(value);

        // Store the definition for local substitution (current file) or to the replace map.
        //
        // Local substitution ('define' mode) is used for upcoming definitions in replace mode too
        // if definition for the define directive having itself as the value is encountered. Local
        // substitution stays in effect for the rest of the current file (not to the files included)
        // or until a definition with any other value is made.

        if (fUseReplace && (name == itsDefineDirective))
          fUseDefine = (value == itsDefineDirective);
        else if (fUseDefine)
          defines[name] = value;
        else
          AddReplaceString(name, value);
      }
    }
    itsString = output.str();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
