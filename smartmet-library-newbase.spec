%bcond_with tests
%define DIRNAME newbase
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-library-%{DIRNAME}

# https://fedoraproject.org/wiki/Changes/Broken_RPATH_will_fail_rpmbuild
%global __brp_check_rpaths %{nil}

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

%if 0%{?rhel} == 8
%global __python3 /usr/bin/python3.9
%global python3_sitearch %{_libdir}/python3.9/site-packages
%endif

%define smartmet_fmt_min 12.0.0
%define smartmet_fmt_max 13.0.0
%define smartmet_fmt fmt-libs >= %{smartmet_fmt_min}, fmt-libs < %{smartmet_fmt_max}
%define smartmet_fmt_devel fmt-devel >= %{smartmet_fmt_min}, fmt-devel < %{smartmet_fmt_max}

Summary: newbase library
Name: %{SPECNAME}
Version: 26.2.4
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-library-newbase
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: %{smartmet_boost}-devel
BuildRequires: bzip2-devel
BuildRequires: %{smartmet_fmt_devel}
BuildRequires: gcc-c++
BuildRequires: gdal312-devel
BuildRequires: geos313-devel
BuildRequires: make
BuildRequires: rpm-build
BuildRequires: smartmet-library-macgyver-devel >= 26.2.4
BuildRequires: smartmet-library-gis-devel >= 26.2.4
%if %{with tests}
BuildRequires: smartmet-library-regression
%endif
Requires: smartmet-library-macgyver >= 26.2.4
Requires: smartmet-library-gis >= 26.2.4
Requires: %{smartmet_boost}-iostreams
Requires: %{smartmet_boost}-regex
Requires: %{smartmet_boost}-system
Requires: %{smartmet_fmt}
Requires: gdal312-libs
Requires: geos313
#TestRequires: %{smartmet_boost}-devel
#TestRequires: bzip2-devel
#TestRequires: gcc-c++
#TestRequires: gdal312-devel
#TestRequires: gdal312-libs
#TestRequires: make
#TestRequires: postgresql15-libs
#TestRequires: smartmet-library-gis-devel >= 26.2.4
#TestRequires: smartmet-library-macgyver-devel >= 26.2.4
#TestRequires: smartmet-library-macgyver >= 26.2.4
#TestRequires: smartmet-library-gis >= 26.2.4
#TestRequires: smartmet-library-regression
#TestRequires: smartmet-timezones
#TestRequires: zlib-devel

%if 0%{?rhel} == 8
BuildRequires: python3.9
BuildRequires: python39-devel
BuildRequires: python39-setuptools
BuildRequires: python3.9-PyBindGen
%else
BuildRequires: python3
BuildRequires: python3-devel
BuildRequires: python3-setuptools
BuildRequires: python-PyBindGen
%endif

Provides: %{LIBNAME}
Obsoletes: libsmartmet-newbase < 16.12.19
Obsoletes: libsmartmet-newbase-debuginfo < 16.12.19

%description
FMI newbase library

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{SPECNAME}

%build
make %{_smp_mflags}
%if %{with tests}
make test %{_smp_mflags}
%endif

%install
%makeinstall
rm -f $RPM_BUILD_ROOT%{python3_sitearch}/newbase.so
strip $RPM_BUILD_ROOT%{_datadir}/smartmet/python/newbase.so

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
%files
%defattr(0775,root,root,0775)
%{_libdir}/libsmartmet-%{DIRNAME}.so

%package -n %{SPECNAME}-devel
Summary: FMI newbase development files
Provides: %{SPECNAME}-devel
Requires: %{SPECNAME} = %{version}-%{release}
Requires: smartmet-library-gis-devel >= 26.2.4
Requires: smartmet-library-macgyver-devel >= 26.2.4
Obsoletes: libsmartmet-newbase-devel < 16.12.19

%description -n %{SPECNAME}-devel
FMI newbase development files

%files -n %{SPECNAME}-devel
%defattr(0664,root,root,0775)
%{_includedir}/smartmet/%{DIRNAME}

####################################################################

%package -n %{SPECNAME}-static
Summary: FMI newbase static library
Provides: %{SPECNAME}-static

%description -n %{SPECNAME}-static
FMI newbase static library

%files -n %{SPECNAME}-static
%defattr(0664,root,root,0775)
%{_libdir}/libsmartmet-%{DIRNAME}.a

####################################################################

%package -n %{SPECNAME}-python
Summary: FMI newbase python bindings
Requires: %{SPECNAME} = %{version}-%{release}
Conflicts: python3-smartmet-library-newbase python36-smartmet-library-newbase
Requires: python3

%description -n %{SPECNAME}-python
FMI newbase python bindings

%files -n %{SPECNAME}-python
%{_datadir}/smartmet/python/newbase.so

%post -n %{SPECNAME}-python
for dir in /usr/lib64/python3*/site-packages; do ln -srfv %{_datadir}/smartmet/python/newbase.so $dir/newbase.so; done

%postun -n %{SPECNAME}-python
if [ $1 -eq 0 ]; then
for dir in /usr/lib64/python3*/site-packages; do if [ -L $dir/newbase.so ] ; then rm -fv $dir/newbase.so; fi; done
fi

%changelog
* Wed Feb  4 2026 Andris Pavēnis <andris.pavenis@fmi.fi> 26.2.4-1.fmi
- Update to proj-9.7, gdal-3.12, fmt-12

* Wed Jan  7 2026 Mika Heiskanen <mika.heiskanen@fmi.fi> - 26.1.7-1.fmi
- Improved gnomonic projection support

* Thu Mar 20 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.3.20-1.fmi
- Added GeoMagneticRIndex

* Tue Feb 18 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.2.18-1.fmi
- Update to gdal-3.10, geos-3.13 and proj-9.5

* Mon Dec 16 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.12.16-1.fmi
- Added ensemble parameters for mean and stdev cloudiness parameters (QDTOOLS-235)

* Tue Dec  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.12.3-1.fmi
- Improved error checking for multiqueryinfo

* Wed Nov  6 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.11.6-1.fmi
- Added parameter ParticleNumberConcentration (STU-25609)

* Tue Oct 15 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.10.15-1.fmi
- Removed landscape interpolation as obsolete

* Wed Sep 25 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.9.25-1.fmi
- STU-24852: Add new parameters for MU CAPE and CIN

* Tue Sep 17 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.9.17-1.fmi
- Added cloud ceiling height (feet) fractiles parameters (QDTOOLS-216)

* Wed Aug 21 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.21-3.fmi
- [python] Fix build and add test

* Wed Aug 21 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.21-2.fmi
- [python] Fix build and install for all installed Python3 versions

* Wed Aug 21 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.21-1.fmi
- Initial version of Python3 support integrated into smartmet-library-newbase build instead of separate project

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-1.fmi
- Update to gdal-3.8, geos-3.12, proj-94 and fmt-11

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-1.fmi
- Replace many boost library types with C++ standard library ones

* Fri May 17 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.5.17-2.fmi
- Added parameter 'WSI' for WIGOS station identifiers

* Fri May 17 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.5.17-1.fmi
- Added -lsmartmet-macgyver
* Thu May 16 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.16-1.fmi
- Clean up boost date-time uses
* Mon May  6 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.6-1.fmi
- Use Date library (https://github.com/HowardHinnant/date) instead of boost date_time

* Wed Apr 24 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.4.24-1.fmi
- Fixed creation of Mercator projections
* Wed Apr 17 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.4.17-1.fmi
- Improved logic for parallel tasks
* Mon Mar 11 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.3.11-1.fmi
- Use NFmiGdalArea instead of NFmiMercatorArea if lat_ts or lon_0 are not zero
* Thu Mar  7 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.3.7-1.fmi
- Enable include and #define for NFmiSettings to make qdtext configuration simpler
* Fri Feb 23 2024 Mika Heiskanen <mheiskan@rhel8.dev.fmi.fi> - 24.2.23-1.fmi
- Do not use boost::noncopyable
- Added safety check to projection parser
* Tue Jan 30 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.1.30-1.fmi
- NFmiInfoAreaMask added multi-source data support (relates multiple synop and flash data usage).
- NFmiInfoAreaMask added support for model-climatology-data (era5 type data).
- NFmiInfoAreaMask added better observation station distance checking ability, new CheckPossibleObservationDistance method.
- NFmiInfoAreaMaskVertFunc added DoPeekZFunction and CalculateUsedPeekZPressureLevel methods.
- NFmiInfoAreaMaskVertFunc simplified IterateLevelsFromGroundUpward method.
- NFmiInfoAreaMaskProbFunc added support for observation data for probability calculations.
- Using NFmiCalculationParams UsedLatlon method to get wanted calculation point.
- Added support for ModAvg, ModMin and ModMax integration functions.
- Fixed NFmiInfoAreaMaskVertFunc::SearchLevels to handle also first level, previously skipped the 1st level.
- For Simple condition calculation added support for -> comparison operator which is so called continuous-equal
- Fixed level data height and pressure interpolation codes to work better in different data cases
- Removed usage of assignment operator (operator=) in NFmiAreaMask class hierarchy. Removed operator= methods from the child classes
- Now using NFmiCalculationParams's UsedLatlon method in calculations
- NFmiMacroParamValue class now has support for time-serial based smarttools calculations.
- NFmiCalculationParams has support for:
-   special calculations (cross-section and time-serial).
-   CalculationPoint type macroParam calculations (new member data itsActualCalculationPoint and new UsedLatlon and SetModifiedLatlon methods).
-   Support for multi-data cases, like synop data has three queryData (Finland, Europe and World) with itsCurrentMultiInfoData data member.
-   Support for observation max distance limiting (itsObservationRadiusInKm data member)
- Added new methods relating to fast bulk get methods (SetValues, SetLevelFromVec, DoSubParamConversions)
- Also simplified some of those fast bulk methods by using GetValues and GetPartialValues methods.
- Added new GetFixedPressureLevelIndex which returns decimal value of level index of given pressure, which can be used in interpolations in vertical dimension.
- Added ElapsedTimeFromLoadInSeconds virtual method that is not used in newbase classes, but in child classes in smarttools library. This is used detect if certain data is newly loaded or not.
- Fixed HeightValue and PressureValue methods to handle correctly the cross-section and time-serial case and always use time interpolation with them.
- Added FunctionDataTimeOffsetInHours and CheckPossibleObservationDistance implementations.
- NFmiAreaMaskImpl::Value method now uses NFmiCalculationParams's UsedLatlon method for actual wanted calculation point (there are normal calculation grid-points and then there are CalculationPoint type observation points that are used in different cases).
- Added several new Functiontype enum values for smarttools language.
- Added better support for limiting used observation station in used calculation grid by introducing CheckPossibleObservationDistance method.
- Moved some generally used DoShallowCopy functions in here to centralize them.
- Added new data-modifier class's includes to this combined includes file.
- Added couple new dataModifier classes to be used with smarttools language. These are used to calculate modular avg and min/max values. Modular parameters, lik
e wind direction, must be handled a bit differently then normal continuous parameters.
- Added couple new values to NFmiInfoData::Type enum, relating to SmartMet-workstation view layers.
- Changed FmiMaskOperation enum: 1. removed numeric values from all values except from the first one, 2. Added new kFmiMaskContinuouslEqual for smarttools calculations.
- New method SetValues, which is used to set e.g. one time-param-level grid values, using vector and startIndex and related step count
- New method IsReadOnly tells if queryData has been read with memory-mapped style (read-only) or into memory, which means that data can be modified
- Fixes to QueryInfoParamStateRestorer class
- Added new QueryInfoTotalStateRestorer class (restores all the dimensions, not just parameter)
- Added couple MetaWindParamUsage constructors
- Added IsLightningTypeData function
- Fixed GetUsedTimeIfModelClimatologyData function to handle also static queryData (like topography which have only one time)
- Fixed FindTimeIndicesForGivenTimeRange function
- Fixed theMaxWantedLocations parameter related crash bug in NFmiLocationBag::NearestLocations method.
- Added new NFmiNanoSecondTimer helper class for more precise time measuring
- Changed NFmiTimeDescriptor::GetIntersection method to const

* Tue Nov 21 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.11.21-2.fmi
- Repackaged since MappedFile ABI changed

* Tue Nov 21 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.11.21-1.fmi
- New parameters: IcingMassRate and IcingGrowthRate (BRAINSTORM-2788)

* Tue Nov 14 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.11.14-1.fmi
- Added parameters WildFireWarning, CloudWaterDryAri, CloudIceDryAir, WildFireIndex
- Replaced boost date objects with macgyver DateTime and LocalDateTime

* Wed Oct 11 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.10.11-1.fmi
- Use Fmi::FileSystem for file compression support

* Tue Oct  3 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.10.3-1.fmi
- Add LZMA and ZSTD compressuion support for query data

* Wed Aug 30 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.8.30-1.fmi
- Added SILAM parameters for smoke (BRAINSTORM-2649)

* Tue Aug 29 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.8.29-1.fmi
- Update new EPS updraft helicity parameters (STU-22037)

* Mon Aug 21 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.8.21-1.fmi
- Use Fmi::MappedFile and remove Advise methods

* Mon Jul 31 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.7.31-1.fmi
- Add four new parameters from ICON model
- snow temperature (SnowTemperature)
- mixing layer CAPE (CAPEMixingLayer)
- fraction of lake (fresh) water (FreshWaterFraction)
- direct short-wave radiation (RadiationSWDirect)

* Fri Jul 28 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.28-1.fmi
- Repackage due to bulk ABI changes in macgyver/newbase/spine

* Thu Jul 27 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.27-1.fmi
- Fix compiler warnings

* Mon Jul 10 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.10-1.fmi
- Use postgresql 15, gdal 3.5, geos 3.11 and proj-9.0

* Thu Jun 15 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.6.15-1.fmi
- Fixed NFmiLocationBag::CoordinateMatrix to return all station coordinates

* Thu Feb  9 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.9-1.fmi
- New probability parameters for AtmosphericIceGrowth (STU-20797)

* Mon Nov 14 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.14-1.fmi
- New parameter LLFCloudTopFL

* Mon Aug 29 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.29-1.fmi
- New parameter AtmosphericIceGrowth

* Wed Aug 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.24-1.fmi
- Fixed resolution calculations in index mask tools

* Thu Jun 16 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.16-1.fmi
- Add support of HEL9, upgrade to libpqxx-7.7.0 (rhel8+) and fmt-8.1.1

* Fri Jun 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.10-2.fmi
- Added NFmiAreaTools::CreateLegacyYKJArea with option for setting degrees/meters for the input coordinates

* Fri Jun 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.10-1.fmi
- Fixed legacy YKJ area detection

* Thu Jun  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.9-2.fmi
- Fixed NFmiAreaTools::CreateLegacyRotatedLatLonArea to take rotated coordinates and handle lat1 > lat2

* Thu Jun  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.9-1.fmi
- Added NFmiAreaTools::CreateLegacyLatLonArea with option to set Pacific view

* Wed Jun  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.8-2.fmi
- Allow legacy sphere to be specified either with +ellps=sphere, plain +R, plain +a or with +a == +b

* Wed Jun  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.8-1.fmi
- Allow both legacy sphere and WGS84 spatial references for legacy NFmiXXXArea classes

* Mon Jun  6 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.6-2.fmi
- Fixed NFmiAreaTools::CreateLegacyYKJArea to force meters as units

* Fri Jun  3 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.3-1.fmi
- Added Fmi::LegacyMode
- Added NFmiArea::CreateFromCenter
- Added NFmiArea::CreateFromCorners
- Added NFmiArea::CreateFromReverseCorners
- Added NFmiArea::CreateFromCornerAndSize
- Added NFmiArea::CreateNewAreaByWorldRect which optionally allows enlarging the area
- Added new parameters for Himan: MaxProbabilityThunderstrom24h, WindGustDuration24h, MaxWindGust24h, MaxSevereThunderstormPotential24h

* Thu Jun  2 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.2-2.fmi
- Added NFmiArea::SetGridSize for ABI compatibility

* Thu Jun  2 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.2-1.fmi
- Fixed world wrap issue

* Wed Jun  1 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.1-2.fmi
- Added NFmiArea::SimpleWKT for compatibility with the WGS84 branch

* Tue May 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.24-1.fmi
- Changed several NFmiArea methods not to be inline for ABI compatibility with the WGS84 branch

* Fri May 20 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.20-1.fmi
- NFmiFastQueryInfo::LatLon no longer returns a const reference due to ABI compability issues
- Removed NFmiFastQueryInfo::LatLonFast as obsolete

* Thu May 19 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.19-1.fmi
- Added NFmiArea::CreateFromBBox for ABI compatibility with the WGS84 branch

* Wed May 18 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.18-1.fmi
- Added convenience methods for creating legacy projections via NFmiAreaTools instead of direct constructors

* Thu Apr 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.4.28-1.fmi
- New Enfuser parameters: LungDepositedSurfaceArea (LDSA) and BlackCarbonConcentration
- New power grid parameters: PowerOutagesByCounty and PowerOutagesByMunicipality

* Tue Mar 22 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.22-1.fmi
- New parameters: SoaringFlightIndex, ThermalBirdMigrationIndex

* Fri Jan 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.1.21-1.fmi
- Repackage due to upgrade of packages from PGDG repo: gdal-3.4, geos-3.10, proj-8.2

* Tue Dec  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.12.7-1.fmi
- Update to postgresql 13 and gdal 3.3

* Wed Dec  1 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.12.1-1.fmi
- New parameters: PowerOutput, PowerOutputSum

* Mon Nov 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.15-1.fmi
- Added EDR (eddy dissipation rate) parameter

* Wed Oct 13 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.13-1.fmi
- Print station coordinates with full accuracy

* Wed Sep 22 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.22-1.fmi
- Added parameter RadiationDownLWClearSky

* Mon Sep 20 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.20-1.fmi
- Added NFmiAreaMaskHelperStructures files

* Tue Sep 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.14-1.fmi
- Added rail probability parameter

* Wed Jun 16 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.16-1.fmi
- Use Fmi::Exception

* Mon Jun  7 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.7-1.fmi
- Fixed self assignment issues

* Thu May  6 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.6-1.fmi
- Fixed azimuthal projections to handle negative central/true latitudes correctly

* Thu Apr 22 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.4.22-1.fmi
- Added NWCSAF parameters

* Sat Mar 20 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.20-1.fmi
- Faster and disk friendlier grid interpolations

* Fri Mar 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.19-1.fmi
- Fixed NFmiEquidistantArea to initialize the spatial reference for all constructor calls

* Tue Mar  9 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.9-1.fmi
- Added move assignment and constructor for NFmiDataMatrix

* Tue Mar  2 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.2-1.fmi
- Added parameter HardPackedSnowIndex (FIN: Polanne)

* Mon Mar  1 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.1-1.fmi
- Faster construction of newbase times from Boost.Date_time ones
- Added numbers for metaparameters used by the SmartMet Server

* Fri Feb 26 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.26-1.fmi
- Added enumerations for metaparameters which are usually calculated on the fly

* Mon Feb 22 2021 Anssi Reponen <anssi.reponen@fmi.fi> - 21.2.22-1.fmi
- Added new PrecipitationAccumulation parameter (QDTOOLS-92)

* Sat Feb 20 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.20-1.fmi
- Fixed NFmiRotatedLatLon PROJ.4 string after a copy paste error

* Thu Feb 18 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.18-1.fmi
- Added NFmiArea::SpatialReference()
- Added NFmiFastQueryInfo::SpatialReference()

* Wed Feb 17 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.17-1.fmi
- Fixed NFmiRotatedLatLon WKT

* Tue Feb 16 2021 Andris Pavēnis <andris.pavenis@fmi.fi> - 21.2.16-1.fmi
- Fmi::NFmiQueryInfo: use std::unique_ptr instead of raw pointers

* Mon Feb 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.15-2.fmi
- Updated NFmiSmoother to use Fmi::CoordinateMatrix

* Mon Feb 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.15-1.fmi
- Merged more API changes from WGS84 branch: interpolation methods return a datamatrix

* Wed Feb 10 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.10-2.fmi
- Fixed GIS-library dependencies

* Wed Feb 10 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.10-1.fmi
- Fixed NFmiLatLonArea WKT to be equirectangular instead of longlat
- Added NFmiArea::CoordinateMatrix
- Added NFmiFastQueryInfo::WorldXY
- Added NFmiFastQueryInfo::NeedsGlobeWrap
- Added linkage to gis-library

* Mon Feb  8 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.8-1.fmi
- Added CoordinateMatrix access

* Fri Jan 22 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.22-1.fmi
- Added probability and fractile parameters for road forecasts

* Tue Jan 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.19-1.fmi
- Added ProbabilityOfForestFireIndexLevel[1-3]

* Thu Jan 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.14-1.fmi
- Repackaged smartmet to resolve debuginfo issues

* Tue Jan  5 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.5-1.fmi
- Upgrade to fmt 7.1.3

* Mon Jan  4 2021 Andris Pavenis <andris.pavenis@fmi.fi> - 21.1.4-1.fmi
- Rebuild due to PGDG repository change: gdal-3.2 uses geos-3.9 instead of geos-3.8

* Tue Dec 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.15-1.fmi
- Upgrade to pgdg12

* Fri Dec 11 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.12.11-1.fmi
- Build and test updates
- Uses gdal-3.2 for RHEL8

* Thu Dec 10 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.12.10-1.fmi
- Rebuild due to makefile.inc changes

* Mon Nov 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.30-1.fmi
- Added maximum thread count option for grid projections

* Wed Oct 28 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.28-1.fmi
- Rebuild due to fmt upgrade

* Fri Oct 16 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.16-1.fmi
- Prevent crash in NFmiTransformList
- Fixed memory leak in NFmiQueryInfo::TimePeriodValue

* Fri Oct  9 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.9-1.fmi
- Use std::array instead of std::vector in interpolation for speed

* Wed Oct  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.7-1.fmi
- Fixed version numbering

* Mon Oct  5 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.5-1.fmi
- Build update (use makefile.inc from smartmet-library-macgyver)

* Fri Oct  2 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.2-1.fmi
- New aviation parameters

* Tue Sep 29 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.29-1.fmi
- Added NFmiWindFix for recalculating wind components from wind speed and direction

* Fri Sep 25 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.25-1.fmi
- Added N200 sea level min, max, mean, dev, probability and fractile parameters

* Fri Sep 11 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.11-1.fmi
- Removed obsolete FMI_DLL export declarations

* Mon Sep  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.7-1.fmi
- Fixed stack smashing issue when reading Lambert Conformal Conic querydata

* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2

* Tue Jun 16 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.16-1.fmi
- Added parameter SeaLevelN2000 where N2000 is the vertical reference system

* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgrade to Boost 1.69

* Tue Apr 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.14-1.fmi
- Add three probability parameters for POT

* Thu Feb 13 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.13-1.fmi
- Use nearest point interpolation for WindVector parameter

* Thu Feb  6 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.6-1.fmi
- Restored ability to read compressed files

* Wed Jan 29 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.29-1.fmi
- New parameter: nearby weather number
- New parameter: snow accumulation 24h fractiles 0,10,25,50,75,90,100
- New parameter: snow accumulation 120h fractiles 0,10,25,50,75,90,100

* Wed Dec  4 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.4-1.fmi
- Use -fno-omit-frame-pointer for a better profiling and debugging experience
- Fixed dependency to be on gdal-libs instead of gdal

* Thu Nov 21 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.21-1.fmi
- Revert old interpolation behaviour of qdpoint -i options
- Fixed eternal loop in NFmiDataMatrixUtils

* Wed Nov 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.20-1.fmi
- Made some NFmiQueryDataUtil methods public to handle time-slice data combinations
- Replaced checkedVector by std::vector
- Moved NFmiDataMatrix interpolation and print methods to SmartMet Workstation library

* Tue Nov 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.19-2.fmi
- Added fractile parameters for LowAndMiddleClouds

* Tue Nov 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.19-1.fmi
- Fixed NFmiStreamQueryData to produce version 7 querydata by default

* Thu Nov 14 2019 Anssi Reponen <anssi.reponen@fmi.fi> - 19.11.14-1.fmi
- Two new functions to NFmiSvgTools

* Fri Nov  1 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.1-1.fmi
- Added PotentialWindSpeed parameter - wind speed reduced to 10 m height

* Thu Oct 31 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.31-1.fmi
- Merged changed from SmartMet Editor branch

* Thu Sep 26 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.26-1.fmi
- Removed global FmiInfoVersion variable
- Added constant global DefaultInfoVersion = 7
- Removed most of support for info versions below 6
- Fixed projection parser memory overflow discovered with ASAN
- Removed NFmiMetBox as obsolete
- Removed constructor/destructor call counters as obsolete debugging variables
- Use memcpy instead of reinterpret_cast to avoid ASAN warnings on alignment issues
- Removed wrong dependency on postgis

* Mon Aug 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.12-1.fmi
- Fixed time interpolation to check the time gap to both times

* Mon Jul 29 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.7.29-2.fmi
- Matrix fetcing Values-methods now throw if the requested time is outside the available time range

* Mon Jul 29 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.7.29-1.fmi
- Add probability parameters for Convective Severity Index

* Tue May 14 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.14-1.fmi
- Add param for 120h precipitation fractile

* Mon May  6 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.6-1.fmi
- Add parameter for 'probability of lightning with area and time aggregation' (STU-10114)

* Tue Apr 23 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.4.23-1.fmi
- Added param SoundingMaximumEBS (STU-10042)

* Thu Apr 11 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.4.11-1.fmi
- Add param ConvectiveSeverityIndex (STU-10042)
- Add param FogProbabilityLimit1 (STU-9623)
- Add fractile parameters for monthly precipitation (STU-9410)

* Fri Apr  5 2019 Santeri Oksman <santeri.oksman@fmi.fi> - 19.4.5-1.fmi
- Added parameters VolumetricSoilWaterLayer28 and VolumetricSoilWaterLayerAnomaly28(SOL-7344)

* Thu Mar 21 2019 Pertti Kinnia <pertti.kinnia@fmi.fi> - 19.3.21-1.fmi
- Fixed bug in creating GDalArea

* Thu Mar 14 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.14-1.fmi
- Added more fractile parameters for Precipitation24, TotalColumnWaterVapour and TotalPrecipitation

* Wed Mar  6 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.6-1.fmi
- Added ERA5 fractile parameters for MaximumTemperature, MinimumTemperature and Precipitation24h

* Fri Mar  1 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.1-1.fmi
- Added parameter RadarPrecipitation1h

* Wed Feb 27 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.27-1.fmi
- Fixed LCC WorldXyToLatLon longitude calculation to use radians in all intermediate stages

* Thu Feb 21 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.21-1.fmi
- New parameters: EFIWindPrecipitation, MonthlyMaximumSnowDepth, SOTWWindGust, SOTPrecipitation (Shift Of Tails)

* Thu Feb 14 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.14-1.fmi
- Added Lambert Conformal Conic projection

* Wed Feb 13 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.13-1.fmi
- New fractile parameters for ERA5 data

* Fri Feb  8 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.8-1.fmi
- Added WebMercator projection (EPSG:3857)

* Wed Feb  6 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.6-1.fmi
- Added secondary parameters for cloud cover (total, low, middle, high)

* Mon Jan 28 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.1.28-1.fmi
- Added parameters for snow load fractiles 0, 10, 25, 50, 75, 90 and 100

* Fri Jan 18 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.1.18-1.fmi
- Added parameter FreezingDegreeDays

* Tue Dec  4 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.12.4-1.fmi
- Added GrassFireIndex and ForestFireIndex
- Made some protected NFmiFastQueryInfo methods public to enable fast multithreaded writing

* Mon Nov 26 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.26-1.fmi
- Add new parameter 'Snow Drift Index'

* Tue Oct 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.23-1.fmi
- Added parameter GroundWaterLevel for measuring the absolute difference from the long term average height
- Added parameter GroundWaterLevelIndex for classifying the relative difference from the long term average height

* Sat Sep 29 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.29-1.fmi
- Upgraded fmt

* Mon Sep 24 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.24-1.fmi
- Added MetCoop global radiation fractile parameters

* Tue Sep 11 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.11-1.fmi
- Fixed NFmiAreaFactory handling of GDAL area definitions

* Mon Sep 10 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.10-1.fmi
- Fixed NFmiGdalArea AreaStr to return the datum given to it

* Thu Sep  6 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.9.6-1.fmi
- Added missing parameter check preventing kFmiPrecipitation1h being used to produce 'weatherandcloudiness' object

* Tue Aug 28 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.28-1.fmi
- Added boost_iostreams linkage

* Thu Aug 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.23-2.fmi
- Compile NFmiEnumConverter with -O0 since compiling takes a lot of memory

* Thu Aug 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.23-1.fmi
- Link boost date_time to the library

* Wed Aug 22 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.22-1.fmi
- Fixed to compile on Windows

* Mon Aug  6 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.6-2.fmi
- New parameter CAPEShear

* Mon Aug  6 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.6-1.fmi
- Fixed NFmiVoidPtrList destructor not to call derived class Clear()
- New parameter InstantaneousWindGust

* Thu Aug  2 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.2-1.fmi
- Fixed constructors/destructors not to call virtual methods

* Wed Aug  1 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.1-1.fmi
- Use C++11 for-loops instead of BOOST_FOREACH

* Mon Jul 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.7.23-1.fmi
- Fixed NFmiQueryInfo::ResetText
- Fixed potential segfault in NFmiQueryInfo::Size
- Fixed NFmiString self assignment

* Wed Jun 13 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.6.13-1.fmi
- Removed -DBOOST, -DFMI_COMPRESSION and -DBOOST_IOSTREAMS_NO_LIB options as obsolete

* Thu Jun  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.6.7-1.fmi
- New parameter: WaterTemperature

* Wed Jun  6 2018 Mikko Parviainen <mikko.parviainen@fmi.fi> - 18.6.6-1.fmi
- New parameter: RadarPrecipitation1d

* Tue Jun  5 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.6.5-1.fmi
- Three new parameters: ProbabilityOfGustLimit1Aggregation1, ProbabilityOfPrecLimit1Aggregation1, ProbabilityOfHailLimit1Aggregation1

* Thu May 24 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.24-1.fmi
- Optimized rotated latlon conversions for speed

* Wed May 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.23-1.fmi
- Removed incorrect virtual specification from NFmiFastQueryInfo::PressureValues

* Mon May 21 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.21-1.fmi
- Added regridding methods with relative_uv parameter to control whether U/V adjustment needs to be applied

* Wed May  2 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.2-1.fmi
- Optimized NFmiEnumConverter for speed

* Thu Apr 19 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.4.19-1.fmi
- NFmiGdalArea::AreaStr(): Return bbox at the end of projection string
- AreaFactory::Create(): Fixed parsing for NFmiGdalArea projection string (BS-1140)

* Mon Apr 16 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.4.16-1.fmi
- GetValueAtHeight(): using ModLinear interpolation for wind direction
- PressureValue(P): fixed factor for ModLinear interpolation

* Sat Apr  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.7-1.fmi
- Upgrade to boost 1.66

* Wed Apr  4 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.4-1.fmi
- Add parameter for average mixing ratio in the lowest 500 meters

* Mon Mar 26 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.26-1.fmi
- Store memory mapped file name into NFmiRawData to ease core dump analysis

* Fri Mar 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.23-1.fmi
- New parameter: GrowingDegreeDays

* Sat Mar 10 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.10-1.fmi
- Avoid ostringstream global locale locks by using fmt library for WKT formatting

* Wed Mar  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.7-1.fmi
- Added new parameter kFmiFeelsLike needed for press production

* Thu Feb  8 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.8-1.fmi
- Added explicit postgis dependency to avoid pgdg dependency problems

* Mon Jan 22 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.1.22-1.fmi
- Improved pressure level interpolations (SOL-6124)

* Sat Jan 13 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.1.13-1.fmi
- Fixed Lambert azimuthal equal area WKT

* Wed Jan  3 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.1.3-1.fmi
- Do not use NetCDF Latitude and Longitude parameter numbers when converting name to integer

* Fri Dec 22 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.12.22-1.fmi
- New parameter PrecipitationForm4
- New parameter DirectNormalIrradianceAccumulation

* Tue Dec 19 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.12.19-1.fmi
- New parameter names for weather symbols
- New parameter name for solar radiation

* Mon Dec 11 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.12.11-1.fmi
- Fixed LAEA CalcDelta method to divide the input by earth radius

* Wed Nov 29 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.29-1.fmi
- Fixed Lambert equal area world coordinates to be scaled by earth radius

* Tue Nov 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.28-1.fmi
- Fixed undefined behavious in assignment

* Mon Nov 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.27-1.fmi
- Fixed potential memory leaks and nullptr dereferences

* Thu Nov 23 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.23-1.fmi
- Improved error messages when trying to create empty output querydata

* Thu Nov 16 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.16-1.fmi
- Added two 24h precipitation probability parameters (which are used even now, but not 'registered')
- Added short wave radiation accumulation parameter (for Harmonie / open data)

* Mon Nov 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.13-1.fmi
- Fixed cached interpolations to work with the nearest point method

* Sun Oct 29 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.10.29-1.fmi
- Optimized data copying from querydata to another

* Tue Oct 17 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.10.17-1.fmi
- Added new parameters for olive pollen

* Thu Oct  5 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.10.5-1.fmi
- New probability type parameters

* Wed Sep 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.27-1.fmi
- Add probability parameters for MUCAPE, MLCAPE and MUCAPE -10 .. -40

* Thu Sep 14 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.14-1.fmi
- NFmiGdalArea now prepends AreaStr with WGS84 so that NFmiAreaFactory can be used on the output

* Wed Sep 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.13-2.fmi
- Added all CF standard names (http://cfconventions.org/Data/cf-standard-names/45/build/cf-standard-name-table.html)

* Wed Sep 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.13-1.fmi
- New sounding index parameters for the last equilibrium level

* Mon Sep 11 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.11-1.fmi
- Fixed NFmiRotatedLatLonArea WKT

* Mon Aug 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.8.28-1.fmi
- Upgrade to boost 1.65
- Ignore use of NFmiGdalArea in NFmiArea if not UNIX

* Tue Aug 1 2017 Ville Ilkka <ville.ilkka@fmi.fi> - 17.8.1-1.fmi
- Added a new parameter for 5cm snow accumulation days

* Tue Jul 25 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.7.25-1.fmi
- Fixed NFmiBox to use delete[]
- Removed NearestMetTime method whose implementation caused infinite recursion

* Thu Jun  1 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.6.1-1.fmi
- Add parameters for sea level fractiles, mean and deviation

* Fri May 26 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.5.26-1.fmi
- Added sea level probability parameters

* Fri Apr 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.28-1.fmi
- Temporary fix to HashValue methods. Will change NFmiArea::HashValue to be virtual later on.

* Tue Apr  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.4-1.fmi
- Added support for interpolating within given time window when interpolating to given pressure or height

* Mon Apr  3 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.3-1.fmi
- New methods for dealing with moving sounding data
- Bug fix to FindTimeIndicesForGivenTimeRange

* Tue Mar 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.28-1.fmi
- Modified info masks to enable operating on climatology data at any given time in history

* Wed Mar 22 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.22-1.fmi
- Added Opera ODIM parameter ZDR with name DifferentialReflectivity
- Added Opera ODIM parameter KDP with name SpecificDifferentialPhase
- Added Opera ODIM parameter PHIDP with name DifferentialPhase
- Added Opera ODIM parameter SQI with name SignalQualityIndex
- Added Opera ODIM parameter RHORV with name ReflectivityCorrelation

* Thu Mar  9 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.9-1.fmi
- Added sounding tools for SmartMet Editor

* Mon Mar  6 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.6-1.fmi
- Added CMakeList.txt created for the Windows build
- Minor fixes needed for by the Windows build

* Wed Mar  1 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.1-1.fmi
- ProbabilityOfColdLimit5 is the probability of temperature being below -35C

* Tue Feb 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.28-1.fmi
- Renamed AvailableEnsembleMemberPercentage to AvailableEnsembleMemberCount
- Renamed HoarFrostTemperature to FrostPoint

* Wed Feb 15 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.15-1.fmi
- Added static packaging

* Mon Feb 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.13-1.fmi
- Fixed a race condition in NFmiQueryData::LatLonCache()

* Fri Feb 10 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.10-1.fmi
- Backported numerous changes from the SmartMet Editor branch

* Tue Feb  7 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.7-1.fmi
- Fixed querydata copying to copy memory mapped data from the correct address

* Thu Feb  2 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.2-1.fmi
- Added possibility to memory map writeable querydata

* Wed Feb  1 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.1-1.fmi
- Fixed two SnowDepth fractile parameter names
- Added parameter for post-processed cloud top height (ft)
- Added parameter for average wind speed (ms)
- Added parameter for potential precipitation type (code)
- Added parameter for humidity relative to ice (%)
- Added parameter for hoar frost temperature (C)
- Added parameter for available ensemble members (%)
- Added parameter for post-processed wind gust speed
- Added four parameters for probability of wave height with different limits (%)

* Thu Jan 26 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.26-1.fmi
- Added methods for calculating a hash value for the grid defined in querydata

* Tue Jan 17 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.17-1.fmi
- New parameters for air pollutant concentrations

* Tue Jan 10 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.10-1.fmi
- New parameter IcingRate [g/h] for wind mill icing forecasts
- New parameter IceMass [kg] for wind mill icing forecasts

* Mon Dec 19 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.12.19-1.fmi
- Added Visibility2 (post processed)
- Added fractile parameters

* Fri Nov 18 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.18-1.fmi
- Added new Copernicus parameters: kFmiFrostLayerTop and kFmiFrostLayerBottom

* Tue Nov 15 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.15-2.fmi
- Fixed NFmiGdalArea to handle an unset authority name or code properly

* Tue Nov 15 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.15-1.fmi
- New parameter: CloudBase2, post processed cloud base height

* Tue Sep 20 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.9.20-1.fmi
- Fixed GdalArea to handle missing authority codes and names

* Fri Sep  9 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.9.9-2.fmi
- Bug fix to preserving header strings

* Fri Sep  9 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.9.9-1.fmi
- Fixed preparation code for time interpolation to preserve querydata header strings

* Wed Aug 31 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.31-1.fmi
- Added safety checks against bad time resolutions in querydata headers

* Thu Jun 16 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.16-1.fmi
- Fixed a bug in handling NFmiTimeBag time resolutions of multiple days

* Wed Jun 15 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.15-1.fmi
- Updated logic for selecting a symbol for cloudy weather (SOL-4077)
- Updated logic for selecting a symbol for heavy thunder (SOL-4077)

* Mon Jun 13 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.13-1.fmi
- Switched to cartesian nearest location searches, the approximation did not satisfy the triangle inequality

* Fri Jun  3 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.3-1.fmi
- Added Clim4Energy parameters (SOL-3596)
- SkinReservoirContent, HighVegetationCover, HighVegetationType, LowVegetationType
- SoilTemperatureLevel1, SoilTemperatureLevel2, SoilTemperatureLevel3, SoilTemperatureLevel4
- VolumetricSoilWaterLayer1, VolumetricSoilWaterLayer2, VolumetricSoilWaterLayer3, VolumetricSoilWaterLayer4

* Thu May 26 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.5.26-1.fmi
- Added SILAM parameter AreaOfRisk

* Tue May 24 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.5.24-1.fmi
- Optimized NFmiLocationBag::IsInside for speed

* Thu May 12 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.5.12-1.fmi
- Much faster nearest location searches for point data

* Wed May  4 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.5.4-1.fmi
- Added sounding parameters previously known only by qdsoundingindex

* Fri Apr  8 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.4.8-1.fmi
- Fixed a memory leak in NFmiNearTree::Clear

* Thu Mar 17 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.3.17-1.fmi
- New parameter snow density (mass per unit of volume), kg/m3

* Thu Feb  4 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.2.4-1.fmi
- Added gridded landscaping functionality

* Wed Feb  3 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.2.3-1.fmi
- New parameter: PotentialPrecipitationForm

* Sun Jan 17 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.17-1.fmi
- Merge from Windows SmartMet editor development:
- Added a producer for BUFR soundings
- Bug fix to subvalue extraction of weather symbols
- Bug fix to precipitation form extraction
- Bug fix to file extension extraction when there is no suffix in the file
- Bug fix to extracting given heights using masks
- Bug fix to pressure level extractions

* Thu Nov 26 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.26-1.fmi
- Added a swap method for NFmiQueryData to speed up qdpress

* Mon Nov 23 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.23-1.fmi
- New parameter: BoundaryLayerTurbulence
- Optimized loop order in numerous NFmiQueryDataUtil utility functions
- Limit the number of threads in area interpolations to 8

* Wed Nov 11 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.11-1.fmi
- Bug fix to SetPrecipitationFormValue

* Tue Nov 10 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.10-2.fmi
- Added normal assignment operators for NFmiTime and NFmiMetTime

* Tue Nov 10 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.10-1.fmi
- Optimized AreaStr methods for speed

* Mon Nov  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.9-1.fmi
- New parameters: NO-, O3-, PM10- and PM25Contents

* Wed Nov  4 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.4-1.fmi
- Added boost local_date_time based constructors for NFmiTime and NFmiMetTime
- Optimized time interpolations to avoid constructing unnecessary time objects from the wall clock

* Mon Oct 26 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.10.26-1.fmi
- Compile with -g to enable studying cores

* Tue Oct 13 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.10.13-1.fmi
- Limit lapse rate corrections during inversion to -300...+150 meters to avoid problems at Finnish hills

* Mon Sep 28 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.28-2.fmi
- Limit lapse rate corrections at sea during inversion to prevent Norwegien fjords from freezing

* Mon Sep 28 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.28-1.fmi
- Added a bbox based constructor to NFmiGdalArea

* Fri Sep 25 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.25-1.fmi
- Swapped lapse rate height limits since default lapse rate is now negative

* Thu Sep 24 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.24-1.fmi
- Default lapse rate is now -6.5 degrees to match the sign in ECMWF data

* Thu Sep 17 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.17-1.fmi
- Added ProbabilityThunderstorm2
- Added 6 precipitation form probability parameters: drizzle, rain (liquid), sleet, snow, freezing drizzle and freezing rain

* Tue Sep 15 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.15-1.fmi
- Added height limits for lapse rate corrections

* Thu Sep 10 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.9.10-1.fmi
- Added reading lapse rate parameter from querydata into landscape corrections

* Fri Sep  4 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.9.4-1.fmi
- Fixed names of various deposition parameters

* Wed Aug 26 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.26-1.fmi
- Optimized parameter changes via FmiParameter name for speed

* Mon Jul 27 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.7.27-1.fmi
- Two new aviation parameters

* Mon Jun 29 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.29-1.fmi
- Fixed NFmiGridBase::InInside

* Tue Jun 23 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.23-1.fmi
- Dropped reference humidity in feels like formulas to 50%

* Mon Jun 22 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.22-1.fmi
- Use 60% as the reference humidity for SummerSimmer index, deserts are not good references points in Finland

* Wed Jun 17 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.17-1.fmi
- Added parameters for de-icing. Class 0 = no de-icing needed, 1 = frost on plain, 2 = weak/moderate precipitation, 3 = heavy precipitation, 4 = no de-icing possible

* Mon Jun 15 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.15-1.fmi
- Tuned the FeelsLike radiation effect based on ApparentTemperature formulas

* Wed Jun 10 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.10-1.fmi
- Added parameter TotalColumnWater

* Mon Jun  8 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.8-1.fmi
- FeelsLike temperature now takes global radiation into account

* Wed Apr 29 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.29-1.fmi
- Enabled landscape interpolation to take water/land mask value as input
- Fixed PeekLocationValue not to modify the location index

* Wed Apr 22 2015  <mheiskan@centos7.fmi.fi> - 15.4.22-1.fmi
- Cleaned NFmiQueryDataUtil API
- Fixed NFmiGdalArea WorldXYWidth and WorldXYHeight methods

* Tue Apr 14 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.15-1.fmi
- Changed NFmiQueryData LatLonCache to be calculated on demand

* Thu Apr  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.9-2.fmi
- Fixed NFmiQueryData::Read to update the LatLonCache

* Thu Apr  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.9-1.fmi
- Made NFmiQueryData::LatLonCache method thread safe

* Mon Mar 30 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.3.30-1.fmi
- Added landscape-interpolation
- Build as a dynamic library instead of static

* Mon Mar 16 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.3.16-1.fmi
- New flight route methods
- Added parameter ClearAirTurbulance2

* Tue Feb 24 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.24-1.fmi
- Fixed WindChill formula to match the Canadien one for speeds < 5 km/h
- New version of FeelsLike combines a customized wind chill with the head index

* Tue Feb 17 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.17-2.fmi
- Added parameter PrecipitationInstantTotal

* Tue Feb 17 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.17-1.fmi
- Fixes to nearest neighbour interpolation for global data

* Fri Feb 13 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.13-1.fmi
- Fixes	to interpolation methods

* Fri Feb  6 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.6-1.fmi
- Fixes to interpolation for global data

* Fri Dec  5 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.12.5-1.fmi
- Added parameters ProbabilityOfPrecLimit5 - ProbabilityOfPrecLimit10

* Tue Nov 11 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.11.11-1.fmi
- Partio added PrecipitationForm3
- Fix to pressure level interpolation

* Wed Nov  5 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.11.5-1.fmi
- Fixed pressure level interpolation to handle subparameters correctly

* Thu Oct 30 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.10.30-1.fmi
- New way to interpolate with respect to pressure is more robust

* Mon Oct 13 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.10.13-2.fmi
- NFmiDataMatrix interpolation now understands nearest neighbor interpolation
- Fixes to Atlantic & Pacific view handling

* Mon Oct 13 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.10.13-1.fmi
- New precipitation and buoyancy parameters

* Tue Sep 30 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.9.30-1.fmi
- Added SeasonalWaterLevelIndex SYKE parameter

* Wed Aug  6 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.8.6-1.fmi
- Fixed WKT for NFmiLatLonArea and NFmiRotatedLatLonArea

* Tue Aug  5 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.8.5-1.fmi
- Fixed WKT for NFmiLatLonArea and NFmiRotatedLatLonArea

* Fri Aug  1 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.8.1-2.fmi
- Use the last file origin time for multiquerydata

* Fri Aug  1 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.8.1-1.fmi
- Added new MyOcean parameters: SeaDepth, MixedlayerThickness, SeaSurfaceElevation
- Marko: Fixed bug in Mercator projection which reflected southern hemisphere data to the northern hemisphere
- Marko: Various bug fixes

* Wed May 28 2014 Santeri Oksman <santeri.oksman@fmi.fi> - 14.5.28-1.fmi
- Added WeatherSymbol parameter

* Mon Apr 28 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.4.28-1.fmi
- Adjusted SYKE - related flooding parameters
- Fixed parameter conflicts due to simultaneous changes by two persons
- 381 ConvectiveInhibition is now Roughness, parameter 66 is to be used for CIN

* Thu Apr 10 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.10-1.fmi
- Added new pollen parameters for birch, grass, alder, mugwort and ragweed

* Mon Apr  7 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.7-1.fmi
- Partio added AbsoluteHumidity and PrecipitationInstantSolid
- Kinniä fixed define-directive handling in the preprocessor

* Fri Mar 28 2014 Pertti Kinniä <pertti.kinnia@fmi.fi> - 14.3.28-1.fmi
- Modifications to NFmiPreProcessor's #define handling (added some functionality needed by frontier)

* Fri Mar 14 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.3.14-1.fmi
- Added possibility to request a time bag when using MakeCombineParams (TAF editor fix)

* Thu Mar 13 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.3.13-1.fmi
- Added dry and wet deposition parameters produced by HILATAR model

* Wed Feb 19 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.2.19-1.fmi
- Wind U/V parameters fixed (BRAINSTORM-332 was reopened, previous fix was not ok)

* Mon Feb 17 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.2.17-1.fmi
- Wind U/V parameters fixed (BRAINSTORM-332)

* Mon Feb 10 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.2.10-1.fmi
- Fixed NFmiStaticTime::my_timegm to work beyond 2038

* Fri Jan 24 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.1.24-1.fmi
- Creating a new area from an original one no longer requires the new one to be inside the old one

* Wed Jan 22 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.1.22-1.fmi
- NFmiAreaFactory now supports creating a projection with the desired resolution
- NFmiAreaFactory now allows 'x' in addition to ',' in the grid size specification

* Mon Jan 20 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.1.20-1.fmi
- Fixed NFmiAreaFactory to throw for unknown projection names

* Thu Jan  9 2014 Santeri Oksman <santeri.oksman@fmi.fi> - 14.1.9-1.fmi
- Added trivial methods to calculate upper and lower limits of water to snow conversion

* Thu Dec  5 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.12.5-1.fmi
- IceVelocity parameter name changes and removed 396 and 397 parameters.

* Wed Dec  4 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.12.4-1.fmi
- Querydata loaded now makes an exact sanity check on the size of the input file raw data section

* Mon Dec  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.12.2-1.fmi
- Station ID is now a signed long

* Thu Nov 28 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.28-1.fmi
- Querydata loader now makes a sanity check on the size of the input file raw data section

* Wed Nov 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.27-1.fmi
- Added support for reading gnomonic querydata

* Mon Nov 25 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.25-1.fmi
- Bug fixes to Atlantic vs. Pacific view code

* Tue Nov 19 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.19-1.fmi
- Fixes to radiation parameter names

* Tue Nov 12 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.12-1.fmi
- New parameters for SADIS forecasts: ClearAirTurbulence, HorizontalExtentOfCumulonimbus, InCloudTurbulence

* Thu Nov  7 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.7-1.fmi
- New parameters for accumulated radiation (FMI open data)

* Wed Nov  6 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.6-2.fmi
- Added Snow1h parameter

* Wed Nov  6 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.6-1.fmi
- Added PrecipitationLevel parameter for radar precipitation adjusted to percieved strength

* Mon Nov  4 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.4-1.fmi
- Overflow bug fix to NFmiMetTime constructor

* Thu Oct 31 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.31-1.fmi
- NFmiMultiQueryInfo now has a time dependent origin time

* Wed Oct 30 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.30-1.fmi
- NFmiMultiQueryInfo now works with overlapping forecasts too

* Fri Oct 25 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.25-1.fmi
- NFmiMetTime now supports implicit conversion to boost::posix_time::ptime

* Thu Oct 24 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.24-1.fmi
- Support for Lambert equal area projection when reading querydata
- Support for boost::posix_time::ptime

* Thu Oct 17 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.17-1.fmi
- Added NFmiMultiQueryInfo for handling multiple datas with different times only
- Improvements to automatic calculation of snow fall rates

* Thu Sep 26 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.26-1.fmi
- Various fixes and improvements by Marko Pietarinen

* Mon Sep 23 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.23-2.fmi
- Patch to NFmiMultiQueryInfo

* Mon Sep 23 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.23-1.fmi
- Added NFmiQueryInfo::IsInside
- Added NFmiMultiQueryInfo

* Tue Sep 17 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.17-1.fmi
- Added NFmiHPlaceDescriptor::IsInside which is speed optimized with a NearTree
- Optimized NFmiLocationBag search operations for speed
- Added EastwardIceVelocity and NorthwardIceVelocity parameters.

* Tue Sep 10 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.9.10-1.fmi
- Added Runoff and WaterLevelDifference SYKE parameters

* Sat Sep  7 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.7-1.fmi
- Added possibility to output accurate precipitation dat into NFmiQueryDataUtil

* Thu Sep  5 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.5-1.fmi
- Added new Read and Write methods to NFmiQueryData

* Sat Aug  3 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-2.fmi
- Added possibility to advice NFmiQueryData memory mapping on access patterns

* Fri Aug  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-1.fmi
- Fixed weather symbol interpolation bug for non-composite weather params
- Enabled Pacific views of data by enabling 0...360 longitude views of data

* Mon Jul 22 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.22-1.fmi
- Fixed code not to call timegm due to thread safety issues
- Fixed code not to call readdir which is not re-entrant
- Fixed code to use rand_r instead of rand

* Wed Jul  3 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.3-1.fmi
- Updated to boost 1.54

* Tue Jul  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.2-1.fmi
- Added NFmiDataModifierChange to be able to process Harmonie precipitation data

* Tue Jun 25 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.6.25-1.fmi
- NFmiEnumConverter lacked 19 parameter names, fixed

* Mon Jun 10 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.6.10-1.fmi
- Using more cores when combining data
- Use 1 hour precipitation if available when combining data

* Tue Jun  4 2013 Timo Kuoremäki <timo.kuoremaki@fmi.fi> - 13.6.4-2.fmi
- Fixed build

* Tue Jun  4 2013 Timo Kuoremäki <timo.kuoremaki@fmi.fi> - 13.6.4-1.fmi
- Added Lambert azimuthal equal-area projection

* Mon May 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.5.27-1.fmi
- Added about 10 parameter names to EnumConverter

* Wed May 22 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.5.22-2.fmi
- Merged improvements made in Smartmet editor branch

* Wed May  22 2013 Roope Tervo <roope.tervo@fmi.fi> - 13.5.22-1.fmi
- fixed centered invrotlatlon projection

* Thu Apr  4 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.4.4-1.fmi
- Fixed NFmiFastQueryInfo::GridValues to rotate wind U/V components

* Wed Apr  3 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.4.3-1.fmi
- Fixed true north azimuth calculation for rotated latlon projection
- Fixed NFmiTotalWind construction from UV components to use the north azimuth

* Thu Mar 21 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.3.21-2.fmi
- Replaced auto_ptr with shared_ptr since C++11 deprecates it

* Thu Mar 21 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.3.21-1.fmi
- Added parameters WindGustU, WindGustV and HeightMinus20C to enum converter

* Wed Mar  6 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.3.6-1.fmi
- Fixed interpolation bug in InterpolatedValueFromTimeList

* Fri Feb 15 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.2.15-1.fmi
- New parameter names added by Mikko Partio
- Fixed NFmiFileSystem::FileSize to work for > 2GB files

* Wed Feb  6 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.2.6-1.fmi
- Pertti's additions

* Wed Jan 16 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.1.16-1.fmi
- Fixed NFmiLocationFinder to work with Unix linefeeds

* Mon Jan 14 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.1.14-1.fmi
- Added new parameter FlagDate

* Thu Jan 10 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.1.10-1.fmi
- WKT method is now compiled only in UNIX mode since the Windows version does not link with macgyver
- Added NFmiArea::CenterLatLon

* Fri Jan  4 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.1.4-1.fmi
- New parameter names for ground minimum temperature fractiles

* Tue Nov 27 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.11.27-1.fmi
- Fixes to wind interpolation

* Wed Nov  7 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.11.7-1.el6.fmi
- Upgrade to boost 1.52

* Fri Oct 19 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.10.19-2.el6.fmi
- Added one more new growth season parameter name

* Fri Oct 19 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.10.19-1.el6.fmi
- Added new growth season parameter names

* Thu Sep 27 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.9.27-1.el6.fmi
- Added RoadIceCover and RoadIceCoverOnVehiclePath parameters for the road model
- API improvements by Marko Pietarinen

* Mon Jul 23 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.23-1.el6.fmi
- Added a function for calculating the Apparent Temperature

* Fri Jul  6 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.6-1.el6.fmi
- Fixed issues discovered by cppcheck

* Thu Jul  5 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.5-1.el6.fmi
- Upgrade to boost 1.50 and icu 49 to get rid of libicu segmentation faults

* Wed Jul  4 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.4-1.el6.fmi
- New parameter names for wind fractiles

* Mon Jun  4 2012 Tuomo Lauri <tuomo.lauri@fmi.fi> - 12.6.4-1.el6.fmi
- Switched to use auto_ptr in NFmiAreaFactory interface

* Fri Jun  1 2012 Tuomo Lauri <tuomo.lauri@fmi.fi> - 12.6.1-1.el6.fmi
- Added Proj4-string support to NFmiAreaFactory

* Wed Apr 18 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.4.18-1.el5.fmi
- Fixed a special case in interpolation to return kFloatMissing when appropriate

* Fri Apr 13 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.4.13-1.el5.fmi
- Fixed month names and time phrases to use latin1

* Thu Apr 12 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.4.12-1.el5.fmi
- Fixed NFmiTime weekday names to use latin1

* Thu Apr  5 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.4.5-1.el5.fmi
- Fixed SummerSimmerIndex to use relative humidity in range 0-1 instead of 0-100

* Sat Mar 31 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.31-1.el5.fmi
- Added SummerSimmerIndex calculation
- Added FeelsLikeTemperature calculation

* Thu Mar 29 2012 Mikko Partio <mikko.partio@fmi.fi> - 12.3.29-1.el5.fmi
- Added new leveltype

* Wed Mar 28 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.28-1.el6.fmi
- Upgraded to boost 1.49

* Tue Mar 27 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.27-1.el6.fmi
- Added parameter AshOnOff for representing Met Office VAA/VAG ash boundary data

* Fri Mar 16 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.16-1.el6.fmi
- Added parameter kFmiConvectiveAvailablePotentialEnergy = 380
- Added parameter kFmiConvectiveInhibition = 381
- Added parameter kFmiStormRelativeHelicity = 382
- Fixed thread safety issues
- Removed unmapping of memory mapped data from NFmiRawData, kernel handles it better
- Removed read and write locks from NFmiRawData as unnecessary

* Fri Feb 24 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.2.24-1.el6.fmi
- Single threaded versions are no longer built
- Added +-inf and NaN detection and their conversion to kFloatMissing
- Multithreading time critical operations
- NFmiAreaMask classes now enable shallow copies
- Code optimizations

* Wed Feb  8 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.2.8-1.el6.fmi
- Added parameternames IceSpeed and IceDirection

* Tue Feb  7 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.2.7-2.el5.fmi
- WindChill formula will now use 1.7 m/s as the minimum wind speed (walking speed)

* Tue Feb  7 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.2.7-1.el5.fmi
- Added Friction parametername
- Removed block based memory mapping from NFmiRawData as detrimental to efficiency
- Bug fix to weather and cloudiness handling

* Fri Nov 25 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.25-1.el5.fmi
- Added an explicit << operator for FmiParameterName to get rid of g++ errors

* Thu Nov 24 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.24-1.el5.fmi
- Added RadarBorder parameter

* Wed Nov 16 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.16-1.el5.fmi
- Added cross section functions

* Tue Nov  8 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.8-1.el5.fmi
- Latvian language upgrades

* Mon Oct 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.17-3.el5.fmi
- Added GrowthPeriodStarted and GrowthPeriodEnded parameters

* Mon Oct 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.17-2.el5.fmi
- Numerous improvements and fixes by Marko Pietarinen

* Mon Oct 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.17-1.el5.fmi
- Added AshConcentration parameter
- Changed fmi.conf to smartmet.conf

* Wed Sep 21 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.9.21-1.el5.fmi
- Added new parameters Reflectivity, RadialVelocity and SpectralWidth

* Tue Aug 30 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.8.30-1.el6.fmi
- NFmiRotatedLatLon area improvement

* Wed Jul 20 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.7.20-1.el6.fmi
- Upgrade to boost 1.47

* Thu Jul  7 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.7.7-1.el6.fmi
- Added biomass parameters

* Wed Jun 15 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.6.15-1.el6.fmi
- Fixed NFmiFileSystem::PatternFiles to work when pattern contains no path component

* Mon Jun  6 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.6.6-1.el5.fmi
- Numerous improvements by Marko

* Tue May 31 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.5.31-1.el6.fmi
- Numerous improvements by Marko

* Thu May 26 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.5.26-1.el6.fmi
- Improvements to handle GRIB conversions better

* Fri May 20 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.5.20-1.el6.fmi
- RHEL6 release for Latvia

* Tue Apr 19 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.4.19-1.el5.fmi
- Fixed error messages to be in English

* Thu Mar 24 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.3.24-1.el5.fmi
- Upgraded to boost 1.46

* Thu Mar 17 2011 Pekka Keränen <pekka.keranen@geosaaga.fi> - 11.3.17-1.el6.fmi
- Fixed compiler errors in RHEL6

* Fri Mar 11 2011 Pekka Keränen <pekkka.keranen@geosaaga.fi> - 11.3.11-1.el6.fmi
- Fixed compiler errors in RHEL6

* Wed Feb 23 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.2.23-1.el5.fmi
- Added TrafficIndex for tiejaksokeli production

* Wed Feb  9 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.2.9-1.el5.fmi
- Fixed FillGridData not to thread when BOOST_DISABLE_THREADS is defined

* Tue Feb  8 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.2.8-1.el5.fmi
- Added parameter name RoadReasoning needed for Karttakeskus co-operation

* Thu Feb  3 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.2.3-1.el5.fmi
- New EPS parameter names

* Tue Jan 11 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.1.11-1.el5.fmi
- DirectoryFiles now skips files starting with a period.

* Tue Jan  4 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.1.4-1.el5.fmi
- Settings variable names can now contain whitespace

* Wed Sep 29 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.9.29-1.el5.fmi
- Fixed bugs found by clang++

* Tue Sep 14 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.9.14-1.el5.fmi
- Upgraded to boost 1.44

* Tue Jul  6 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.7.6-2.el5.fmi
- Removed NFmiLevel::gMissingLevel due to thread safety issues
- Added NFmiLevel::IsMissing() to replace gMissingLevel

* Mon Jul  5 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.7.5-2.el5.fmi
- Fixed numbers for SurfaceWaterPhase and ProbabilityOfSnow

* Mon Jul  5 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.7.5-1.el5.fmi
- Fixed hessaa interpretation to be the same as in press production

* Mon Jun  7 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.6.7-1.el5.fmi
- Added support for PKJ projection in NFmiAreaFactory
- NFmiVersion.h into use by Asko

* Fri Apr 23 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.4.23-1.el5.fmi
- Added Tuliset ensemble parameters

* Fri Mar 26 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.3.26-1.el5.fmi
- Various small bugfixes and improvements

* Tue Feb  2 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.2.2-1.el5.fmi
- Small new utility methods required by other programs

* Fri Jan 15 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.1.15-1.el5.fmi
- Upgrade to boost 1.41

* Tue Jan 12 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.1.12-1.el5.fmi
- Fixed NFmiSettings variable substitutions

* Mon Jan 11 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.1.11-1.el5.fmi
- Numerous small improvements and bug fixes

* Tue Jul 14 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.7.14-1.el5.fmi
- Upgrade to boost 1.39

* Tue Apr 21 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.4.21-1.el5.fmi
- Fixed bug in NFmiQueryInfo::InterpolatedValueForTimeBag

* Thu Apr 16 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.4.16-1.el5.fmi
- Bugfix to NFmiQueryInfo constructor for named files

* Wed Apr  8 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.4.8-1.el5.fmi
- Build both single threaded and multithreaded versions

* Mon Apr  6 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.4.6-1.el5.fmi
- Comment preprocessor crashed if file started with line containing only spaces

* Mon Mar 16 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.3.16-1.el5.fmi
- const correctness improvements

* Wed Mar  4 2009 Santeri Oksman <santeri.oksman@fmi.fi> - 9.3.4-2.el5.fmi
- Fixed if-else-endif parsering.

* Wed Mar  4 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.3.4-1.el5.fmi
- Fixed NFmiPreprocessor error message for missing include files

* Mon Feb  9 2009 Santeri Oksman <santeri.oksman@fmi.fi> - 9.2.9-1.el5.fmi
- Added NFmiLocationFinder::FindWmo by Lars Winberg
- New datatype kSingleStationRadarData by Marko Pietarinen
- New method NFmiMilliSecondTimer::CurrentTimeDiffInMSeconds by Marko Pietarinen
- Find in NFmiFileSystem is implemented using boost by Santeri Oksman

* Wed Jan 21 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.1.21-1.el5.fmi
- Added radar detectability (kFmiDetectability)

* Tue Jan 20 2009 Marko Pietarinen <marko.pietarinen@fmi.fi> - 9.1.20-1.el5.fmi
- Enabled larger filesizes in NFmiFileSystem

* Mon Jan 19 2009 Marko Pietarinen <marko.pietarinen@fmi.fi> - 9.1.19-1.el5.fmi
- Bug fixes on handling relative paths in NFmiFileString (Saudi-Arabia METARs)

* Fri Jan  9 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.1.9-1.el5.fmi
- Fixed V5 querydata write to always be in ASCII mode

* Thu Dec 18 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.12.18-1.el5.fmi
- NFmiEnumConverter is now case independent
- API additions by Marko Pietarinen

* Thu Nov 20 2008 Santeri Oksman <santeri.oksman@fmi.fi> - 8.11.20-1.el5.fmi
- Mika Heiskanen fixed a bug in if-else-endif parsing in NFmiPreProcessor.

* Mon Nov 10 2008 Antti Westerberg <antti.westerberg@fmi.fi> - 8.11.10-1.el5.fmi
- New parametername added (ExceptionalPrecipitation)

* Tue Oct 21 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.10.21-1.el5.fmi
- Changed const char * exceptions to runtime_errors

* Tue Oct  7 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.10.7-1.el5.fmi
- FindQueryData will now discard 0-size files

* Mon Sep 29 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.9.29-1.el5.fmi
- New parameternames added for eps fractiles (cloudiness + precipitation)

* Mon Sep 22 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.9.22-3.el5.fmi
- Revived NFmiMetBox since seaphone binary needs it

* Mon Sep 22 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.9.22-2.el5.fmi
- Marko Pietarinen fixed a bug in wind direction interpolation

* Mon Sep 22 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.9.22-1.el5.fmi
- A few bugfixes and improvements

* Thu Sep 11 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.9.11-1.el5.fmi
- Bugfix release

* Wed Jul 16 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.7.16-1.el5.fmi
- Added support for reading compressed querydata

* Tue Jul 15 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.7.15-1.el5.fmi
- Added support for memory mapped querydata
- Removed FmiRound and FmiTrunc macros

* Wed Mar 12 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.3.12-1.el5.fmi
- Fixed ARFLAGS

* Tue Mar 11 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.3.11-1.el5.fmi
- Fixed wind interpolation
- Minor code improvements

* Mon Feb 18 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.2.18-2.el5.fmi
- Precipitation form filtering improvements
- NFmiSettings API now uses strings instead of char *

* Fri Jan 25 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.1.25-2.el5.fmi
- Forgot to commit NFmiEnumConverter changes

* Fri Jan 25 2008 Mika Heiskanen <mika.heiskanen@fmi.fi> - 8.1.25-1.el5.fmi
- Added PedestrianIndex and TimeToNextSalting parameter names

* Wed Jan 16 2008 Mikko Rauhala <mikko.rauhala@fmi.fi> - 8.1.16-1.el5.fmi
- Various fixes by Marko Pietarinen,
- Changed to Ubuntu style version numbering

* Sat Jan 12 2008 mheiskan <mika.heiskanen@fmi.fi> - 1.0.8-1.el5.fmi
- GRIB related updates

* Thu Dec 27 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.7-1.el5.fmi
- Fixed interpolation bug in wind direction code

* Tue Dec 18 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.6-1.el5.fmi
- Various bugfixes by Marko Pietarinen
- Added missing Linux implementations to NFmiFileSystem namespace

* Fri Nov 30 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.5-1.el5.fmi
- Bugfixes in projection classes and wind chill calculations

* Mon Nov 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.3-1.el5.fmi
- Fixed a bug which caused qdinfo to crash for data with multiple levels

* Thu Nov 15 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.3-1.el5.fmi
- Added new parameters for EC EPS fractiles

* Thu Oct 18 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.2-1.el5.fmi
- Added EpochTime method to NFmiStaticTime
- Fixed typo in WKT() for NFmiStereographicArea
- Fixed regression test build system

* Mon Sep 24 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-4.el5.fmi
- Fixed "make depend".

* Fri Sep 14 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-3.el5.fmi
- Added "make tag" feature.

* Thu Sep 13 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-2.el5.fmi
- Simplied makefile logic.

* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 1.0.1-1.el5.fmi
- Changed file owners into root.
- Initial build.
