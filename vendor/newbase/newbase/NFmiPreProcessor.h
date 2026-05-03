// ======================================================================
/*!
 * \file NFmiPreProcessor.h
 * \brief Interface of class NFmiPreProcessor
 */
// ======================================================================

#pragma once

#include "NFmiCommentStripper.h"
#include <map>
#include <string>

//! Undocumented
class NFmiPreProcessor : public NFmiCommentStripper
{
 public:
  NFmiPreProcessor(const NFmiPreProcessor& theStripper);

  NFmiPreProcessor(bool stripPound = false,
                   bool stripDoubleSlash = true,
                   bool stripSlashAst = true,
                   bool stripSlashAstNested = true,
                   bool stripEndOfLineSpaces = true);

  NFmiPreProcessor(const std::string& theFileName,
                   bool stripPound = false,
                   bool stripDoubleSlash = true,
                   bool stripSlashAst = true,
                   bool stripSlashAstNested = true,
                   bool stripEndOfLineSpaces = true);

  bool CompleteFileName(std::string& theFileName);

  using NFmiCommentStripper::Strip;
  bool Strip();

  bool StripDoubleEnds(const std::string& theBeginDirective,
                       const std::string& theEndDirective,
                       const std::string& theEndDirective2);

  bool StripConditionally(bool theCondValue,
                          const std::string& theConditionalBeginDirective,
                          const std::string& theConditionalNotBeginDirective = "",
                          const std::string& theConditionalEndDirective = "#endif",
                          const std::string& theConditionalElseDirective = "#else");

  bool IncludeFiles(const std::string& theIncludeDirective = "#Include",
                    const std::string& theIncludePath = "",
                    const std::string& theIncludeExtension = "inc");

  bool SetIncluding(const std::string& theIncludeDirective = "#Include",
                    const std::string& theIncludePath = "",
                    const std::string& theIncludeExtension = "inc");

  bool SetDefine(const std::string& theDefineDirective = "#define", bool useReplace = false);

  bool SetConditionalStripping(bool theCondValue,
                               const std::string& theConditionalBeginDirective,
                               const std::string& theConditionalNotBeginDirective = "",
                               const std::string& theConditionalEndDirective = "#endif",
                               const std::string& theConditionalElseDirective = "#else");

  bool SetReplaceMap(const std::map<std::string, std::string>& theMap);
  bool AddReplaceString(const std::string fromString, const std::string toString);

  int NumOfReplacesDone() const;
  bool NoFilesIncluded();

  void SetNumOfIncludes(int newValue) { itsCurNumOfIncludes = newValue; }
  bool ReplaceAll();

 private:
  bool StripConditionally();
  bool Replace();
  bool Include();
  bool Define();
  bool StripNestedConditionally(std::vector<unsigned long> theBeginPositions,
                                std::vector<unsigned long> theEndPositions,
                                bool theCond);
  bool IncludeFile(const std::string& theFileName, std::string& theFileContentsToInclude);
  bool IsConditional() const;
  bool IsInclude() const;
  bool IsReplace() const;
  bool IsDefine() const;

  std::string itsDefineDirective;
  std::string itsIncludeDirective;
  std::string itsIncludePath;
  std::string itsIncludeExtension;
  std::string itsConditionalBeginDirective;
  std::string itsConditionalNotBeginDirective;
  std::string itsConditionalEndDirective;
  std::string itsConditionalElseDirective;
  std::map<std::string, std::string> itsReplaceMap;
  int itsNumOfReplacesDone;
  bool fConditionValue;
  bool fNoFilesIncluded;

  bool fUseReplace;

  int itsCurNumOfIncludes;  // estää mm. rekursiivisen includen käädon ohjelmassa

};  // class NFmiPreProcessor

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline int NFmiPreProcessor::NumOfReplacesDone() const
{
  return itsNumOfReplacesDone;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo This should be const
 */
// ----------------------------------------------------------------------

inline bool NFmiPreProcessor::NoFilesIncluded()
{
  return fNoFilesIncluded;
}
// ----------------------------------------------------------------------
/*!
 * \return True, if a #define directive has been defined
 */
// ----------------------------------------------------------------------

inline bool NFmiPreProcessor::IsDefine() const
{
  return !itsDefineDirective.empty();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiPreProcessor::IsConditional() const
{
  return !itsConditionalBeginDirective.empty();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiPreProcessor::IsReplace() const
{
  return !itsReplaceMap.empty();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiPreProcessor::IsInclude() const
{
  return !itsIncludeDirective.empty();
}

// ======================================================================
