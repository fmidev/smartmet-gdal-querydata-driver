#pragma once

#include <gdal_rasterband.h>

namespace SmartMet
{
namespace GdalQueryData
{
class QDDataset;

// One band = one time step of the dataset's fixed (param, level).
class QDRasterBand : public GDALRasterBand
{
 public:
  QDRasterBand(QDDataset* parent, int nBand, unsigned long timeIdx);

  CPLErr IReadBlock(int nBlockXOff, int nBlockYOff, void* pImage) override;
  double GetNoDataValue(int* pbSuccess) override;

 private:
  unsigned long itsTimeIndex;
};

}  // namespace GdalQueryData
}  // namespace SmartMet
