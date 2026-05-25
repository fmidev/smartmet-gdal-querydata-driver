%define DIRNAME gdal-querydata-driver
%define SPECNAME smartmet-%{DIRNAME}
%define DRIVERNAME querydata
%define GDAL_PLUGIN_DIR %{_libdir}/gdalplugins

%global __brp_check_rpaths %{nil}

Summary: GDAL driver for FMI QueryData (.sqd) files
Name: %{SPECNAME}
Version: 26.5.3
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-gdal-querydata-driver
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# The driver depends on the same Boost version as the vendored smartmet libraries
# in case of RHEL/RockyLinux 8 (note, that it must be downloaded from
# smartmet-open-ext repository, but not the EPEL one)
# On RHEL/RockyLinux 9 and newer, the stock boost package is new enough to be

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

%if 0%{?rhel} && ((%{rhel} == 8) || (%{rhel} == 9) || (0%{rhel} == 10))
%define GDAL gdal312
%define GEOS geos313
%define PROJ proj97
%else
%define GDAL gdal
%define GEOS geos
%define PROJ proj
%endif

# This package vendors the smartmet libraries (newbase, macgyver, gis) as git
# submodules under vendor/ and statically links them into the plugin .so. The
# source tarball ships the submodule working trees so rpmbuild does not need
# network access. The resulting RPM has *no* runtime dependency on
# smartmet-library-* packages — only on Fedora-stock libraries — which makes
# it installable on a fresh Fedora desktop alongside QGIS without pulling in
# the FMI build environment.

BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: rpm-build
BuildRequires: %{GDAL}-devel
BuildRequires: %{GEOS}-devel
BuildRequires: %{PROJ}-devel
BuildRequires: fmt-devel
BuildRequires: libicu-devel
BuildRequires: %{smartmet_boost}-devel
BuildRequires: double-conversion-devel
BuildRequires: ctpp2-devel
BuildRequires: libtiff-devel >= 4.1
BuildRequires: sqlite-devel >= 3.22.0
BuildRequires: libcurl-devel >= 7.68.0

# Fedora alternatives if the gdal312/geos313/proj97 side-by-side packages are
# unavailable: gdal-devel, geos-devel, proj-devel. Pick one set or the other.

# Runtime requirements are deliberately tight: this list mirrors the NEEDED
# entries actually present in the .so (verify with `objdump -p .so | grep
# NEEDED`). The vendored libraries' translation units that would have dragged
# in libpqxx, libsqlite3, etc., are excluded from VENDOR_SRCS in the Makefile.
# geos/proj/icu/double-conversion are reachable transitively via libgdal but
# the driver itself never references them, so --as-needed drops them.
Requires: %{GDAL}-libs
Requires: fmt-libs
Requires: %{smartmet_boost}-iostreams
Requires: %{smartmet_boost}-thread

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
GDAL. The driver bundles its three smartmet C++ library dependencies
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
* Sun May 3 2026 Mika Heiskanen <mika.heiskanen@fmi.fi> - 26.5.3-1.fmi
- Vendored newbase, macgyver, gis as git submodules and statically link them.
  The plugin RPM now has no smartmet-library runtime dependencies.

* Fri May 1 2026 Mika Heiskanen <mika.heiskanen@fmi.fi> - 26.5.1-1.fmi
- Initial version: read-only driver with subdatasets per (parameter, level)
