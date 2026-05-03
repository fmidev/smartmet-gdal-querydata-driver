// ======================================================================
/*!
 * \file NFmiLocationBag.cpp
 * \brief Implementation of class NFmiLocationBag
 */
// ======================================================================
/*!
 * \class NFmiLocationBag
 *
 * Undocumented
 *
 * \todo Convert itsSortedLocations from set<NFmiLocation> to
 *       set<NFmiLocation *> to save memory.
 */
// ======================================================================

#include "NFmiLocationBag.h"
#include "NFmiArea.h"
#include "NFmiRadarStation.h"
#include "NFmiStation.h"
#include "NFmiValueString.h"
#include "NFmiVersion.h"
#include <macgyver/Hash.h>
#include <gis/CoordinateMatrix.h>
#include <macgyver/Exception.h>
#include <functional>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLocationBag::~NFmiLocationBag()
{
  try
  {
    Destroy();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLocationBag::NFmiLocationBag() : NFmiSize(), itsLocations(), itsSortedLocations(), itsNearTree()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor based on a single location. You can achieve the same
 * thing by using a void constructor and by calling AddLocation.
 * Hence this constructor seems an unnecessary cluttering of the
 * class interface, and should probably be deprecated.
 *
 * \param theLocation Undocumented
 */
// ----------------------------------------------------------------------

NFmiLocationBag::NFmiLocationBag(const NFmiLocation &theLocation)
    : NFmiSize(), itsLocations(), itsSortedLocations(), itsNearTree()
{
  try
  {
    AddLocation(theLocation);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor based on an array of stations. All the stations
 * in the array are added to the constructed bag.
 *
 * \param theLocationArray The array of NFmiStation objects
 * \param numberOfLocations The size of the array
 */
// ----------------------------------------------------------------------

NFmiLocationBag::NFmiLocationBag(const NFmiStation *theLocationArray,
                                 unsigned long numberOfLocations)
    : NFmiSize(numberOfLocations), itsLocations(), itsSortedLocations(), itsNearTree()
{
  try
  {
    itsLocations.reserve(numberOfLocations);
    for (unsigned long i = 0; i < numberOfLocations; i++)
    {
      AddLocation(*theLocationArray);
      theLocationArray++;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor based on an array of locations. All the locations
 * in the array are added to the constructed bag.
 *
 * \param theLocationArray The array of NFmiLocation objects
 * \param numberOfLocations The size of the array
 */
// ----------------------------------------------------------------------

NFmiLocationBag::NFmiLocationBag(const NFmiLocation *theLocationArray,
                                 unsigned long numberOfLocations)
    : NFmiSize(numberOfLocations), itsLocations(), itsSortedLocations(), itsNearTree()
{
  try
  {
    itsLocations.reserve(numberOfLocations);
    for (unsigned long i = 0; i < numberOfLocations; i++)
    {
      AddLocation(*theLocationArray);
      theLocationArray++;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor based on an array of pointers to locations. All the
 * locations pointed to in the array are added to the constructed
 * bag. Note that the bag takes ownership of the pointers, no
 * cloning is done.
 *
 * \param theLocationArray The array of pointers to NFmiLocation objects.
 * \param numberOfLocations Size of the array.
 */
// ----------------------------------------------------------------------

NFmiLocationBag::NFmiLocationBag(NFmiLocation **theLocationArray, unsigned long numberOfLocations)
    : NFmiSize(numberOfLocations), itsLocations(), itsSortedLocations(), itsNearTree()
{
  try
  {
    itsLocations.reserve(numberOfLocations);
    for (unsigned long i = 0; i < numberOfLocations; i++)
      AddLocation(*theLocationArray[i]);
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
 * \param theLocationBag The object being copied
 */
// ----------------------------------------------------------------------

NFmiLocationBag::NFmiLocationBag(const NFmiLocationBag &theLocationBag)
    : NFmiSize(theLocationBag), itsLocations(), itsSortedLocations(), itsNearTree()
{
  try
  {
    itsLocations.reserve(theLocationBag.itsLocations.size());

    for (unsigned long i = 0; i < theLocationBag.itsLocations.size(); i++)
      AddLocation(*(theLocationBag.itsLocations[i]), false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destroy the internal contents of the object
 *
 */
// ----------------------------------------------------------------------

void NFmiLocationBag::Destroy()
{
  try
  {
    for (unsigned long i = 0; i < itsLocations.size(); i++)
      delete itsLocations[i];

    itsLocations.clear();
    itsSortedLocations.clear();
    itsNearTree.Clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Create a new bag from the unique locations in this and the
 * given bag.
 *
 * \param theBag The bag to combine with this.
 * \return The new bag containing the combined locations.
 */
// ----------------------------------------------------------------------

const NFmiLocationBag NFmiLocationBag::Combine(const NFmiLocationBag &theBag)
{
  try
  {
    NFmiLocationBag outbag;
    outbag.itsLocations.reserve(itsLocations.size() + theBag.itsLocations.size());

    StorageType::const_iterator begin = itsLocations.begin();
    StorageType::const_iterator end = itsLocations.end();
    StorageType::const_iterator iter;
    for (iter = begin; iter != end; ++iter)
      outbag.AddLocation(*(*iter), true);

    begin = theBag.itsLocations.begin();
    end = theBag.itsLocations.end();
    for (iter = begin; iter != end; ++iter)
      outbag.AddLocation(*(*iter), true);

    return outbag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return pointer to the current location. Note that the returned
 * pointer must be const, otherwise the internal set may become
 * corrupted.
 *
 * \return The current location, or 0 pointer if none is active.
 */
// ----------------------------------------------------------------------

const NFmiLocation *NFmiLocationBag::Location() const
{
  try
  {
    if (CurrentIndex() == -1)
      return nullptr;

    return itsLocations[CurrentIndex()];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Test if the given location exists in the bag.
 *
 * \param theLocation The location to search for
 * \return True if the location is already in the bag
 */
// ----------------------------------------------------------------------

bool NFmiLocationBag::Location(const NFmiLocation &theLocation)
{
  try
  {
    NFmiNearTreeLocation searchloc(theLocation, 0);

    // Search with a zero radius to get an exact coordinate match
    NFmiNearTreeLocation result;
    if (itsNearTree.NearestPoint(result, searchloc, 0))
    {
      itsIndex = result.GetIndex();
      return true;
    }
    else
    {
      Reset();
      return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Find the location nearest to the given location. If the nearest one
 * is within (<=) the given maximum distance, the current location is set
 * to the nearest location. Otherwise the current location is preserved.
 *
 * \param theLocation The location for which to find the nearest one
 * \param theMaxDistance The maximum allowed distance
 * \return True if the nearest location is within the maximum allowed distance
 */
// ----------------------------------------------------------------------

bool NFmiLocationBag::NearestLocation(const NFmiLocation &theLocation, double theMaxDistance)
{
  try
  {
    NFmiNearTreeLocation searchloc(theLocation, 0);

    // Equivalent unit sphere cord distance
    // Ref: https://en.wikipedia.org/wiki/Great-circle_distance

    theMaxDistance = std::min(theMaxDistance, kPii * kRearth);  // limit cannot exceed a half circle
    double cordlimit = 2 * sin(theMaxDistance / (2 * kRearth));

    NFmiNearTreeLocation result;
    if (itsNearTree.NearestPoint(result, searchloc, cordlimit))
    {
      itsIndex = result.GetIndex();
      return true;
    }
    else
    {
      return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Find the location nearest to the given location. If the nearest one
 * is within (<=) the given maximum distance, the current location is set
 * to the nearest location. Otherwise the current location is preserved.
 *
 * \param theLocation The location for which to find the nearest one
 * \param theMaxDistance The maximum allowed distance
 * \param theArea Wanted point must be inside this area.
 * \return True if the nearest location is within the maximum allowed distance
 */
// ----------------------------------------------------------------------

bool NFmiLocationBag::NearestLocation(const NFmiLocation &theLocation,
                                      const NFmiArea *theArea,
                                      double theMaxDistance)
{
  try
  {
    // NFmiNearTree does not sort the results in increasing order
    // of distance, hence this cannot be used.

    bool found = false;
    unsigned long best_index = 0;
    double best_distance = 0;

    for (unsigned long i = 0; i < itsLocations.size(); i++)
    {
      double dist = theLocation.Distance(*itsLocations[i]);
      if (dist <= theMaxDistance && (!found || dist < best_distance))
      {
        if (theArea->IsInside(itsLocations[i]->GetLocation()))  // optimointi kysymys, kumpi
                                                                // hitaampi Distance vai IsInside?
        {
          found = true;
          best_index = i;
          best_distance = dist;
        }
      }
    }

    // Set current to best location, if one was found
    if (found)
      itsIndex = best_index;

    return found;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Add a new location to the bag. If the given flag is true, the bag
 * is first searched for an equal location. If one is found, the location
 * is not added and false is returned. The flag should be false only
 * when it is known the location is not already in the bag, and hence
 * the time consuming search loop can be avoided.
 *
 * \param theLocation The location to add.
 * \param theChecking If true, duplicates will not be added
 * \return True if the location was added
 */
// ----------------------------------------------------------------------

bool NFmiLocationBag::AddLocation(const NFmiLocation &theLocation, bool theChecking)
{
  try
  {
    if (theChecking)
      if (itsSortedLocations.find(theLocation) != itsSortedLocations.end())
        return false;

    Add(theLocation);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Add a location to the internal variables, no checking is done
 * to see if the location is already in the bag.
 *
 * \param theLocation The location to add
 */
// ----------------------------------------------------------------------

void NFmiLocationBag::Add(const NFmiLocation &theLocation)
{
  try
  {
    NFmiLocation *tmp = theLocation.Clone();
    itsLocations.push_back(tmp);
    itsSortedLocations.insert(*tmp);
    itsSize = itsLocations.size();  // safer than itsSize++
    itsNearTree.Insert(NFmiNearTreeLocation(*tmp, itsSize - 1));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 * \bug itsLocations size is not checked to be nonempty, but
 *      element 0 is referenced anyway.
 */
// ----------------------------------------------------------------------

std::ostream &NFmiLocationBag::Write(std::ostream &file) const
{
  try
  {
    NFmiSize::Write(file);

    // We trust all data to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file << itsLocations[0]->ClassId() << std::endl;
    }

    for (unsigned long i = 0; i < itsLocations.size(); i++)
    {
      file << *(itsLocations[i]);
    }
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

std::istream &NFmiLocationBag::Read(std::istream &file)
{
  try
  {
    Destroy();

    NFmiSize::Read(file);
    int classID = kNFmiStation;

    // We trust all data to be at least version 6 by now
    if (DefaultFmiInfoVersion >= 4)
    {
      file >> classID;
    }

    unsigned long newSize = GetSize();
    itsLocations.reserve(newSize);

    NFmiLocation *temp;

    switch (classID)
    {
      case kNFmiLocation:
      {
        temp = new NFmiLocation;
        break;
      }
      case kNFmiStation:
      {
        temp = new NFmiStation;
        break;
      }
      case kNFmiRadarStation:
      {
        temp = new NFmiRadarStation;
        break;
      }
      default:
      {
        temp = new NFmiLocation;
      }
    }

    // Note: Must use newSize, not GetSize in the loop, since Add modifies itsSize
    for (unsigned long i = 0; i < newSize; i++)
    {
      file >> *temp;
      Add(*temp);
    }
    delete temp;

    Reset();

    // This is enough to make using itsNearTree thread safe if no more points are added
    itsNearTree.Flush();

    return file;
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
 * \param theLocationBag The other object being copied
 * \return The object assigned to
 * \bug Must protect from self assignment
 * \todo Should call NFmiSiz::operator=
 */
// ----------------------------------------------------------------------

NFmiLocationBag &NFmiLocationBag::operator=(const NFmiLocationBag &theLocationBag)
{
  try
  {
    Destroy();

    itsIndex = theLocationBag.CurrentIndex();
    itsSize = theLocationBag.GetSize();

    itsLocations.reserve(theLocationBag.itsLocations.size());
    for (unsigned int i = 0; i < theLocationBag.itsLocations.size(); i++)
      Add(*(theLocationBag.itsLocations[i]));

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theLocationBag The object being compared to
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiLocationBag::operator==(const NFmiLocationBag &theLocationBag) const
{
  try
  {
    if (itsLocations.size() != theLocationBag.itsLocations.size())
      return false;

    for (int i = 0; i < static_cast<int>(this->GetSize()); i++)
    {
      if (!(this->itsLocations[i]->IsEqual(*(theLocationBag.itsLocations[i]))))
        return false;
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//! Undocumented typedef

typedef pair<int, double> IndDistPari;

// ----------------------------------------------------------------------
/*!
 * Huom! T:n pitää olla pair<>-tyyppinen!!!!
 */
// ----------------------------------------------------------------------

template <typename T>
struct LocationIndexDistanceLess
{
  bool operator()(const T &eka, const IndDistPari &toka) { return eka.second < toka.second; }
};

// ----------------------------------------------------------------------
/*!
 * Huom! T:n pitää olla pair<>-tyyppinen!!!!
 */
// ----------------------------------------------------------------------
template <typename T>
struct LocationIndexDistanceGreater
{
  LocationIndexDistanceGreater(double theDistance) : itsDistance(theDistance) {}
  bool operator()(const T &indDist) { return indDist.second > itsDistance; }
  double itsDistance;
};

// ----------------------------------------------------------------------
/*!
 * Hakee listan paikkaindeksi/etäisyys metreinä pareja.
 * Listaan haetaan annettua paikkaa lähimmat datapisteet järjestyksessä
 * lähimmästä kauimpaan. Listaan haetaan joko haluttu määrä lähimpiä pisteitä
 * tai hakua voi myös rajoittaa maksimi etäisyydellä. Jos maksimi määräksi
 * laitetaan -1, haetaan paikkoja niin paljon kuin löytyy (maxEtäisyys rajoitus
 * huomiooon ottaen).
 *
 * Huom! maxWantedLocations ohittaa maxDistance määrityksen (jos molemmat
 * annettu).
 *
 * Erilaisia kombinaatioita haun rajoituksessa:
 *
 * \li theMaxWantedLocations = -1 ja theMaxDistance = kFloatMissing =>
 *     palauttaa kaikki sortattuna.
 * \li theMaxWantedLocations = 5 ja theMaxDistance = kFloatMissing =>
 *     palauttaa 5 sortattuna.
 * \li theMaxWantedLocations = -1 ja theMaxDistance = 45678.9m =>
 *     palauttaa kaikki annetun säteen sisältä sortattuna.
 * \li theMaxWantedLocations = 5 ja theMaxDistance = 45678.9m =>
 *     palauttaa max 5 annetun säteen sisältä sortattuna.
 *
 * \param theLocation The center location for the search
 * \param theMaxWantedLocations Maximum number of locations to return, or -1
 * \param theMaxDistance Maximum distance allowed, or kFloatMissing
 * \return Vector of location indices and distances
 */
// ----------------------------------------------------------------------

const std::vector<pair<int, double> > NFmiLocationBag::NearestLocations(
    const NFmiLocation &theLocation, int theMaxWantedLocations, double theMaxDistance) const
{
  try
  {
    auto size = static_cast<int>(this->GetSize());
    std::vector<IndDistPari> tempValues(size, make_pair(-1, kFloatMissing));
    for (int i = 0; i < size; i++)
      tempValues[i] = make_pair(i, theLocation.Distance(*this->itsLocations[i]));

    if (theMaxWantedLocations == -1 && theMaxDistance == kFloatMissing)
    {
      // molemmat rajoittimet puuttuvat, palautetaan kaikki sortattuna
      std::sort(tempValues.begin(), tempValues.end(), LocationIndexDistanceLess<IndDistPari>());
      return tempValues;
    }

    if (theMaxWantedLocations != -1 && theMaxDistance == kFloatMissing)
    {
      if (tempValues.size() == 0)
        return std::vector<pair<int, double> >();
      else
      {
        // halutaan n kpl lahimpiä paikkoja
        int usedCount = theMaxWantedLocations;
        if (usedCount > static_cast<int>(tempValues.size()))
          usedCount =
              static_cast<int>(tempValues.size());  // maksimissaan voidaan sortata size:iin asti
        std::partial_sort(tempValues.begin(),
                          tempValues.begin() + usedCount,
                          tempValues.end(),
                          LocationIndexDistanceLess<IndDistPari>());
        // palautetaan haluttu määrä locatioita
        return std::vector<IndDistPari>(tempValues.begin(), tempValues.begin() + usedCount);
      }
    }

    // theMaxDistance != kFloatMissing)

    // haetaan kaikki annetun säteen sisällä olevat paikat
    std::sort(tempValues.begin(), tempValues.end(), LocationIndexDistanceLess<IndDistPari>());
    auto pos = std::find_if(tempValues.begin(),
                            tempValues.end(),
                            LocationIndexDistanceGreater<IndDistPari>(theMaxDistance));

    if (theMaxWantedLocations != -1)
    {
      auto usedMaxLocationCount =
        (tempValues.size() >= theMaxWantedLocations) ? theMaxWantedLocations : tempValues.size();
      auto maxWantedPos = tempValues.begin() + usedMaxLocationCount;
      if (pos > maxWantedPos) 
        pos = maxWantedPos;
    }

    if (pos == tempValues.end())
      return tempValues;
    else
      return std::vector<IndDistPari>(tempValues.begin(), pos);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//----------------------------------------------------------------------
/*!
 * \brief Test if the given point is in the data
 */
// ----------------------------------------------------------------------

bool NFmiLocationBag::IsInside(const NFmiPoint &theLatLon, double theRadius) const
{
  try
  {
    // Equivalent unit sphere cord distance
    // Ref: https://en.wikipedia.org/wiki/Great-circle_distance

    theRadius = std::min(theRadius, kPii * kRearth);  // limit cannot exceed a half circle

    double cordlimit = 2 * sin(theRadius / (2 * kRearth));

    // The location to search for
    NFmiLocation location(theLatLon);
    NFmiNearTreeLocation searchloc(location, 0);

    // We'll ignore this result, we just want to know if there is one
    NFmiNearTreeLocation result;
    return itsNearTree.NearestPoint(result, searchloc, cordlimit);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the set of locations
 */
// ----------------------------------------------------------------------

std::size_t NFmiLocationBag::HashValue() const
{
  try
  {
    std::size_t hash = 0;

    for (NFmiLocation *location : itsLocations)
    {
      if (location != nullptr)
        Fmi::hash_combine(hash, location->HashValue());
    }

    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the native coordinates in a 2D matrix
 */
// ----------------------------------------------------------------------

Fmi::CoordinateMatrix NFmiLocationBag::CoordinateMatrix() const
{
  try
  {
    // Note: The coordinates are assumed to be WGS84. We return them in
    // a 1D vector since there is no structure.

    const auto ysize = 1;
    Fmi::CoordinateMatrix matrix(itsLocations.size(), ysize);

    const auto j = 0;
    std::size_t i = 0;
    for (const auto *location : itsLocations)
    {
      if (location == nullptr)
        ++i;
      else
        matrix.set(i++, j, location->GetLocation());
    }
    return matrix;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
