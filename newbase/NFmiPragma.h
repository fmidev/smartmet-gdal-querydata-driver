// ======================================================================
/*!
 * \file NFmiPragma.h
 * \brief Portable pragmas
 */
// ======================================================================

#pragma once

#ifdef _MSC_VER

#define PRAGMA_WARNING_PUSH() __pragma(warning(push))
#define PRAGMA_WARNING_DISABLE(X) __pragma(warning(disable : X))
#define PRAGMA_WARNING_POP() __pragma(warning(pop))

#else

#define PRAGMA_WARNING_PUSH()
#define PRAGMA_WARNING_DISABLE(X)
#define PRAGMA_WARNING_POP()

#endif
