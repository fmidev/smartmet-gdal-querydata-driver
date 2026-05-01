# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`smartmet-gdal-querydata-driver` is an **out-of-tree GDAL driver plugin** that exposes FMI's **QueryData (`.sqd`, `.fqd`) format** to GDAL consumers (gdalinfo, gdal_translate, gdalwarp, QGIS, rasterio, xarray via rioxarray).

**QueryData is a legacy FMI-internal format.** It dates to the 1990s, predates the modern geospatial-format consensus (NetCDF / GeoTIFF / Zarr / GRIB), and is not — and will not be — part of upstream GDAL. The format is maintained only inside the SmartMet ecosystem, primarily for backward compatibility with decades of archived forecast and observation data. New data products at FMI generally do not start in QueryData; this plugin exists to *bridge* the legacy archive into modern tooling, not to encourage it as a target format.

The format itself is documented in newbase: see [`smartmet-library-newbase/docs/querydata.md`](https://github.com/fmidev/smartmet-library-newbase/blob/master/docs/querydata.md) (or `~/hub/newbase/docs/querydata.md` locally) for the binary layout, the descriptor classes, and the 4D `parameters × locations × levels × times` data model. The newbase [README](https://github.com/fmidev/smartmet-library-newbase) and `~/hub/newbase/CLAUDE.md` cover the C++ API.

That legacy status shapes the design priorities of this driver:

- **Reading is the first-class case.** It needs to work for any historical `.sqd` you encounter — multiple parameters, levels, time steps, sub-parameter unpacking from packed `NFmiTotalWind` / `NFmiWeatherAndCloudiness` floats, FMI's full hierarchy of native projections, occasional `kTopLeft` grids alongside the usual `kBottomLeft`, Latin-1 parameter names. Both the classic 2D + subdatasets facade and the Multi-Dimensional Raster facade are aimed at *getting data out*.
- **Writing (`CreateCopy`) is a secondary, narrower path.** It is good enough for `gdal_translate -of querydata` round-trips, single-parameter exports, and quick interop tests, but it makes no attempt to faithfully reconstruct the rich legacy semantics (composite parameters, ensemble members, hybrid vertical coordinates, producer metadata). For most workflows the right answer is to write to GeoTIFF / NetCDF / Zarr instead — the read side here makes that easy. If a write fails on an unsupported input, prefer surfacing a clear `CPLError` over silently producing a degenerate file.

The plugin is loaded by GDAL at runtime from `$GDAL_DRIVER_PATH` and registers a driver named `querydata` whose entry point is the `GDALRegisterMe()` symbol in `QDDriver.cpp`. Everything else — `QDDataset`, `QDRasterBand`, the multidim facade, `CreateCopy` — lives behind that single entry point. The rest of the SmartMet ecosystem (~65 repos under `~/hub`) reads QueryData directly via `smartmet-library-newbase`; this driver is the only path *outward* into general geospatial tooling.

The wider hub context (build conventions, layering of newbase / spine / engines / plugins) is in `/home/mheiskan/hub/CLAUDE.md`. This file covers only the driver-specific bits.

## Build / install / test

```bash
make                                     # builds gdal_querydata.so
make install                             # → $(libdir)/gdalplugins/gdal_querydata.so
make rpm                                 # builds RPM (smartmet-gdal-querydata-driver)
make format                              # clang-format the source
make clean
make test                                # runs test/smoke.sh
```

Build needs `gdal312-devel`, `smartmet-library-newbase-devel`, `smartmet-library-macgyver-devel`, `smartmet-library-gis-devel`, and `fmt-devel`. Headers from GDAL 3.12 are picked up automatically by the smartbuildcfg machinery in `makefile.inc` (which adds `-isystem /usr/gdal312/include` when `REQUIRES` mentions `gdal`). **Note:** clangd / LSP usually doesn't see those headers, so red squiggles like "GDALDataset not found" in the IDE are normal — trust `make`, not the LSP.

### Testing without `make install`

The smoke test stages the `.so` into a local `plugins/` directory and overrides `GDAL_DRIVER_PATH`:

```bash
cp gdal_querydata.so plugins/
export GDAL_DRIVER_PATH=$(pwd)/plugins
/usr/gdal312/bin/gdalinfo --formats | grep querydata
/usr/gdal312/bin/gdalinfo path/to/file.sqd
/usr/gdal312/bin/gdalmdiminfo -array Lampotila path/to/file.sqd
```

Sample data lives at `../newbase/test/data/*.sqd`.

## Architecture

The driver is **three facades over the same `NFmiFastQueryInfo` reader**:

1. **Classic 2D + subdatasets** (`QDDataset`, `QDRasterBand`). Top-level open of `file.sqd` enumerates one subdataset per `(parameter, level)` via `SUBDATASETS` metadata; opening a subdataset URL `querydata:"file.sqd":P:L[:S]` yields a 2D raster with one band per time step. Composite parameters (TotalWind id 19, WeatherAndCloudiness id 326) expand into one subdataset per sub-parameter; the optional `:S` segment of the URL is the sub-param's `FmiParameterName`. This is the path QGIS and `gdal_translate` use.

2. **Multi-Dimensional Raster** (`QDMultidim.cpp`). `QDDataset::GetRootGroup()` returns a `QDRootGroup` exposing one `GDALMDArray` per `(param, sub-param)` shaped `(time, level, y, x)` plus coordinate variables (`x`, `y`, `level`, `time` as Int64 epoch seconds with a `units` attribute). This is the path `gdalmdiminfo` and `xarray` use.

3. **CreateCopy** (`QDWrite.cpp`, registered via `pfnCreateCopy`). Reads any GDAL source dataset, builds an `NFmiArea` via `NFmiArea::CreateFromBBox(SpatialReference, blWorldXY, trWorldXY)` so native newbase projections are picked when `DetectClassId` recognizes the source SRS (Polar Stereographic, Mercator, Lambert, LatLon, etc.), and only falls back to `NFmiGdalArea` for unsupported SRS. Time, parameter id, and level can come from band metadata or creation options.

Strings cross a Latin-1 ↔ UTF-8 boundary at every read/write: newbase stores parameter names in Latin-1, GDAL metadata is UTF-8 by contract. `QDStrings.h` has `toUtf8` and `fromUtf8` helpers; **use them at every boundary**, not just where it currently breaks visibly. For MDArray identifiers (which need ASCII for netCDF/Zarr/HDF5), the Latin-1 source is also transliterated (ä→a, ö→o, å→a) so arrays get readable names like `Lampotila` rather than `L__mp__tila`.

QD grids default to `kBottomLeft` starting-corner storage; GDAL's row 0 is the top. Both reads (in `QDRasterBand::IReadBlock` and `QDDataMDArray::IRead`) and writes (in `createCopy`) flip Y. Don't remove the flip without checking `qi.Grid()->Origo()` — files do exist with `kTopLeft`.

## Gotchas hit during implementation

These are non-obvious and not documented in newbase. Save future agents the time:

- **`NFmiQueryData::Write(const char*)` resolves to the `Write(bool)` stdout overload.** `const char*` converts to `bool` more cheaply than to `std::string`. Always pass `std::string(path)` explicitly. Symptom: file binary contents printed to stdout, on-disk file is 0 bytes.

- **`NFmiQueryData(info)` does not allocate the data pool.** It only copies descriptors. Use `NFmiQueryDataUtil::CreateEmptyData(info)` instead — that allocates a `kFloatMissing`-filled pool of the right size. Symptom: written file is ~600 bytes (header only), reads back with "Invalid datapool size in querydata".

- **`NFmiHPlaceDescriptor(area, grid)` reports `Size() == 1`.** Both `itsArea` and `itsGrid` get set, and `Size()` short-circuits on the `IsArea()` branch before checking the grid. For write you must use the grid-only constructor `NFmiHPlaceDescriptor(grid)`. Symptom: `SetValues` writes exactly one cell (the rest stay `kFloatMissing`); the resulting file shows min=max=value-at-[0,0].

- **`Param(FmiParameterName)` is the only way to activate a sub-param.** Calling `ParamIndex(idx)` afterwards resets `fUseSubParam` to false. In `QDRasterBand::IReadBlock` and `QDDataMDArray::IRead`, when a sub-param is configured, call `qi.Param(static_cast<FmiParameterName>(subParamId))` and **do not** also call `ParamIndex`. Symptom: reads return raw bit-packed parent values instead of unpacked sub-components.

- **`#include <gdal_multidim.h>` requires `<gdal_priv.h>` first** (otherwise `gdal_pam_multidim.h` fails because `GDALMDArray` is only forward-declared at that point). Always include `gdal_priv.h` ahead of any multidim header.

- **GDAL plugin entry point is `GDALRegisterMe`**, not `GDALRegister_<DriverName>`. The latter is for in-tree drivers. Get this wrong and the `.so` loads silently but registers nothing.

- **`MDArray::Create` factory + `SetSelf(p)` is mandatory.** GDAL needs a `weak_ptr<self>` for some operations. If you skip it, `gdalmdiminfo -stats` prints "Driver implementation issue: m_pSelf not set !". All array classes in `QDMultidim.cpp` follow this pattern — keep it that way.

- **The dataset-level `itsGridXSize` / `itsGridYSize` are zero on a top-level open** (only `openSubdataset` populates them). The MDR facade lives on the top-level dataset, so its arrays must capture grid dimensions explicitly at construction time, not via `itsDataset.gridYSize()`. Symptom: Y-flip math underflows `unsigned long`, garbage reads.

## Project-level constraints

- **`NFmiGdalArea` is ABI-frozen and known-wobbly.** It cannot be improved without breaking newbase ABI across ~65 downstream repos. Work around its limits in driver code (e.g. by routing through `NFmiArea::CreateFromBBox` so native projections are preferred); don't try to fix it in place.

- **YKJ (EPSG:2393) is deprecated and broken in modern PROJ.** PROJ removed `+towgs84`, leaving ~200 m positional error even when invoked via the EPSG code. Don't add new YKJ-aware code paths; existing legacy support stays for reading old archives.

## File magic

QueryData files always start with the 9 bytes `@$\xb0\xa3QINFO` (followed by `@$\xb0\xa3 VER N\n` where N is 6 or 7). `QDDataset::Identify` matches on those 9 bytes; the file extension (`.sqd` / `.fqd`) is registered for file-dialog filtering only, not used for identification.
