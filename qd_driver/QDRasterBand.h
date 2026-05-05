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

  // Bands here are timesteps of a single physical parameter, never colour
  // channels. Returning GCI_GrayIndex stops consumers like QGIS from
  // auto-picking a "Multiband color" renderer that maps bands 1/2/3 to R/G/B.
  GDALColorInterp GetColorInterpretation() override { return GCI_GrayIndex; }

 private:
  unsigned long itsTimeIndex;
};

}  // namespace GdalQueryData
}  // namespace SmartMet
