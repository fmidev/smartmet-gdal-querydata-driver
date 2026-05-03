#include "NFmiGlobals.h"

namespace Fmi
{
// Legacy mode detection before moving on to using the PROJ.X libraries only
bool LegacyMode()
{
#ifdef WGS84
  return false;
#else
  return true;
#endif
}
}  // namespace Fmi
