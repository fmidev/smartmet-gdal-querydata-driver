%define DIRNAME gdal-querydata-driver
%define SPECNAME smartmet-%{DIRNAME}
%define DRIVERNAME querydata
%define GDAL_PLUGIN_DIR %{_libdir}/gdalplugins

%global __brp_check_rpaths %{nil}

Summary: GDAL driver for FMI QueryData (.sqd) files
Name: %{SPECNAME}
Version: 26.5.1
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-gdal-querydata-driver
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: rpm-build
BuildRequires: gdal312-devel
BuildRequires: smartmet-library-newbase-devel >= 26.2.4
BuildRequires: smartmet-library-macgyver-devel >= 26.2.4
BuildRequires: smartmet-library-gis-devel >= 26.2.4

Requires: gdal312-libs
Requires: smartmet-library-newbase >= 26.2.4
Requires: smartmet-library-macgyver >= 26.2.4
Requires: smartmet-library-gis >= 26.2.4

Provides: %{SPECNAME}

%description
GDAL plugin that exposes FMI QueryData (.sqd) files as a GDAL raster
dataset, enabling read access via gdalinfo, gdal_translate, gdalwarp,
QGIS, rasterio, and any other GDAL consumer. Each (parameter, level)
combination is exposed as a subdataset, with one band per time step.

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q -n %{SPECNAME}

%build
make %{_smp_mflags}

%install
mkdir -p $RPM_BUILD_ROOT%{GDAL_PLUGIN_DIR}
install -p -m 755 gdal_%{DRIVERNAME}.so \
    $RPM_BUILD_ROOT%{GDAL_PLUGIN_DIR}/gdal_%{DRIVERNAME}.so

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0755,root,root,0755)
%{GDAL_PLUGIN_DIR}/gdal_%{DRIVERNAME}.so

%changelog
* Fri May 1 2026 Mika Heiskanen <mika.heiskanen@fmi.fi> - 26.5.1-1.fmi
- Initial version: read-only driver with subdatasets per (parameter, level)
