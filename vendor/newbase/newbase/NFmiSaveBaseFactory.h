// ======================================================================
/*!
 * \file
 * \brief Interface of NFmiSaveBaseFactory
 */
// ======================================================================

#pragma once

const unsigned long kNFmiSaveBase = 0;
const unsigned int kNFmiArea = 1;

const unsigned int kNFmiGdalArea = 8;
const unsigned int kNFmiLambertEqualArea = 9;  // Epsg:3035
const unsigned int kNFmiLatLonArea = 10;
const unsigned int kNFmiRotatedLatLonArea = 11;
// const unsigned int kNFmiPolSterArea			= 12;	// deprecated
const unsigned int kNFmiStereographicArea = 13;
const unsigned int kNFmiPKJArea = 14;
const unsigned int kNFmiYKJArea = 15;
const unsigned int kNFmiEquiDistArea = 16;
const unsigned int kNFmiGnomonicArea = 17;
const unsigned int kNFmiKKJArea = 18;
const unsigned int kNFmiMercatorArea = 19;

const unsigned int kNFmiQueryData = 20;
const unsigned int kNFmiQueryInfo = 21;
const unsigned int kNFmiRadarQueryInfo = 22;
const unsigned int kNFmiSatelQueryInfo = 23;

const unsigned int kNFmiSatelQueryData = 29;
const unsigned int kNFmiRadarQueryData = 30;

const unsigned int kNFmiLocationBag = 31;
const unsigned int kNFmiStationBag = 32;
const unsigned int kNFmiTimeDescriptor = 33;
const unsigned int kNFmiParamDescriptor = 34;
const unsigned int kNFmiHPlaceDescriptor = 35;
const unsigned int kNFmiVPlaceDescriptor = 36;

const unsigned int kNFmiLocation = 37;
const unsigned int kNFmiStation = 38;
const unsigned int kNFmiRadarStation = 39;

const unsigned int kNFmiGrid = 41;

const unsigned int kNFmiTaf = 51;
const unsigned int kNFmiMetar = 52;

const unsigned int kNFmiString = 54;
const unsigned int kNFmiStatusString = 55;
const unsigned int kNFmiStatusPositionString = 56;

const unsigned int kNFmiRadar = 60;
const unsigned int kNFmiRadarIrisInfo = 61;
const unsigned int kNFmiRadarNordRadInfo = 62;
const unsigned int kNFmiRadarData = 63;

const unsigned int kNFmiPpiInfo = 70;
const unsigned int kNFmiCappiInfo = 71;
const unsigned int kNFmiEchoTopInfo = 72;
const unsigned int kNFmiRhiInfo = 73;
const unsigned int kNFmiRainInfo = 74;
const unsigned int kNFmiRawInfo = 75;
const unsigned int kNFmiCrossInfo = 76;
const unsigned int kNFmiVvpInfo = 77;
const unsigned int kNFmiWindInfo = 78;
const unsigned int kNFmiMaxInfo = 79;
const unsigned int kNFmiVilInfo = 80;
const unsigned int kNFmiGribInfo = 81;
const unsigned int kNFmiGrib = 82;

const unsigned int kNFmiWebMercatorArea = 83;
const unsigned int kNFmiLambertConformalConicArea = 84;

const unsigned int KNFmiMaxClass = 99;

void* CreateSaveBase(unsigned int classId);

// ======================================================================
