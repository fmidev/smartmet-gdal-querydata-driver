#include "QDDataset.h"
#include "QDWrite.h"

#include <gdal_driver.h>
#include <gdal_drivermanager.h>
#include <gdal_priv.h>

namespace
{
constexpr const char* kDriverName = "querydata";

void registerQuerydataDriver()
{
  if (GDALGetDriverByName(kDriverName) != nullptr) return;

  auto* driver = new GDALDriver();
  driver->SetDescription(kDriverName);
  driver->SetMetadataItem(GDAL_DCAP_RASTER, "YES");
  driver->SetMetadataItem(GDAL_DCAP_MULTIDIM_RASTER, "YES");
  driver->SetMetadataItem(GDAL_DCAP_OPEN, "YES");
  driver->SetMetadataItem(GDAL_DCAP_CREATECOPY, "YES");
  driver->SetMetadataItem(GDAL_DCAP_SUBCREATECOPY, "NO");
  driver->SetMetadataItem(
      GDAL_DMD_CREATIONOPTIONLIST,
      "<CreationOptionList>"
      "  <Option name='PARAM_ID'        type='int'    description='FMI parameter id'/>"
      "  <Option name='PARAM_NAME'      type='string' description='Parameter name'/>"
      "  <Option name='ORIGIN_TIME'     type='string' description='ISO 8601 origin time'/>"
      "  <Option name='START_TIME'      type='string' description='ISO 8601 first valid time'/>"
      "  <Option name='TIMESTEP_MINUTES' type='int'   description='Minutes between bands'/>"
      "  <Option name='LEVEL_VALUE'     type='float'  description='Level value'/>"
      "  <Option name='LEVEL_TYPE'      type='int'    description='FmiLevelType'/>"
      "</CreationOptionList>");
  driver->SetMetadataItem(GDAL_DMD_LONGNAME, "FMI QueryData");
  driver->SetMetadataItem(GDAL_DMD_EXTENSIONS, "sqd fqd");
  driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "https://github.com/fmidev/smartmet-library-newbase");
  driver->SetMetadataItem(GDAL_DMD_SUBDATASETS, "YES");
  driver->SetMetadataItem(GDAL_DCAP_VIRTUALIO, "NO");

  driver->pfnOpen = SmartMet::GdalQueryData::QDDataset::Open;
  driver->pfnIdentify = SmartMet::GdalQueryData::QDDataset::Identify;
  driver->pfnCreateCopy = SmartMet::GdalQueryData::createCopy;

  GetGDALDriverManager()->RegisterDriver(driver);
}

}  // namespace

// GDAL plugin entry point. The shared object is loaded by the GDAL driver
// manager from $GDAL_DRIVER_PATH, and this symbol is looked up by name.
extern "C" void CPL_DLL GDALRegisterMe()
{
  registerQuerydataDriver();
}
