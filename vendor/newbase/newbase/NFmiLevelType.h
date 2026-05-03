// ======================================================================
/*!
 * \file NFmiLevelType.h
 * \brief FmiLevelType enumeration
 */
// ======================================================================

#pragma once

//! Enumeraatio mahdollisista korkeuspintojen tyypeistä

enum FmiLevelType
{
  kFmiNoLevelType = 0,      //!< This is error or no initialized leveltype value
  kFmiGroundSurface = 1,    //!< Ground or water surface
  kFmiSoundingLevel = 50,   //!< Sounding level type
  kFmiAmdarLevel = 51,      //!< Amdar level type
  kFmiPressureLevel = 100,  //!< Isobaric level
  kFmiMeanSeaLevel = 102,   //!< ???
  kFmiAltitude = 103,       //!< Altitude above mean sea level
  kFmiHeight = 105,         //!< Height above ground
  kFmiHybridLevel = 109,    //!< NWP model level
  kFmiFlightLevel = 120,  //!< Flight Level tyyppi eli korkeus 100 jaloissa (FL150 = 150*100*0.305m)
  kFmiDepth = 160,        //!< Depth below mean sea level (meters)
  kFmiRoadClass1 = 169,   //!< Aurausmallin kunnossapitoluokka 1
  kFmiRoadClass2 = 170,   //!< Aurausmallin kunnossapitoluokka 2
  kFmiRoadClass3 = 171,   //!< Aurausmallin kunnossapitoluokka 3

  kFmi = 1001,             //!< Level of SYNOP observation, height 0...10 m
  kFmiAnyLevelType = 5000  //!< ???
};

// ======================================================================
