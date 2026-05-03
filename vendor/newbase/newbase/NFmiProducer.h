// ======================================================================
/*!
 * \file NFmiProducer.h
 * \brief Implementation of class NFmiProducer
 */
// ======================================================================
/*!
 * \class NFmiProducer
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiIndividual.h"

//! Undocumented
class NFmiProducer : public NFmiIndividual
{
 public:
  NFmiProducer();
  NFmiProducer(const NFmiProducer& theProducer);
  explicit NFmiProducer(unsigned long theIdent, const NFmiString& theName = "Kennel");

  NFmiProducer& operator=(const NFmiProducer& theProducer);
  bool operator==(const NFmiProducer& theProducer) const;

  virtual const char* ClassName() const;

 private:
};  // class NFmiProducer

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiProducer::NFmiProducer() {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIdent Undocumented
 * \param theName Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiProducer::NFmiProducer(unsigned long theIdent, const NFmiString& theName)
    : NFmiIndividual(theIdent, theName)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theProducer The object being copied
 */
// ----------------------------------------------------------------------

inline NFmiProducer::NFmiProducer(const NFmiProducer& theProducer) : NFmiIndividual(theProducer) {}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theProducer The object to compare with
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiProducer::operator==(const NFmiProducer& theProducer) const
{
  return GetIdent() == theProducer.GetIdent();
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theProducer The object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

inline NFmiProducer& NFmiProducer::operator=(const NFmiProducer& theProducer)
{
  NFmiIndividual::operator=(theProducer);
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiProducer::ClassName() const
{
  return "NFmiProducer";
}

// ======================================================================
