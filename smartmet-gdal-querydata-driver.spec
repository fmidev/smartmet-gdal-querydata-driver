%define DIRNAME gdal-querydata-driver
%define SPECNAME smartmet-%{DIRNAME}
%define DRIVERNAME querydata
%define GDAL_PLUGIN_DIR %{_libdir}/gdalplugins

%global __brp_check_rpaths %{nil}

Summary: GDAL driver for FMI QueryData (.sqd) files
Name: %{SPECNAME}
Version: 26.5.21
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-gdal-querydata-driver
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# The smartmet libraries (newbase, macgyver, gis) are statically linked into
# the plugin from their *-static / *-devel packages, so the installed
# gdal_querydata.so has no runtime dependency on smartmet-library-* shared
# objects — only on Fedora-stock libraries.

BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: rpm-build
BuildRequires: gdal312-devel
BuildRequires: geos313-devel
BuildRequires: proj97-devel
BuildRequires: fmt-devel
BuildRequires: libicu-devel
BuildRequires: boost-devel
BuildRequires: double-conversion-devel
BuildRequires: sqlite-devel
BuildRequires: libcurl-devel
BuildRequires: smartmet-library-newbase-devel
BuildRequires: smartmet-library-newbase-static
BuildRequires: smartmet-library-macgyver-devel
BuildRequires: smartmet-library-macgyver-static
BuildRequires: smartmet-library-gis-devel
BuildRequires: smartmet-library-gis-static

# Runtime requirements are deliberately tight: this list mirrors the NEEDED
# entries actually present in the .so (verify with `objdump -p .so | grep
# NEEDED`). The smartmet libraries are statically linked, so their shared
# variants are not required at runtime. geos/proj/icu/double-conversion are
# reachable transitively via libgdal but the driver itself never references
# them, so --as-needed drops them.
Requires: gdal312-libs
Requires: fmt-libs
Requires: boost-iostreams
Requires: boost-thread
# tzdata is a data-only dep, read at runtime by Howard Hinnant's date library
# (USE_OS_TZDB=1) from /usr/share/zoneinfo.
Requires: tzdata

Provides: %{SPECNAME}

%description
Out-of-tree GDAL plugin that exposes FMI QueryData (.sqd, .fqd) files as a
GDAL raster dataset. Once installed, gdalinfo, gdal_translate, gdalwarp,
QGIS, rasterio, xarray, and any other GDAL consumer can read .sqd files
directly. Each (parameter, level) combination is exposed as a subdataset,
with one band per time step. Composite parameters (TotalWind, Weather-
AndCloudiness) are unpacked into their sub-components. The plugin also
exposes the data via GDAL's Multi-Dimensional Raster API for xarray /
NetCDF / Zarr workflows, and supports CreateCopy for round-tripping
single-parameter rasters back to .sqd via gdal_translate.

QueryData is a legacy FMI-internal format and is not part of upstream
GDAL. The plugin links its three smartmet C++ library dependencies
(newbase, macgyver, gis) statically, so installing this RPM does not
pull any FMI runtime libraries onto the system.

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q -n %{SPECNAME}

%build
make %{_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT GDAL_PLUGIN_DIR=%{GDAL_PLUGIN_DIR}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0755,root,root,0755)
%{GDAL_PLUGIN_DIR}/gdal_%{DRIVERNAME}.so

%changelog
* Thu May 21 2026 Andris Pavenis <andris.pavenis@fmi.fi> - 26.5.21-1.fmi
- Build against installed smartmet-library-{newbase,macgyver,gis}-static
  packages instead of vendored git subtrees. The plugin RPM still has no
  smartmet-library shared-object runtime dependencies.

* Sun May 3 2026 Mika Heiskanen <mika.heiskanen@fmi.fi> - 26.5.3-1.fmi
- Vendored newbase, macgyver, gis as git subtrees and statically link them.
  The plugin RPM now has no smartmet-library runtime dependencies.

* Fri May 1 2026 Mika Heiskanen <mika.heiskanen@fmi.fi> - 26.5.1-1.fmi
- Initial version: read-only driver with subdatasets per (parameter, level)
