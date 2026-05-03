// ======================================================================
/*!
 * \file NFmiGeomList.h
 * \brief Declaration of struct GeomList
 */
// ======================================================================
/*!
 * \struct GeomList
 *
 * \deprecated
 * Näyttää siltä, ettei GeomList structia eikä GEOMNAMEMAXSIZE määritelmää
 * käytetää missään kirjastossa. Siten se tulisi poistaa. Toisaalta koodi
 * on niin C-tyyppistä, ettei se kuulu C++ kirjastoon.
 *
 * \todo Ota selvää, tarvitaanko tätä tiedostoa. Ilmeisesti struct on
 * Vilin määrittelemä.
 */
// ======================================================================

#pragma once

#define GEOMNAMEMAXSIZE 15

//! Jonkinlainen struct nimien ja kokojen säilytämiseen
typedef struct
{
  long size;
  char geomName[GEOMNAMEMAXSIZE][GEOMNAMELENGTH];
} GeomList;

// ======================================================================
