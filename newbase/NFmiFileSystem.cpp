// ======================================================================
/*!
 * \file NFmiFileSystem.cpp
 * \brief Implementation of class NFmiFileSystem
 */
// ======================================================================

#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include <macgyver/Exception.h>
#include <macgyver/FileSystem.h>

#include <filesystem>

#include <cctype>  // for isalpha
#include <cstdio>
#include <ctime>    // for time()
#include <fstream>  // for time()
#include <sstream>
#include <stdexcept>
#include <vector>  // for time()

#ifndef _MSC_VER
#include <dirent.h>
#else
// Must use MoveFileExA -funktion on Windows and this is how you have to include and do required
// stuff
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // Specifies that the minimum required platform is Windows Vista.
#endif
#define _AFXDLL
#include <afx.h>
#ifdef min
#undef min  // have to remove min -macro defined by win32 system
#endif
#ifdef CreateDirectory
#undef CreateDirectory  // have to remove CreateDirectory -macro defined by win32 system
#endif
#ifdef CopyFile
#undef CopyFile  // have to remove CopyFile -macro defined by win32 system
#endif
#endif

#include <cassert>

#ifdef BOOST
// Finding files is implemented in Linux using boost filesystem & regex
#include <boost/algorithm/string/predicate.hpp>  //Lasse
#include <filesystem>
#include <boost/regex.hpp>
#endif

extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>

#ifndef UNIX
#include <direct.h>
#include <io.h>
#else
#include <sys/dir.h>  // opendir() etc
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#endif
}

// VC++ does not seem to define these macors:

#ifndef S_ISREG
#define S_ISREG(mode) (mode & S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) (mode & S_IFDIR)
#endif

using namespace std;

#ifndef UNIX
static bool IsWinDir(const struct _finddata_t &fileinfo)
{
  if ((fileinfo.attrib & 16) == 16)
    return true;
  return false;
}
#endif

// Tuli ongelmia Winkkarissa isojen tiedostojen (> 2-4 GB) kanssa, joille pitää käyttää __stat64
// -tructia ja _stat64 -funktiota.
// Linux ja gcc hanskaa jutut ilman 64-virityksiä, koska siellä on käytetty mm. tiedosto koon
// kertomiseen
// _off_t     st_size
// ja _off_t on tyyppiä long tai long long. 64-bit ympäristössä se on gcc:ssä 64-bit integer, kun
// VC++:lla long on 32-bit.
#if defined _MSC_VER && defined _WIN64
#define FMI_stat struct __stat64
#define FMI_stat_func _stat64
#else
#define FMI_stat struct stat
#define FMI_stat_func stat
#endif

// ----------------------------------------------------------------------
/*!
 * \brief Unix specific utility subroutines
 */
// ----------------------------------------------------------------------

#ifdef BOOST
namespace Unix
{
// ----------------------------------------------------------------------
/*!
 * \brief Extend plain regex to POSIX regex
 *
 * -# Replace * with .*
 * -# Replace ? with .
 */
// ----------------------------------------------------------------------

string extend_plain_regex(const string &thePattern)
{
  string ret;
  ret.reserve(thePattern.size());

  for (char i : thePattern)
  {
    if (i == '*')
      ret += ".*";
    else if (i == '?')
      ret += '.';
    else
      ret += i;
  }
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract path from MSDOS pattern of type path/pattern
 *
 * We assume the pattern does not contain the "/"-character.
 */
// ----------------------------------------------------------------------

string path_of_msdos_pattern(const string &theMsPattern)
{
  string::size_type pos = theMsPattern.rfind('/');
  if (pos == string::npos)
    return ".";
  else
    return theMsPattern.substr(0, pos);
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract regex from MSDOS pattern of type path/pattern
 *
 * We assume the pattern does not contain the "/"-character.
 */
// ----------------------------------------------------------------------

string regex_of_msdos_pattern(const string &theMsPattern)
{
  string::size_type pos = theMsPattern.rfind('/');
  if (pos == string::npos)
    return extend_plain_regex(theMsPattern);
  else
  {
    string mspattern = theMsPattern.substr(pos + 1);
    return extend_plain_regex(mspattern);
  }
}

}  // namespace Unix
#endif

#ifndef UNIX

// Tein winkkari puolelle FMI-versiot findfirst, findnext ja findclose -funktioista, koska
// haluan napata kaikki mahdolliset poikkeukset kiinni ja jatkaa kuten virhetilanteessa normaalisti.
// Nyt olen törmännyt monesti SmartMetin kanssa että ohjelma on vain kaatunut käydessään läpi jotain
// hakemistoa.
intptr_t FmiFindFirst(const char *filespec, struct _finddata_t *fileinfo)
{
  try
  {
    return ::_findfirst(filespec, fileinfo);
  }
  catch (...)
  {
    return -1;
  }
}

int FmiFindNext(intptr_t handle, struct _finddata_t *fileinfo)
{
  try
  {
    return ::_findnext(handle, fileinfo);
  }
  catch (...)
  {
    return -1;
  }
}

int FmiFindClose(intptr_t handle)
{
  try
  {
    return ::_findclose(handle);
  }
  catch (...)
  {
    return -1;
  }
}
#endif  // #ifdef UNIX

namespace NFmiFileSystem
{
// ----------------------------------------------------------------------
/*!
 * Given an exact filename, and a :-separated search path, returns the
 * completed filename based on whether the filename is found where it is, or
 * at the given path. If the file is not found at either place, the original
 * input is returned back. The user is expected to verify with FileReadable
 * or some other method that the returned file does exist.
 *
 * \param theFile The filename to search for. May also be a relative path.
 * \param theSearchPath The search paths
 * \return The found full filenane, or theFile if no file was found
 */
// ----------------------------------------------------------------------

const string FileComplete(const string &theFile, const string &theSearchPath)
{
  // Safety
  if (theFile.empty() || theSearchPath.empty())
    return theFile;

  // Return existing files as is
  if (FileExists(theFile) || DirectoryExists(theFile))
    return theFile;

  // Return absolute paths as they are
  if (theFile[0] == '/' || theFile[0] == '\\')
    return theFile;

  // Split the search path into individual paths

  using PathList = std::list<std::string>;
  PathList paths;
  string::size_type pos1 = 0;
  while (pos1 < theSearchPath.size())
  {
    string::size_type pos2 = theSearchPath.find(':', pos1);
    if (pos2 == std::string::npos)
      pos2 = theSearchPath.size();
    paths.push_back(theSearchPath.substr(pos1, pos2 - pos1));
    pos1 = pos2 + 1;
  }

  // Try each individual search path

  for (PathList::const_iterator it = paths.begin(); it != paths.end(); ++it)
  {
    string name = *it;
    const char lastchar = name[name.size() - 1];
    if (lastchar != '/' && lastchar != '\\')
      name += '/';
    name += theFile;

    // Return the completed name if the respective file exists
    if (FileExists(name) || DirectoryExists(name))
      return name;
  }

  // Otherwise return the original name back
  return theFile;
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists and is a regular file
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool FileExists(const string &theFile)
{
  FMI_stat st;

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  return (S_ISREG(st.st_mode) != 0);
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists, is a regular file and is readable.
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool FileReadable(const string &theFile)
{
  FMI_stat st;

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  if (!S_ISREG(st.st_mode))
    return false;

#ifdef UNIX
  int rmask;
  if (st.st_uid == getuid())
    rmask = S_IRUSR;
  else if (st.st_gid == getgid())
    rmask = S_IRGRP;
  else
    rmask = S_IROTH;
  return ((st.st_mode & rmask) != 0 ? true : false);
#else
  return true;
#endif
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists, is a regular file and is writable
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool FileWritable(const string &theFile)
{
  FMI_stat st;

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  if (!S_ISREG(st.st_mode))
    return false;

#ifdef UNIX
  int wmask;
  if (st.st_uid == getuid())
    wmask = S_IWUSR;
  else if (st.st_gid == getgid())
    wmask = S_IWGRP;
  else
    wmask = S_IWOTH;
  return ((st.st_mode & wmask) != 0 ? true : false);
#else
  return true;
#endif
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists, is a regular file and is executable
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool FileExecutable(const string &theFile)
{
  FMI_stat st;

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  if (!S_ISREG(st.st_mode))
    return false;

#ifdef UNIX
  int xmask;
  if (st.st_uid == getuid())
    xmask = S_IXUSR;
  else if (st.st_gid == getgid())
    xmask = S_IXGRP;
  else
    xmask = S_IXOTH;
  return ((st.st_mode & xmask) != 0 ? true : false);
#else
  return true;
#endif
}

// ----------------------------------------------------------------------
/*!
 * Return file size, or 0 if the size is unknown
 *
 * \param theFile The filename
 * \return The size of the file
 */
// ----------------------------------------------------------------------

size_t FileSize(const string &theFile)
{
  try
  {
    std::filesystem::path filePath(theFile);
    return std::filesystem::file_size(filePath);
  }
  catch (...)
  {
  }
  return 0;
}

// ----------------------------------------------------------------------
/*!
 * Return true if file is missing or is empty
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool FileEmpty(const string &theFile)
{
  FMI_stat st;
  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return true;
  return (st.st_size == 0);
}

// ----------------------------------------------------------------------
/*!
 * Return last file access time.
 * Return value is undefined if file does not exist.
 *
 * \param theFile The filename
 * \return The last access time in epoch seconds
 */
// ----------------------------------------------------------------------

time_t FileAccessTime(const string &theFile)
{
  FMI_stat st;
  FMI_stat_func(theFile.c_str(), &st);
  return st.st_atime;
}

// ----------------------------------------------------------------------
/*!
 * Return last file modification time.
 * Return value is undefined if file does not exist.
 *
 * \param theFile The filename
 * \return The last modification time in epoch seconds
 */
// ----------------------------------------------------------------------

time_t FileModificationTime(const string &theFile)
{
  FMI_stat st;
  FMI_stat_func(theFile.c_str(), &st);
  return st.st_mtime;
}

// ----------------------------------------------------------------------
/*!
 * Return last file change time.
 * Return value is undefined if file does not exist. This is always
 * newer than modification time, since a file ctime changes for example
 * when commands chmod, chown and link are used. Use mtime to get
 * the real modification for file contents.
 *
 * \param theFile The filename
 * \returns The last change time in epoch seconds
 */
// ----------------------------------------------------------------------

time_t FileChangedTime(const string &theFile)
{
  FMI_stat st;
  FMI_stat_func(theFile.c_str(), &st);
  return st.st_ctime;
}

// ----------------------------------------------------------------------
/*!
 * Return time passed since last file modification time.
 * Return value is -1 if the file does not exist.
 *
 * \param theFile The filename
 * \return The age of the file in seconds
 */
// ----------------------------------------------------------------------

long FileAge(const string &theFile)
{
  FMI_stat st;
  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return -1;
  time_t modtime = st.st_mtime;

  time_t nowtime = time(nullptr);

  return static_cast<long>(nowtime - modtime);
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists and is a directory
 *
 * \param theFile The file
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool DirectoryExists(const string &theFile)
{
  if (theFile.empty() == true)
    return false;
  FMI_stat st;

  char ch = theFile[theFile.size() - 1];
  if (ch == '/' || ch == '\\')
  {  // ainakin VC71 ei tunnista hakemistoa olevaksi jos polku päättyy '/' tai '\' merkkiin,
     // joten poistan sen lopusta ja kutsun rekursiivisesti funktiota uudestaan.
    string file2(theFile.begin(), theFile.begin() + theFile.size() - 1);
    return DirectoryExists(file2);
  }

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  if (!S_ISDIR(st.st_mode))
    return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists, is a directory and is readable.
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool DirectoryReadable(const string &theFile)
{
  FMI_stat st;

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  if (!S_ISDIR(st.st_mode))
    return false;

#ifdef UNIX
  int rmask;
  if (st.st_uid == getuid())
    rmask = S_IRUSR;
  else if (st.st_gid == getgid())
    rmask = S_IRGRP;
  else
    rmask = S_IROTH;
  return ((st.st_mode & rmask) != 0 ? true : false);
#else
  return true;
#endif
}

// ----------------------------------------------------------------------
/*!
 * Test if file exists, is a directory file and is writable
 *
 * \param theFile The filename
 * \return True if the conditions are met
 */
// ----------------------------------------------------------------------

bool DirectoryWritable(const string &theFile)
{
  FMI_stat st;

  if (FMI_stat_func(theFile.c_str(), &st) != 0)
    return false;

  if (!S_ISDIR(st.st_mode))
    return false;

#ifdef UNIX
  int wmask;
  if (st.st_uid == getuid())
    wmask = S_IWUSR;
  else if (st.st_gid == getgid())
    wmask = S_IWGRP;
  else
    wmask = S_IWOTH;
  return ((st.st_mode & wmask) != 0 ? true : false);
#else
  return true;
#endif
}

// ----------------------------------------------------------------------
// System independent MKDIR() method
// Unix moodi on rwxr-xr-x
// ----------------------------------------------------------------------

#ifdef UNIX
#define MKDIR(path) mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#else
#define MKDIR(path) ::_mkdir(path)
#endif

// ----------------------------------------------------------------------
/*!
 * Creates the given path if it does not exist beforehand.
 *
 * \param thePath The path to create
 * \return Always true, but should return false if the operation fails
 * \todo Make the operation return false on failure
 */
// ----------------------------------------------------------------------

bool CreateDirectory(const string &thePath)
{
  if (thePath.empty() || DirectoryExists(thePath))
    return true;  // jos polku oli jo ei tarvitse jatkaa

  string tmpPath(thePath);
  NFmiStringTools::ReplaceChars(tmpPath, '\\', '/');
  string::size_type pos = 0;
  while ((pos = tmpPath.find('/', pos)) != string::npos)
  {
    string path = tmpPath.substr(0, pos);
    // "." has been optimized here for speed
    if ((path != ".") && !DirectoryExists(path))
      MKDIR(path.c_str());
    pos += 2;
  }

  if (!DirectoryExists(tmpPath))
    MKDIR(tmpPath.c_str());

  return DirectoryExists(tmpPath);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return name of newest regular file in directory
 *
 * \param thePath The directory name
 * \return The newest file, or an empty string if there is no newest file
 */
// ----------------------------------------------------------------------

const string NewestFile(const string &thePath)
{
  if (!DirectoryExists(thePath))
    return "";

  list<string> files = DirectoryFiles(thePath);
  if (files.empty())
    return "";

  string newestfile;
  time_t newesttime = 0;

  for (list<string>::const_iterator f = files.begin(); f != files.end(); ++f)
  {
    string filename = thePath + '/' + *f;
    if (FileReadable(filename))
    {
      time_t modtime = FileModificationTime(filename);
      if (modtime > newesttime)
      {
        newesttime = modtime;
        newestfile = *f;
      }
    }
  }

  return newestfile;
}

std::time_t NewestFileTime(const std::string &thePath)
{
  if (!DirectoryExists(thePath))
    return 0;

  list<string> files = DirectoryFiles(thePath);
  return NewestFileTime(files, thePath);
}

// filelist:issa o nvain pelkät tiedosto nimet ilman polkua, siksi polku on annettava toisena
// parametrina
std::time_t NewestFileTime(const std::list<std::string> &theFileList, const std::string &thePath)
{
  if (theFileList.empty())
    return 0;

  time_t newesttime = 0;
  for (const auto &f : theFileList)
  {
    string filename = thePath + '/' + f;
    if (FileReadable(filename))
    {
      time_t modtime = FileModificationTime(filename);
      if (modtime > newesttime)
        newesttime = modtime;
    }
  }
  return newesttime;
}

std::string NewestFileName(const std::list<std::string> &theFileList, const std::string &thePath)
{
  if (theFileList.empty())
    return "";

  std::string newestFileName;
  time_t newesttime = 0;
  for (const auto &f : theFileList)
  {
    string filename = thePath + f;
    if (FileReadable(filename))
    {
      time_t modtime = FileModificationTime(filename);
      if (modtime > newesttime)
      {
        newesttime = modtime;
        newestFileName = filename;
      }
    }
  }
  return newestFileName;
}

std::time_t NewestPatternFileTime(const std::string &thePattern)
{
  std::list<std::string> files = PatternFiles(thePattern);
  return NewestFileTime(files, PathFromPattern(thePattern));
}

std::string NewestPatternFileName(const std::string &thePattern)
{
  std::list<std::string> files = PatternFiles(thePattern);
  return NewestFileName(files, PathFromPattern(thePattern));
}

// Joskus on tarvetta saada vain ensimmäinen patterniin sopiva tiedoston nimi
std::string FirstPatternFileName(const std::string &thePattern)
{
  std::string firstFileName;
  std::string path = PathFromPattern(thePattern);

#ifdef UNIX
// ei ole toteutettu
#else   // windows
  struct _finddata_t fileinfo;
  intptr_t handle;
  if ((handle = FmiFindFirst(const_cast<char *>(thePattern.c_str()), &fileinfo)) != -1)
  {
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
      firstFileName = fileinfo.name;
  }
  else  // jos find_first ei löytänyt mitään ja mentiin silti find_next:iin, käätui XP:ssä
        // rakennettu juttu NT4:ssa
    return firstFileName;

  if (firstFileName.empty())
  {
    while (!FmiFindNext(handle, &fileinfo))
    {
      if (!::IsWinDir(fileinfo))  // ei ole hakemisto
      {
        firstFileName = fileinfo.name;
      }
    }
  }
  FmiFindClose(handle);
#endif  // windows

  return path + firstFileName;
}

// ----------------------------------------------------------------------
/*!
 * Returns a list of the files in the given directory.
 * The list contains only readable regular files, for example
 * directories are not listed. If the directory does not
 * exist, an empty list is returned.
 *
 * \param thePath The name of the directory
 * \return List of files in the directory
 */
// ----------------------------------------------------------------------

const list<string> DirectoryFiles(const string &thePath)
{
  list<string> out;

  namespace fs = std::filesystem;

  fs::path p = thePath;

  if (!fs::exists(p))
    return out;

  if (!fs::is_directory(p))
    return out;

  for (fs::directory_iterator end, dir(thePath); dir != end; ++dir)
  {
    if (fs::is_regular_file(*dir))
    {
      if (dir->path().filename().c_str()[0] != '.')
        out.push_back(dir->path().filename().string());
    }
  }
  return out;
}

// ----------------------------------------------------------------------
/*!
 * Returns a list of the files fittin to given pattern.
 * e.g. myDir/PAL_skandinavia*.sqd return all the files
 * that are in myDir-directory and matches with pattern
 * PAL_skandinavia*.sqd
 * If no mathches, an empty list is returned.
 *
 * \param thePattern Pattern e.g. mydir/mydata*.txt
 * \return List of files that match the pattern.
 */
// ----------------------------------------------------------------------

const std::list<std::string> PatternFiles(const std::string &thePattern)
{
  list<string> out;

#ifdef BOOST

  namespace fs = std::filesystem;

  // Conversion to Boost type searching

  string path = Unix::path_of_msdos_pattern(thePattern);
  string regex = Unix::regex_of_msdos_pattern(thePattern);

  // Safety checks

  if (!fs::exists(path))
    return out;
  if (!fs::is_directory(path))
    return out;

  // Search matching files

  boost::regex reg(regex);

  fs::directory_iterator end_dir;
  for (fs::directory_iterator it(path); it != end_dir; ++it)
  {
    if (boost::regex_match(it->path().filename().string().c_str(), reg))
      if (!fs::is_directory(*it))
      {
        out.emplace_back(it->path().filename().string().c_str());
      }
  }

#else

  struct _finddata_t fileinfo;
  intptr_t handle;
  if ((handle = FmiFindFirst(const_cast<char *>(thePattern.c_str()), &fileinfo)) != -1)
  {
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
      out.push_back(string(fileinfo.name));
  }
  else  // jos find_first ei löytänyt mitään ja mentiin silti find_next:iin, käätui XP:ssä
        // rakennettu juttu NT4:ssa
    return out;
  while (!FmiFindNext(handle, &fileinfo))
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
      out.push_back(string(fileinfo.name));
  FmiFindClose(handle);

#endif

  return out;
}

std::string PathFromPattern(const std::string &thePattern)
{
  NFmiFileString fileString(thePattern);
  fileString.NormalizeDelimiter();  // varmistetaan että kenoviivat on oikein päin
  std::string pathStr;
  if (fileString.IsAbsolutePath())
    pathStr += fileString.Device();
  pathStr += fileString.Path();
  return pathStr;
}

std::string FileNameFromPath(const std::string &theTotalPathAndFileStr)
{
  NFmiFileString fileString(theTotalPathAndFileStr);
  fileString.NormalizeDelimiter();  // varmistetaan että kenoviivat on oikein päin
  std::string fileNameStr = fileString.FileName().CharPtr();
  return fileNameStr;
}

// ----------------------------------------------------------------------
/*!
 * Returns a list of the files fittin to given pattern.
 * e.g. myDir/PAL_skandinavia*.sqd return all the files
 * that are in myDir-directory and matches with pattern
 * PAL_skandinavia*.sqd
 * Files must be also newer (modification time) that given time.
 * Also this modification time is returned along filenames.
 * If no mathches, an empty list is returned.
 *
 * \param thePattern Pattern e.g. mydir/mydata*.txt
 * \param timeLimit files must be also newer than this time.
 * \return List of files that match the pattern.
 */
// ----------------------------------------------------------------------

#ifndef UNIX
// Not implemented for Unix, yet
const std::list<std::pair<std::string, std::time_t> > PatternFiles(const std::string &thePattern,
                                                                   std::time_t timeLimit)
{
  std::list<std::pair<std::string, std::time_t> > out;

  NFmiFileString fileString(thePattern);
  std::string pathStr;
  if (fileString.IsAbsolutePath())
    pathStr += fileString.Device();
  pathStr += fileString.Path();

  struct _finddata_t fileinfo;
  intptr_t handle;
  if ((handle = FmiFindFirst(const_cast<char *>(thePattern.c_str()), &fileinfo)) != -1)
  {
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
      if (timeLimit < fileinfo.time_write)
        out.push_back(make_pair(pathStr + fileinfo.name, fileinfo.time_write));
  }
  else  // jos find_first ei löytänyt mitään ja mentiin silti find_next:iin, käätui XP:ssä
        // rakennettu juttu NT4:ssa
    return out;
  while (!FmiFindNext(handle, &fileinfo))
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
      if (timeLimit < fileinfo.time_write)
        out.push_back(make_pair(pathStr + fileinfo.name, fileinfo.time_write));
  FmiFindClose(handle);

  return out;
}
#endif

// ----------------------------------------------------------------------
/*!
 * Returns a list of the files fittin to given pattern.
 * e.g. myDir/PAL_skandinavia*.sqd return all the files
 * that are in myDir-directory and matches with pattern
 * PAL_skandinavia*.sqd
 * If no mathches, an empty list is returned.
 *
 * \param thePattern Pattern e.g. mydir/mydata*.txt
 * \return List of files that match the pattern.
 */
// ----------------------------------------------------------------------

const std::list<std::string> Directories(const std::string &thePath)
{
  list<string> out;

#ifdef BOOST
  namespace fs = std::filesystem;

  // Safety checks

  if (!fs::exists(thePath))
    return out;
  if (!fs::is_directory(thePath))
    return out;

  // Search matching files

  fs::directory_iterator end_dir;
  for (fs::directory_iterator it(thePath); it != end_dir; ++it)
  {
    if (fs::is_directory(*it))
    {
      out.emplace_back(it->path().filename().string().c_str());
    }
  }

#else

  std::string usedPath(thePath);
  usedPath += "/*";
  //	NFmiStringTools::TrimR(usedPath, '\\');
  //	NFmiStringTools::TrimR(usedPath, '/');

  struct _finddata_t fileinfo;
  intptr_t handle;
  if ((handle = FmiFindFirst(const_cast<char *>(usedPath.c_str()), &fileinfo)) != -1)
  {
    if (::IsWinDir(fileinfo))  // jos on hakemisto
      out.push_back(string(fileinfo.name));
  }
  else  // jos find_first ei löytänyt mitään ja mentiin silti find_next:iin, käätui XP:ssä
        // rakennettu juttu NT4:ssa
    return out;
  while (!FmiFindNext(handle, &fileinfo))
    if (::IsWinDir(fileinfo))  // jos on hakemisto
      out.push_back(string(fileinfo.name));
  FmiFindClose(handle);

#endif

  return out;
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa filtterinä määrätyn tiedostojoukon uusimman/vanhimman
 * muokatun tiedoston nimen eli jos filtteri on
 * "c:\\weto\\wrk\\data\\in\\data*.*" ja kyseisestä hakemistosta löytyy
 * data-alkuinen tiedostoja, palautetaan uusimman/vanhimman tiedoston
 * nimi, mutta vain tiedoston nimi eli esim. "data_hirlam_2606.sqd" (ei
 * polkua!)  Huom! jos tiedostoa ei löydy, on palautetun time_t:n
 * (aikaleima) arvo 0.
 *
 * \param theFileFilter Undocumented
 * \param fSearchNewest Undocumented
 * \param theFoundFileName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

time_t FindFile(const string &theFileFilter,
                bool fSearchNewest,
                string *theFoundFileName /*RELATIVE, NO PATH!*/)
{
#ifdef BOOST
  namespace fs = std::filesystem;

  // Collect matches into a modification time sorted map

  typedef multimap<time_t, string> Matches;
  Matches matches;

  // Conversion to Boost type searching

  string path = Unix::path_of_msdos_pattern(theFileFilter);
  string regex = Unix::regex_of_msdos_pattern(theFileFilter);

  // Safety checks

  if (!fs::exists(path))
    return 0;
  if (!fs::is_directory(path))
    return 0;

  // Search matching files

  boost::regex reg(regex);

  fs::directory_iterator end_dir;
  for (fs::directory_iterator it(path); it != end_dir; ++it)
  {
    if (boost::regex_match(it->path().filename().string().c_str(), reg))
    {
      std::error_code ec;
      const std::time_t lwt = Fmi::last_write_time(*it, ec);
      if (!ec)
        matches.insert(std::make_pair(lwt, it->path().filename().string().c_str()));
    }
  }

  // Return the desired age file (newest/oldest)

  if (matches.size() == 0)
    return 0;
  else
  {
    Matches::const_iterator it = (fSearchNewest ? --matches.end() : matches.begin());
    if (theFoundFileName != nullptr)
      *theFoundFileName = it->second;
    return it->first;
  }
#else
  struct _finddata_t fileinfo;
  intptr_t handle;
  time_t currentTime = 0;

  if ((handle = FmiFindFirst(const_cast<char *>(theFileFilter.c_str()), &fileinfo)) != -1)
  {
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
    {
      *theFoundFileName = fileinfo.name;
      currentTime = fileinfo.time_write;  // time_write, time_create, time_access
    }
  }
  else
  {
    *theFoundFileName = "";
    return 0;
  }

  while (!FmiFindNext(handle, &fileinfo))
  {
    if (!::IsWinDir(fileinfo))  // ei ole hakemisto
    {
      if ((fSearchNewest && (currentTime < fileinfo.time_write)) ||
          (!fSearchNewest && (currentTime > fileinfo.time_write)))
      {
        currentTime = fileinfo.time_write;
        *theFoundFileName = fileinfo.name;
      }
    }
  }
  FmiFindClose(handle);
  return currentTime;
#endif
}

// ----------------------------------------------------------------------
/*!
 * Delete the given file.
 *
 * \param theFile The file to delete
 * \return True on successful delete
 *
 * \note
 * According to Unix stdio.h, should return true on success.
 * If the method is POSIX, so should Windows.
 * Also, the method works equally well for directories, atleast
 * on Unix.
 */
// ----------------------------------------------------------------------

bool RemoveFile(const string &theFile)
{
  return ::remove(theFile.c_str()) == 0;
}
// ----------------------------------------------------------------------
/*!
 * Rename the given file.
 *
 * \param theOldFileName The old name of the file
 * \param theNewFileName The new name of the file
 * \return True on successful rename
 *
 * \note
 * According to Unix stdio.h, should return true on success.
 * If the method is POSIX, so should Windows.
 */
// ----------------------------------------------------------------------

bool RenameFile(const string &theOldFileName, const string &theNewFileName)
{
#ifdef UNIX
  int err = ::rename(theOldFileName.c_str(), theNewFileName.c_str());
  if (err == 0)
    return true;

  // Try to copy across file systems
  if (errno == EXDEV)
  {
    if (!CopyFile(theOldFileName, theNewFileName))
      return false;
    RemoveFile(theOldFileName);
    return true;
  }
  // Unknown error, exit
  return false;
#else
  // MSVC rename ei suostu toimimaan jos on jo olemassa theNewFileName-tiedosto
  // Siksi että saadaan homma toimimaan kuten linuxissa, pitää deletoida mahd.
  // olemassa oleva tiedosto.
  if (NFmiFileSystem::FileExists(theNewFileName))
    NFmiFileSystem::RemoveFile(theNewFileName);
  return ::rename(theOldFileName.c_str(), theNewFileName.c_str()) == 0;
#endif  // UNIX
}

// ----------------------------------------------------------------------
/*!
 * Read the given file to a given string.
 *
 * \param theFileName Name of the file that is to be read to the string.
 * \param theFileContent The string that will be filled with file's content.
 * \param theMaxByteSize If file size is larger than this, file won't be read.
 * \return Returns true if successful, else false.
 */
// ----------------------------------------------------------------------

bool ReadFile2String(const std::string &theFileName,
                     std::string &theFileContent,
                     long theMaxByteSize)
{
  if (theFileName != "")
  {
    size_t fileSize = FileSize(theFileName);
    ifstream in(theFileName.c_str(), std::ios::binary);
    if (fileSize > 0 && static_cast<long>(fileSize) <= theMaxByteSize && in)
    {
      vector<char> buffer(fileSize);
      // Luetaan vectoriin mieluummin kuin suoraan stringiin, koska standardi ei
      // takaa että string-otus pitäisi merkkipuskuriaan yhtenäisenä muistissa.
      in.read(&buffer[0], fileSize);
      theFileContent.resize(buffer.size());
      memcpy(&theFileContent[0], &buffer[0], buffer.size());  // sijoitus pitää tehdä memcpy:llä,
      // muuten 0-merkit binääri tiedostossa
      // lopettaa kopioinnin
      in.close();
      return in.good();
    }
    else if (FileExists(theFileName) && fileSize == 0)
    {
      theFileContent = "";  // pitää tyhjentää stringi, koska tiedosto oli tyhjä
      return true;          // tiedoston luku onnistui, vaikka se olikin tyhjä
    }
  }
  return false;
}

bool ReadFileStart2String(const std::string &theFileName,
                          std::string &theFileContent,
                          long theBytesFromStart)
{
  if (theFileName != "")
  {
    size_t fileSize = FileSize(theFileName);
    ifstream in(theFileName.c_str(), std::ios::binary);
    if (fileSize > 0 && in)
    {
      size_t readBytes = std::min(static_cast<long>(fileSize), theBytesFromStart);
      vector<char> buffer(readBytes + 1);  // +1 tekee tilaa 0-merkille
      // Luetaan vectoriin mieluummin kuin suoraan stringiin, koska standardi ei
      // takaa että string-otus pitäisi merkkipuskuriaan yhtenäisenä muistissa.
      in.read(&buffer[0], readBytes);
      buffer[readBytes] = '\0';  // päätetään 'stringi' loppumerkillä, että kopiointi onnistuu
      theFileContent.resize(buffer.size());
      memcpy(&theFileContent[0], &buffer[0], buffer.size());  // sijoitus pitää tehdä memcpy:llä,
      // muuten 0-merkit binääri tiedostossa
      // lopettaa kopioinnin
      in.close();
      return in.good();
    }
    else if (FileExists(theFileName) && fileSize == 0)
    {
      theFileContent = "";  // pitää tyhjentää stringi, koska tiedosto oli tyhjä
      return true;          // tiedoston luku onnistui, vaikka se olikin tyhjä
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Copy the given file.
 *
 * \param theSrcFileName The source name of the file
 * \param theDstFileName The destination name of the file
 * \return True on successful copy
 *
 */
// ----------------------------------------------------------------------

bool CopyFile(const std::string &theSrcFileName, const std::string &theDstFileName)
{
  if (theSrcFileName != "" && theDstFileName != "")
  {
    ifstream in(theSrcFileName.c_str(), std::ios::binary | std::ios::in);
    ofstream out(theDstFileName.c_str(), std::ios::binary | std::ios::out);
    if (in && out)
    {
      out << in.rdbuf();
      return (!out.fail()) && (!in.fail());
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a temporary file name in the given directory
 *
 * The standard mktemp is unsafe, and the recommended substitutes
 * are not portable. Also, using mktemp causes severe warnings
 * with g++ which are annoying during link time. Hence a
 * substitute algorithm was created.
 *
 * Throws if the path does not exist.
 *
 * \param thePath The path to the desired directory
 * \return Full pathname to a non-existing file
 */
// ----------------------------------------------------------------------

const std::string TemporaryFile(const std::string &thePath)
{
  if (!DirectoryReadable(thePath))
    throw Fmi::Exception(BCP, "Directory '" + thePath + "' does not exist or is not readable");

  static unsigned long counter = 0;

  for (int attempts = 0; attempts < 10; ++attempts)
  {
    ostringstream name;

    name << thePath << kFmiDirectorySeparator << ".newbase_tmpfile_"
#ifdef UNIX
         << getpid()
#else
         << attempts
#endif
         << '_' << ++counter;

    if (!FileExists(name.str()))
      return name.str();
  }

  throw Fmi::Exception(BCP, "Failed to create a temporary filename in directory '" + thePath + "'");
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract directory name from given path
 *
 * This works like the Unix shell command \c dirname:
 *
 * \code
 * (empty)    ==> .
 * .          ==> .
 * /          ==> /
 * /a         ==> /
 * /a/        ==> /
 * /a/b       ==> /a
 * /a/b/      ==> /a
 * /a/b/c     ==> /a/b
 * a          ==> .
 * a/         ==> .
 * a/b        ==> a
 * a/b/       ==> a
 * a/b/c      ==> a/b
 * \endcode
 *
 * \param theName The name
 * \return The directory part
 */
// ----------------------------------------------------------------------

const std::string DirName(const std::string &theName)
{
  if (theName.empty())
    return ".";

  if (theName == "/")
    return theName;

  string name;
  if (theName[theName.size() - 1] == '/')
    name = theName.substr(0, theName.size() - 1);
  else
    name = theName;

  const string::size_type pos = name.rfind('/');

  if (pos == string::npos)
    return ".";

  if (pos == 0)
    return "/";

  return name.substr(0, pos);
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract base name from given path
 *
 * \code
 * This works like the Unix shell command \c dirname:
 *
 * \code
 * (empty)    ==> (empty)
 * .          ==> .
 * /          ==> /
 * /a         ==> a
 * /a/        ==> a
 * /a/b       ==> b
 * /a/b/      ==> b
 * /a/b/c     ==> c
 * a          ==> a
 * a/         ==> a
 * a/b        ==> b
 * a/b/       ==> b
 * a/b/c      ==> c
 * \endcode
 *
 * \param theName The name
 * \return The filename part
 */
// ----------------------------------------------------------------------

const std::string BaseName(const std::string &theName)
{
  if (theName.empty())
    return theName;

  if (theName.size() == 1)
    return theName;

  string name;
  if (theName[theName.size() - 1] == '/')
    name = theName.substr(0, theName.size() - 1);
  else
    name = theName;

  const string::size_type pos = name.rfind('/');

  if (pos == string::npos)
    return name;

  return name.substr(pos + 1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Cleans files from given directory, unless they are
 * newer then given hour-value. Is not recursive.
 *
 * Throws if the directory does not exist.
 *
 * \param thePath The directory that is to be cleaned.
 * \param theMaxFileAgeInDays How old can files be max so that they are not cleaned.
 *       If value is negative, nothing is done.
 *
 */
// ----------------------------------------------------------------------
void CleanDirectory(const std::string &thePath,
                    double theMaxFileAgeInHours,
                    std::list<std::string> *theDeletedFilesOut)
{
  if (theMaxFileAgeInHours < 0)
    return;
  if (!DirectoryExists(thePath))
    throw Fmi::Exception(BCP,
                         "NFmiFileSystem::CleanDirectory - directory doesn't exist: " + thePath);

  list<string> files = DirectoryFiles(thePath);
  if (files.empty())
    return;

  time_t currentTime;
  static_cast<void>(::time(&currentTime));

  for (list<string>::const_iterator f = files.begin(); f != files.end(); ++f)
  {
    string filename = thePath + '/' + *f;
    time_t modtime = FileModificationTime(filename);
    if (currentTime - modtime > theMaxFileAgeInHours * 3600)
    {
      bool status = RemoveFile(filename);
      if (theDeletedFilesOut && status)
        theDeletedFilesOut->push_back(filename);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Cleans files from given file pattern (e.g. mydirectory/myfiles_*.txt),
 * but leaves as many files as wanted.
 *
 * Throws if the directory does not exist.
 *
 * \param theFilePattern The file pattern that is to be cleaned.
 * \param theKeepFileCount How many newest files are left.
 *       If value is negative, nothing is done.
 *
 */
// ----------------------------------------------------------------------
void CleanFilePattern(const std::string &theFilePattern,
                      int theKeepFileCount,
                      std::list<std::string> *theDeletedFilesOut)
{
  if (theKeepFileCount < 0)
    return;
  NFmiFileString fileString(theFilePattern);
  std::string path = fileString.Device().CharPtr();
  path += fileString.Path().CharPtr();
  if (!DirectoryExists(path))
    throw Fmi::Exception(BCP,
                         "NFmiFileSystem::CleanFilePattern - directory doesn't exist: " + path);
  list<string> files = PatternFiles(theFilePattern);
  std::map<time_t, std::string> fileMap;
  for (list<string>::const_iterator f = files.begin(); f != files.end(); ++f)
  {
    string filename = path + '/' + *f;
    time_t modtime = FileModificationTime(filename);
    fileMap.insert(std::make_pair(modtime, filename));
  }

  auto mapIt = fileMap.begin();
  for (int i = 0; mapIt != fileMap.end(); ++mapIt, i++)
  {
    if (static_cast<int>(fileMap.size()) - i <= theKeepFileCount)
      break;
    else
    {
      std::string &filename = (*mapIt).second;
      bool status = RemoveFile(filename);
      if (theDeletedFilesOut && status)
        theDeletedFilesOut->push_back(filename);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find latest querydata from directory
 *
 * a) a file is OK as is
 * b) directory is scanned for newest file
 *    1. whose name does not start with "."
 *    2. whose suffix is fqd or sqd with optional .gz, .bz2, .xz or .zstd suffix  *
 */
// ----------------------------------------------------------------------

string FindQueryData(const string &thePath)
{
  if (!DirectoryExists(thePath))
  {
    if (!FileReadable(thePath))
      throw Fmi::Exception(BCP, "File '" + thePath + "' is not readable");
    return thePath;
  }

  using Files = list<string>;
  Files files = DirectoryFiles(thePath);

  string newestfile;
  time_t newesttime = 0;

  for (Files::const_iterator f = files.begin(); f != files.end(); ++f)
  {
    const string &name = *f;

    if (!name.empty() && name[0] == '.')
      continue;

    if (IsQueryData(name))
    {
      string fullpath = thePath + '/' + name;

      if (FileSize(fullpath) != 0)
      {
        time_t modtime = FileModificationTime(fullpath);
        if (modtime > newesttime)
        {
          newesttime = modtime;
          newestfile = fullpath;
        }
      }
    }
  }

  if (newesttime == 0)
    throw Fmi::Exception(BCP, "No querydata found from '" + thePath + "'");

  return newestfile;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if filename has query data (or compressed query data) suffix
 */
// ----------------------------------------------------------------------

bool IsQueryData(const string &theName)
{
#ifdef BOOST
  static boost::regex r_qd("\\.(s|f)qd$");
  if (Fmi::is_compressed(theName))
  {
    std::filesystem::path p(theName);
    p.replace_extension();
    return boost::regex_search(p.string(), r_qd);
  }
  else
  {
    return boost::regex_search(theName, r_qd);
  }
#else
  // FIXME: it would perhaps best to avoid using boost::regex here and
  //        write own test fir suffixes
  return true;
#endif
}

// Tarkoitus on tehdä annetusta theOrigPath-parametrista absoluuttinen polku.
// Jos se oli jo sitä, palautetaan se.
// Jos se oli suhteellinen palku, liitetaan se annettuun theWorkingDirectory-parametriin,
// joka oletetaan olevan absoluuttinen polku.
// theOrigPath -parametri voi olla polku tai polku tiedostoon.
// Esim1. theOrigPath = C:\xxx    theWorkingDirectory = D:\work       --> C:\xxx
// Esim2. theOrigPath = xxx       theWorkingDirectory = D:\work       --> D:\work\xxx
std::string MakeAbsolutePath(const std::string &theOrigPath, const std::string &theWorkingDirectory)
{
  NFmiFileString fileString(theOrigPath);
  if (fileString.IsAbsolutePath())
    return theOrigPath;
  else
  {
    std::string finalPath;
    finalPath = theWorkingDirectory;
    if (theWorkingDirectory.size() &&
        theWorkingDirectory[theWorkingDirectory.size() - 1] != kFmiDirectorySeparator)
#ifdef WIN32
      if (theWorkingDirectory[theWorkingDirectory.size() - 1] != '/')
#endif  // WIN32
        finalPath += kFmiDirectorySeparator;
    finalPath += theOrigPath;
    return finalPath;
  }
}

// Polku on absoluuttinen jos:
// 1. Sen 1. kirjain on '\\'
// 2. Sen 1. kirjain on '/'
// 3. Siinä on kirjain, ':' ja joko '\\' tai '/' eli windows device esim. C:/tmp
bool IsAbsolutePath(const std::string &thePath)
{
  if (thePath.size())
  {
    if (thePath[0] == '/' || thePath[0] == '\\')
      return true;

    if (thePath.size() >= 3)
    {
      if (std::isalpha(thePath[0]) && thePath[1] == ':' &&
          (thePath[2] == '\\' || thePath[2] == '/'))
        return true;
    }
  }
  return false;
}

// Tämä on olevinaan turvallinen tiedostoon talletus:
// 1. Luodaan tmp tiedosto ja kirjoitetaan sisältö siihen.
// 2. Rename:taan tmp-tiedosto lopulliseen muotoon.
void SafeFileSave(const std::string &theFileName, const std::string &theContents)
{
  const std::string tmpHelpStr = "_TMP";
  std::string tmpFileName = theFileName + tmpHelpStr;
  std::ofstream out(tmpFileName.c_str(), std::ios::binary);
  if (!out)
    throw Fmi::Exception(BCP,
                         "Failed to open temporary file '" + tmpFileName +
                             "' for writing. No changes to original file '" + theFileName +
                             "' was made.");
  out << theContents;
  if (!out)
  {
    out.close();  // pitää sulkea, muuten ei voi poistaa
    RemoveFile(tmpFileName);
    throw Fmi::Exception(BCP,
                         "Error while writing to temporary file: '" + tmpFileName +
                             "'. No changes to original file '" + theFileName + "' was made.");
  }
  out.close();

#ifdef _MSC_VER
  // NFmiFileSystem::RenameFile doesn't work allways desired way on Windows, in some cases
  // people have noticed that only the TMP-file was left after execution. That's is why I try
  // to solve this with Windows own win32 functionality.
  // Using the ascii version of MoveFileEx -function (A -letter as last character), because we
  // are operating with ascii file paths.
  if (!::MoveFileExA(tmpFileName.c_str(), theFileName.c_str(), MOVEFILE_REPLACE_EXISTING))
  {
    throw Fmi::Exception(BCP,
                         "Error while moving temporary file: '" + tmpFileName +
                             "' to replace the original file '" + theFileName + "' was made.");
  }
#else
  // RenameFile -funktio tuhoaa vanhan tiedoston uuden tieltä. En tiedä mitä pitäisi tehdä,
  // jos tämä epäonnistuu (vanha tiedosto on ehkä tuhoutunut ja uusi on ehkä tmp-tiedostona).
  if (!RenameFile(tmpFileName, theFileName))
    throw Fmi::Exception(BCP,
                         "Error while renaming temporary file: '" + tmpFileName +
                             "' to final file: '" + theFileName + "'. Results unknown.");
#endif
}

}  // namespace NFmiFileSystem
