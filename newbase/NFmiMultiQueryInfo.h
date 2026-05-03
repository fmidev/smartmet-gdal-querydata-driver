// ======================================================================
/*!
 * \file NFmiMultiQueryInfo.h
 * \brief Interface of class NFmiMultiQueryInfo
 *
 * How does it work?
 *
 * We take as input any of the following:
 *
 *  1. filename containing querydata
 *  2. directory name containing multiple querydata
 *  3. list of filenames containing querydata
 *  4. a set of NFmiFastQueryInfo objects
 *
 * Cases 1-3 reduce to the same case - constructing from
 * a list of filenames. In this case we construct a list
 * of NFmiQueryData objects for which have no further use but
 * to keep the relevant objects alive. From these we
 * construct shared NFmiFastQueryInfo objects which we
 * shall actually use when referring to data.
 *
 * In case 4, which we also reach once initialization
 * of cases 1-3 complete, we initialize *this* based on
 * the NFmiFastQueryInfo object with the latest origin time.
 * Any other object is then required to have the same parameter,
 * level and horizontal descriptors. Any nonconforming data shall
 * not be used even though the data and its* NFmiFastQueryInfo
 * have already been constructed.
 *
 * For conforming data we maintain the NFmiFastQueryInfo
 * object, which is actually *this* from which we have
 * derived. Any descriptor changes apart from the time
 * descriptor will work fine. Whenever we change the time
 * index, we have to make sure we are referencing the correct
 * NFmiQueryData, its NFmiRawData, and have altered the
 * time index to point to the correct time within the
 * referenced NFmiQueryData. This is handled by initializing
 * a mapping from a global multi-time-index to individual
 * querydata objects and the time indices within them in
 * the constructor. Any change in the time index will
 * then simply have to make sure the relevant pointers
 * and time indices are properly set.
 *
 * What this DOES NOT work for:
 *
 * Any method which uses the time descriptor directly instead
 * of a virtual NFmiQueryInfo method and has not been overridden
 * in NFmiMultiQueryInfo.
 *
 * Any NFmiFastQueryInfo method which uses the time indices directly.
 *
 * What it DOES work for:
 *
 * Any method which has been verified to use virtual methods only:
 *
 *   Values methods
 *   Higher level InterpolateValue methods
 *
 *
 * To see how things work, see how TimeIndex(idx) is implemented.
 * Everything else follows.
 */
// ======================================================================

#pragma once

// Ilman seuraavaa define:ä Visual C++ 2012 kääntäjällä tulee käännösvirhe boostin
// detail/container_fwd.hpp -headerissa
// määriteltyjen ei-oletusarvoja käyttävien std::list (ja muiden std-containereiden) forward
// esittelyn kanssa.
// siellä on siis esi-esitelty std::list seuraavasti:
// template <class T, class Allocator> class list;
//, kun pitäisi olla (kuten onkin container/container_fwd.hpp)
// template <class T,class Allocator = std::allocator<T> > class list;
// Miksi ongelma ei esiinny Linux + gcc yhdistelmän kanssa, ei ole tietoa.
#define BOOST_DETAIL_NO_CONTAINER_FWD

#include "NFmiDataMatrix.h"
#include "NFmiFastQueryInfo.h"
#include <macgyver/DateTime.h>

//! Undocumented
class NFmiMultiQueryInfo : public NFmiFastQueryInfo
{
 public:
  NFmiMultiQueryInfo(const NFmiMultiQueryInfo& other) = delete;
  NFmiMultiQueryInfo& operator=(const NFmiMultiQueryInfo& other) = delete;
  NFmiMultiQueryInfo(NFmiMultiQueryInfo&& other) = delete;
  NFmiMultiQueryInfo& operator=(NFmiMultiQueryInfo&& other) = delete;

  
  using NFmiFastQueryInfo::InterpolatedValue;
  using NFmiFastQueryInfo::Time;

  NFmiMultiQueryInfo(const std::string& thePath);
  NFmiMultiQueryInfo(const std::list<std::string>& thePaths);
  NFmiMultiQueryInfo(std::vector<std::shared_ptr<NFmiFastQueryInfo> >& theInfos);

  using NFmiFastQueryInfo::OriginTime;
  const NFmiMetTime& OriginTime() const;  // we may have multiple origin times

  const NFmiMetTime& ValidTime() const;
  bool Time(const NFmiMetTime& theTime);

  void ResetTime();
  bool NextTime();
  bool FirstTime();
  bool LastTime();
  bool PreviousTime();
  unsigned long TimeIndex() const;
  bool TimeIndex(unsigned long theIndex);
  bool IsInside(const NFmiMetTime& theTime) const;

  float InterpolatedValue(const NFmiMetTime& theTime, int theMaxMinuteRange);

  bool FindNearestTime(const NFmiMetTime& theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing)
  {
    return TimeToNearestStep(theTime, theDirection, theTimeRangeInMinutes);
  }

  bool TimeToNearestStep(const NFmiMetTime& theTime,
                         FmiDirection theDirection,
                         long theTimeRangeInMinutes = kLongMissing);

  // This is used to store querydata objects if a filename or directory
  // is used to construct the object. If NFmiFastQueryInfos are used,
  // the container is empty

  std::list<std::shared_ptr<NFmiQueryData> > itsDatas;

  // This is used by all constructors

  std::vector<std::shared_ptr<NFmiFastQueryInfo> > itsInfos;

  // We index NFmiFastQueryInfo objects and their time indexes through this struct

  struct MultiIndex
  {
    MultiIndex(const NFmiMetTime& t, std::size_t i_index, unsigned long t_index)
        : valid_time(t), info_index(i_index), time_index(t_index)
    {
    }

    NFmiMetTime valid_time;
    std::size_t info_index;
    unsigned long time_index;

    bool operator<(const MultiIndex& obj) const { return valid_time < obj.valid_time; }
    bool operator<(const NFmiMetTime& t) const { return valid_time < t; }
  };

  typedef std::vector<MultiIndex> MultiIndexes;
  MultiIndexes itsMultiIndexes;

  unsigned long itsMultiTimeIndex;

  // Utility interpolation method needed in a couple places

  float InterpolatedValue(const NFmiMetTime& theTime,
                          const MultiIndexes::const_iterator& thePos,
                          int theMaxMinuteRange);

  // Utility initialization scripts for common parts of the constructors

  void Init(const std::list<std::string>& theFiles);
  void Init();

  // This is needed only for comparing file modification times
  std::vector<time_t> itsFileModificationTimes;

};  // class NFmiMultiQueryInfo

// ======================================================================
