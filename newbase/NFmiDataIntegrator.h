// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiDataIntegrator
 */
// ======================================================================

#pragma once

class NFmiDataModifier;
class NFmiFastQueryInfo;
class NFmiIndexMask;
class NFmiIndexMaskSource;
class NFmiMetTime;

namespace NFmiDataIntegrator
{
// Integrate over time with current location & param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier);

// Integrate over time with subintervals w/ current location & param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                int theInterval,
                NFmiDataModifier& theSubTimeModifier,
                NFmiDataModifier& theMainTimeModifier);

// Integrate over grid with current param & time & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier);

// Integrate over grid and time with current param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier);

// Integrate over time and grid with current param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier);

// Integrate over grid and time with time dependend mask, current P & L

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiIndexMaskSource& theMaskSource,
                NFmiDataModifier& theSpaceModifier,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                NFmiDataModifier& theTimeModifier);

// Integrate over time, time and grid with current param & level

float Integrate(NFmiFastQueryInfo& theQI,
                const NFmiMetTime& theStartTime,
                const NFmiMetTime& theEndTime,
                int theInterval,
                NFmiDataModifier& theSubTimeModifier,
                NFmiDataModifier& theMainTimeModifier,
                const NFmiIndexMask& theIndexMask,
                NFmiDataModifier& theSpaceModifier);

}  // namespace NFmiDataIntegrator

// ======================================================================
