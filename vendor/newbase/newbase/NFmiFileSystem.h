// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiFileSystem
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"

#include <ctime>   // time_t
#include <list>    // for ListDirectory
#include <string>  // filenames are strings

// Koska Winkkari headerissa
// on makro joka muuttaa funktion nimeä:
//
// #ifdef UNICODE
// #define CreateDirectory  CreateDirectoryW
// #else
// #define CreateDirectory  CreateDirectoryA
// #endif // !UNICODE

#if defined CreateDirectory
#undef CreateDirectory
#endif

// sama juttu CopyFile*:n kanssa
#if defined CopyFile
#undef CopyFile
#endif

namespace NFmiFileSystem
{
// File access checking:

bool FileExists(const std::string &theFile);
bool FileReadable(const std::string &theFile);
bool FileWritable(const std::string &theFile);
bool FileExecutable(const std::string &theFile);
size_t FileSize(const std::string &theFile);
bool FileEmpty(const std::string &theFile);

// Filename completion
const std::string FileComplete(const std::string &theFile, const std::string &theSearchPath);

std::time_t FileAccessTime(const std::string &theFile);
std::time_t FileModificationTime(const std::string &theFile);
std::time_t FileChangedTime(const std::string &theFile);
std::time_t FindFile(const std::string &theFileFilter,
                     bool fSearchNewest,
                     std::string *theFoundFileName);

long FileAge(const std::string &theFile);

bool DirectoryExists(const std::string &theFile);
bool DirectoryReadable(const std::string &theFile);
bool DirectoryWritable(const std::string &theFile);

bool CreateDirectory(const std::string &thePath);
const std::string NewestFile(const std::string &thePath);
std::time_t NewestFileTime(const std::string &thePath);
std::time_t NewestFileTime(const std::list<std::string> &theFileList, const std::string &thePath);
std::string NewestFileName(const std::list<std::string> &theFileList, const std::string &thePath);
std::time_t NewestPatternFileTime(const std::string &thePattern);
std::string NewestPatternFileName(const std::string &thePattern);
std::string FirstPatternFileName(const std::string &thePattern);
std::string PathFromPattern(const std::string &thePattern);
std::string FileNameFromPath(const std::string &theTotalPathAndFileStr);
const std::list<std::string> DirectoryFiles(const std::string &thePath);
const std::list<std::string> PatternFiles(const std::string &thePattern);
const std::list<std::pair<std::string, std::time_t> > PatternFiles(const std::string &thePattern,
                                                                   std::time_t timeLimit);
const std::list<std::string> Directories(const std::string &thePath);

bool RemoveFile(const std::string &theFile);
bool RenameFile(const std::string &theOldFileName, const std::string &theNewFileName);
bool CopyFile(const std::string &theSrcFileName, const std::string &theDstFileName);

bool ReadFile2String(const std::string &theFileName,
                     std::string &theFileContent,
                     long theMaxByteSize = 1024 * 1024 * 500);  // 500 MB
bool ReadFileStart2String(const std::string &theFileName,
                          std::string &theFileContent,
                          long theBytesFromStart);

const std::string TemporaryFile(const std::string &thePath);
const std::string BaseName(const std::string &theName);
const std::string DirName(const std::string &theName);

void CleanDirectory(const std::string &thePath,
                    double theMaxFileAgeInHours,
                    std::list<std::string> *theDeletedFilesOut = 0);
void CleanFilePattern(const std::string &theFilePattern,
                      int theKeepFileCount,
                      std::list<std::string> *theDeletedFilesOut = 0);

std::string FindQueryData(const std::string &thePath);
bool IsQueryData(const std::string &theName);
std::string MakeAbsolutePath(const std::string &theOrigPath,
                             const std::string &theWorkingDirectory);
bool IsAbsolutePath(const std::string &thePath);

void SafeFileSave(const std::string &theFileName, const std::string &theContents);

}  // namespace NFmiFileSystem

// ======================================================================
