#pragma once

#include <gdal_priv.h>
#include <gdal_multidim.h>

#include <memory>
#include <string>
#include <vector>

namespace SmartMet
{
namespace GdalQueryData
{
class QDDataset;

// Builds the root-level GDALGroup that exposes the QD as a multidimensional
// dataset: one GDALMDArray per (parameter, sub-parameter) at all (time, level)
// combinations, plus coordinate variables for the dimensions.
//
// Dimensions and coordinate variables are owned by the group and shared (by
// shared_ptr) into every data array, so they survive as long as anything
// references them.
std::shared_ptr<GDALGroup> buildRootGroup(QDDataset& ds);

}  // namespace GdalQueryData
}  // namespace SmartMet
