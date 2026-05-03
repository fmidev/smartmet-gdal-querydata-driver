// ======================================================================
/*!
 * \file NFmiDef.h
 * \brief Some globally essential definitions.
 */
// ======================================================================

#pragma once

//! Directory separator is system dependent

#ifdef UNIX
const unsigned char kFmiDirectorySeparator = '/';
#else
const unsigned char kFmiDirectorySeparator = '\\';
#endif

// ======================================================================
