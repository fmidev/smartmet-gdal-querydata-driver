// ======================================================================
/*!
 * \file NFmiProducerName.h
 * \brief FmiProducerName enumeration
 */
// ======================================================================

#pragma once

//! Undocumented
enum FmiProducerName
{
  kFmiNoProducer = 0,       //!< Fmi number for Un specified producer
  kFmiHIRLAM = 1,           //!< Fmi number for Hirlam
  kFmiHIRMESO = 2,          //!< Fmi number for Hirmeso
  kFmiTAFHIR = 11,          //!< Fmi number for TAFHIR
  kFmiMesan = 104,          //!< Fmi number for smesan
  kFmiECMWF = 131,          //!< Fmi number for ECMWF
  kFmiSmhiMesan = 160,      //!< Fmi number for Smhi mesan2
  kFmiMTAHIRLAM = 230,      //!< Fmi number for MTAHIRLAM
  kFmiMTAHIRMESO = 220,     //!< Fmi number for MTAHIRMESO
  kFmiMTAECMWF = 240,       //!< Fmi number for MTAECMWF
  kFmiSYNOP = 1001,         //!< Fmi number for synop observations
  kFmiSHIP = 1002,          //!< Fmi number for SHIP observations
  kFmiTEMP = 1005,          //!< Fmi number for TEMP sounding observations
  kFmiRAWTEMP = 1006,       //!< Fmi number for RAW-TEMP sounding observations (used in MetEditor)
  kFmiFlashObs = 1012,      //!< Fmi number for flash detection producer
  kFmiRADARNRD = 1014,      //!< Fmi number for nrd radar
  kFmiBUOY = 1017,          //!< Fmi number for BUOY observations
  kFmiRoadObs = 1023,       //!< Fmi number for road observation station data
  kFmiMETAR = 1025,         //!< Fmi number for METAR messages
  kFmiBufrTEMP = 1034,      //!< Fmi number for TEMP sounding observations from bufr data format
  kFmiMAST = 1099,          //!< Fmi number for Mast observations
  kFmiTestBed = 1101,       //!< Fmi number for testbed vxt data
  kFmiHakeMessages = 1107,  //!< Fmi number for HAKE message data
  kFmiKaHaMessages = 1108,  //!< Fmi number for KaHa message data (kansalais havainto)
  kFmiMETEOR = 2001,        //!< Fmi number for meteorologist
  kFmiAUTOMATIC = 2004,     //!< Fmi number for automatic (derived) values
  kFmiTAFMET = 2011         //!< Fmi number for TAFMET
};

// ======================================================================
