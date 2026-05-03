// ======================================================================
/*!
 * \file NFmiProducerIdLister.h
 * \brief Interface of class NFmiProducerIdLister
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiTimeDescriptor.h"

class NFmiQueryInfo;

class NFmiProducerIdLister
{
 public:
  NFmiProducerIdLister();
  NFmiProducerIdLister(const NFmiTimeDescriptor &theTimes, int theDefaultProducerId);
  NFmiProducerIdLister(const std::string &theProducersString);
  NFmiProducerIdLister(const NFmiProducerIdLister &theOther);
  NFmiProducerIdLister(NFmiQueryInfo &theInfo);  // Huom! pitäisi olla const info, mutta pari
                                                 // metodia pitää muuttaa ensin
  const std::string MakeProducerIdString();
  bool IntepretProducerIdString(const std::string &theString);
  bool ChangeTimeResolution(int theNewResolutionInMinutes);
  int ProducerId(const NFmiMetTime &theTime, bool fAllowInterpolation = false);
  int ProducerId(int theIndex) const;
  void ProducerId(const NFmiMetTime &theTime, int theProducerId);
  void ProducerId(int theIndex, int theProducerId);

  const NFmiMetTime &ModelOriginTime(const NFmiMetTime &theTime, bool fAllowInterpolation = false);
  const NFmiMetTime &ModelOriginTime(int theIndex) const;
  void ModelOriginTime(const NFmiMetTime &theTime, const NFmiMetTime &theOriginTime);
  void ModelOriginTime(int theIndex, const NFmiMetTime &theOriginTime);

  const NFmiTimeDescriptor &Times() const { return itsTimes; }
  const std::vector<int> &ProducerIds() const { return itsProducerIds; }
  const std::vector<NFmiMetTime> &ModelOriginTimes() const { return itsModelOriginTimes; }
  void DefaultProducerId(int value) { itsDefaultProducerId = value; }
  int DefaultProducerId() const { return itsDefaultProducerId; }
  const std::string &ProducerString() const { return itsProducerString; }
  void ProducerString(const std::string &value) { itsProducerString = value; }
  bool IsEmpty() const;

 private:
  NFmiTimeDescriptor itsTimes;
  std::string itsProducerString;
  std::vector<int> itsProducerIds;               // tähän talletetaan kunkin ajan tuottaja id
  std::vector<NFmiMetTime> itsModelOriginTimes;  // tähän talletetaan kunkin ajan mallin origin aika
  int itsDefaultProducerId;
};
