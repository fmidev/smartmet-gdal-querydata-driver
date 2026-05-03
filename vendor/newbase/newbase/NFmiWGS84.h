#pragma once

#include "NFmiGlobals.h"

namespace Fmi
{
class SpatialReference;
}

class NFmiWGS84
{
 public:
  static const Fmi::SpatialReference& SpatialReference();

 private:
  NFmiWGS84() = delete;
  NFmiWGS84(const NFmiWGS84& other) = delete;
  NFmiWGS84& operator=(const NFmiWGS84& other) = delete;
};
