// ======================================================================
/*!
 * \file NFmiLocationFinder.cpp
 * \brief Implementation of class NFmiLocationFinder
 */
// ======================================================================
/*!
 * \class NFmiLocationFinder
 *
 * This class is intended for reading textual location databases
 * in a specific format. The data is used for example in GSM-applications
 * and www-applications to convert user specified location names
 * into geographic coordinates.
 *
 * Sample usage:
 *
 * \code
 * #include "NFmiLocationFinder.h"
 * ...
 * NFmiLocationFinder finder;
 * finder.AddFile("/var/www/share/querydata/Maailma.txt",false);
 * NFmiPoint latlon = finder.Find("Pariisi");
 * if(!finder.LastSearchFailed())
 * { ... }
 * \endcode
 *
 * \todo
 * Add the possibility to find nearest points using a NFmiNearTree.
 * This could be implemented using a pointer to a NFmiNearTree, and the
 * tree would be built when a point is searched for the first time.
 * One would have to decide how to handle multiple names for a single
 * point though.
 *
 */
// ======================================================================

#include "NFmiLocationFinder.h"
#include "NFmiStringTools.h"
#include <macgyver/Exception.h>
#include <cstdio>  // for sscanf()
#include <cstdlib>
#include <fstream>
#include <vector>
// atoi()

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLocationFinder::~NFmiLocationFinder() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLocationFinder::NFmiLocationFinder()
    : itsPrimaryPoints(), itsSecondaryPoints(), itsLastSearchFailed(false)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theLocationFinder The object to copy
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

NFmiLocationFinder::NFmiLocationFinder(const NFmiLocationFinder& theLocationFinder)

    = default;

// ======================================================================
/*!
 * This method reads the given file one line at a time, and extracts
 * location names and coordinates from it.
 *
 * The allowed data format consists of
 *
 *  -# Empty lines
 *  -# Comment lines beginning with // or #
 *  -# Data lines consisting of up to 6 location names
 *     and a single longitude-latitude pair. The fields are separated
 *     with tabulators. Empty location fields are allowed and are
 *     ignored.
 *
 * When the parameter lonFirst is true (the default), the first number
 * on the data line is a longitude, the latter a latitude.
 *
 * It is allowed to read multiple databases, the information is added
 * to the internal tables. It is allowed to define multiple coordinates
 * for the same location name in the datafiles, however the last one
 * read in will overwrite the earlier definitions.
 *
 * The method returns true on success.
 *
 * \param theFileName The name of the file to read
 * \param lonFirst True, if longitude is before latitude in the file
 * \param optionalFileType Undocumented
 * \return Undocumented
 */
// ======================================================================

bool NFmiLocationFinder::AddFile(const NFmiString& theFileName,
                                 bool lonFirst,
                                 bool optionalFileType)
{
  try
  {
    if (optionalFileType)
      return AddFileOfOptionalType(theFileName, lonFirst);

    // We expect max 6 location names, then 2 coordinates

    const unsigned int maxlocations = 6;
    const unsigned int maxwords = maxlocations + 2;
    const unsigned int coordinate1 = maxlocations;
    const unsigned int coordinate2 = coordinate1 + 1;

    // Open the file, returning false if the file cannot be read

    ifstream in(theFileName.CharPtr(), ios::in | ios::binary);
    if (!in)
      return false;

    // Process one line at a time

    string line;
    while (getline(in, line))
    {
      // Ignore the line if it is a comment line

      if (line[0] == '#' || line.substr(0, 2) == "//")
        continue;

      // Split the line from tabulators into a vector

      vector<string> fields;

      string::size_type pos1 = 0;
      while (pos1 < line.size())
      {
        string::size_type pos2 = line.find("\t", pos1);
        if (pos2 == string::npos)
          pos2 = line.size();
        fields.push_back(line.substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
      }

      // We require 6 text fields + 2 number fields + optional extra fields

      if (fields.size() < maxwords)
        continue;

      // Extract the numbers. Note that atof returns nothing
      // to indicate success, hence we use sscanf instead.

      double value1, value2;
      if (sscanf(fields[coordinate1].c_str(), "%lg", &value1) != 1)
        continue;
      if (sscanf(fields[coordinate2].c_str(), "%lg", &value2) != 1)
        continue;

      double longitude = lonFirst ? value1 : value2;
      double latitude = lonFirst ? value2 : value1;

      // Then add all non-empty names into the internal container

      NFmiPoint lonlat(longitude, latitude);
      for (unsigned int idx = 0; idx < maxlocations; idx++)
      {
        string word = fields[idx];
        if (word.empty())
          continue;

        // Convert the name to lower case

        word = NFmiStringTools::LowerCase(word);

        // And add it into the maps

        if (idx == 0)
          itsPrimaryPoints.insert(make_pair(word, lonlat));
        else
          itsSecondaryPoints.insert(make_pair(word, lonlat));
      }
    }  // Next line loop

    // Done inserting

    in.close();

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
 * \param lonFirst Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLocationFinder::AddFileOfOptionalType(const NFmiString& theFileName, bool lonFirst)

// this file type differs in the following aspects:
// - location(s?) and coordinates separated by spaces/tabs
// - names are not converted to lower cases
// -
// used by the Press application

{
  try
  {
    // We expect max 6 location names, then 2 coordinates

    const unsigned int maxlocations = 6;
    const unsigned int maxwords = maxlocations + 2;

    // Open the file, returning false if the file cannot be read

    ifstream in(theFileName.CharPtr(), ios::in | ios::binary);
    if (!in)
      return false;

    // Process one line at a time

    string line;
    int nerr = 0;
    int nlong = 0;
    while (getline(in, line))
    {
      // Ignore the line if it is a comment line or "thrash" line

      if (line[0] == '#' || line.substr(0, 2) == "//" || line.size() < 6)
        continue;

      // Split the line from white spaces into a vector

      vector<string> fields;

      string::size_type pos1 = 0;
      string::size_type pos2 = 0;
      vector<string>::size_type firstCoord = 0;
      pos1 = line.find_first_not_of("\t", pos2);
      while (pos1 < line.size())
      {
        pos2 = line.find_first_of("\t", pos1);
        if (pos2 == string::npos)
          pos2 = line.size();
        fields.push_back(line.substr(pos1, pos2 - pos1));
        if (line.substr(pos1, 1).find_first_of("01234567890-+.", 0) == 0 && firstCoord == 0)
        {
          firstCoord = fields.size();  // you must know which words shall be converted to doubles
          if (firstCoord > 2)
            nlong++;
        }

        if ((fields.size() > maxwords) || ((firstCoord > 0) && (fields.size() > firstCoord)))
          break;  // mahdolliset kommentit pois
        pos1 = line.find_first_not_of("\t", pos2);
      }

      // We require 1-6 text fields + 2 number fields + optional extra fields

      //      if(fields.size()<maxwords)
      //	continue;

      // Extract the numbers. Note that atof returns nothing
      // to indicate success, hence we use sscanf instead.

      double value1, value2;
      if (sscanf(fields[firstCoord - 1].c_str(), "%lg", &value1) != 1)
      {
        nerr++;
        continue;
      }
      if (sscanf(fields[firstCoord].c_str(), "%lg", &value2) != 1)
      {
        nerr++;
        continue;
      }

      double longitude = lonFirst ? value1 : value2;
      double latitude = lonFirst ? value2 : value1;

      // Then add all non-empty names into the internal container

      NFmiPoint lonlat(longitude, latitude);
      for (unsigned int idx = 0; idx < maxlocations && idx < firstCoord - 1; idx++)
      {
        string word = fields[idx];
        if (word.empty())
          continue;

        // Convert the name to lower case

        word = NFmiStringTools::LowerCase(word);

        // And add it into the map

        if (idx == 0)
          itsPrimaryPoints.insert(make_pair(word, lonlat));
        else
          itsSecondaryPoints.insert(make_pair(word, lonlat));
      }
    }  // Next line loop

    // Done inserting

    in.close();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Find the coordinates of the given named location.
 * If no match is found, NFmiPoint(kFloatMissing,kFloatMissing)
 * is returned. The case of the word is ignored in comparisons.
 * The status of the search is stored internally, and can be tested
 * with LastSearchFailed method.
 *
 * \param theName The name of the location
 * \return The coordinates of the location
 */
// ======================================================================

const NFmiPoint NFmiLocationFinder::Find(const NFmiString& theName)
{
  try
  {
    string name(theName.CharPtr());
    name = NFmiStringTools::LowerCase(name);

    LocationFinderMap::const_iterator iter;

    iter = itsPrimaryPoints.find(name);

    if (iter != itsPrimaryPoints.end())
    {
      itsLastSearchFailed = false;
      return iter->second;
    }

    iter = itsSecondaryPoints.find(name);

    itsLastSearchFailed = (iter == itsSecondaryPoints.end());
    if (itsLastSearchFailed)
      return NFmiPoint(kFloatMissing, kFloatMissing);
    else
      return iter->second;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
/*!
 * Find the WMO-number of the given named location.
 * The WMO-number must have been give as an alternative name in the
 * form wmoNNNNN as the second, third..... name.
 * The parameter name must have given as the first name.
 * If no match is found, 0 is returned.
 * The case of the word is ignored in comparisons.
 * The status of the search is stored internally, and can be tested
 * with LastSearchFailed method.
 *
 * \param theName The name of the location
 * \return The wmo-numer of the location
 */
// ======================================================================
unsigned long NFmiLocationFinder::FindWmo(const NFmiString& theName)
{
  try
  {
    string name(theName.CharPtr());
    string wmoName;
    name = NFmiStringTools::LowerCase(name);
    LocationFinderMap::const_iterator iter;
    iter = itsPrimaryPoints.find(name);
    unsigned long wmo;

    if (iter == itsPrimaryPoints.end())
    {
      itsLastSearchFailed = false;
      return 0;
    }
    NFmiPoint lonLat = iter->second;
    for (iter = itsSecondaryPoints.begin(); iter != itsSecondaryPoints.end(); ++iter)
    {
      // NFmiPoint lonLat2 = iter->second;
      // if(abs(lonLat2.X()-lonLat.X())< 0.5 && abs(lonLat2.Y()-lonLat.Y())< 0.5)
      if (iter->second ==
          lonLat)  // flotarin epätarkkuus voi aiheuttaa eron n:ssä desimaalissa (Madrid???)
      {
        wmoName = iter->first;
        if (wmoName.substr(0, 3) == "wmo" && wmoName.size() == 8)
        {
          wmoName.erase(0, 3);
          wmo = atoi(wmoName.c_str());
          return wmo;
        }
      }
    }
    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ======================================================================
