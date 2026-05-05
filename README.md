# smartmet-gdal-querydata-driver

A GDAL driver plugin for FMI's **QueryData** (`.sqd`, `.fqd`) raster format. Drop the plugin into your GDAL installation and `.sqd` files become a normal raster source for the entire GDAL ecosystem:

- **CLI / desktop GIS** — `gdalinfo`, `gdal_translate`, `gdalwarp`, QGIS, GRASS GIS, SAGA, ArcGIS Pro, FME.
- **Map / tile servers** — MapServer, GeoServer (via ImageIO-Ext), Mapnik, `gdal2tiles`, TiTiler, `rio-tiler`, terracotta.
- **Language bindings** — Python (`rasterio`, `xarray` / `rioxarray`, `osgeo.gdal`), R (`terra`, `sf`, `stars`), Julia (`ArchGDAL.jl`), Rust (`gdal`), Go (`godal`), Node.js (`gdal-async`), Java, .NET, MATLAB.
- **Cloud-native / STAC** — anything in the `pystac` / `odc-stac` / `stackstac` / planetary-computer chain, since they read through `rasterio`.

## Why this exists

QueryData is a **legacy FMI-internal format** dating to the 1990s. It predates the modern geospatial-format consensus (NetCDF / GeoTIFF / Zarr / GRIB) and is not part of upstream GDAL. The format is maintained inside the SmartMet ecosystem mainly for backward compatibility with decades of archived forecast and observation data.

This plugin is a **one-way bridge from the legacy archive into modern tooling**. Reading is the first-class case; writing exists for round-trip and quick interop, but for new data products you almost certainly want to write GeoTIFF, NetCDF, or Zarr instead.

The format itself is documented in [smartmet-library-newbase/docs/querydata.md](https://github.com/fmidev/smartmet-library-newbase/blob/master/docs/querydata.md).

## Capabilities

- **Read** any gridded `.sqd` / `.fqd` file (station-data files are not supported).
- **Subdatasets** — one entry per `(parameter, level)` combination, with one band per time step.
- **Composite parameter unpacking** — packed `NFmiTotalWind` and `NFmiWeatherAndCloudiness` parameters expand into their sub-components (wind speed, wind direction, U, V, total cloud cover, precipitation form, etc.) instead of exposing the raw bit-packed float.
- **Native projection detection** — picks the right newbase `NFmiArea` subclass (Polar Stereographic, Lambert, Mercator, LatLon, etc.) when the source SRS is recognised, falls back to `NFmiGdalArea` otherwise.
- **UTF-8** parameter and level names (newbase stores them as Latin-1; the plugin transcodes).
- **Multi-Dimensional Raster** facade — exposes the QD as a 4-D `(time, level, y, x)` array per parameter, suitable for `xarray` / Zarr / NetCDF consumers via `gdalmdiminfo` and `gdalmdimtranslate`.
- **Write** via `CreateCopy` — single-parameter, regular gridded inputs. Faithfully round-trips `.sqd → .sqd` (preserves SRS, geotransform, time stamps, band statistics).

## Installation

The plugin must be built against the same GDAL major.minor version as the consumer that will load it (GDAL plugin ABI is only stable within `3.X.*`).

### From source (Linux)

Build dependencies:

| Package | Notes |
|---|---|
| `gdal312-devel` | or whichever `gdal3XY-devel` matches your runtime GDAL |
| `smartmet-library-newbase-devel` | |
| `smartmet-library-macgyver-devel` | |
| `smartmet-library-gis-devel` | |
| `fmt-devel` | 12.x |
| `gcc-c++` | C++17+ |

```bash
make
sudo make install      # → $(libdir)/gdalplugins/gdal_querydata.so
```

Or build an RPM (the standard FMI deployment route):

```bash
make rpm
sudo dnf install smartmet-gdal-querydata-driver-*.rpm
```

### Verifying

```bash
gdalinfo --formats | grep querydata
#   querydata -raster,multidimensional raster- (rws): FMI QueryData (*.sqd, *.fqd)
```

The `(rws)` capabilities mean **r**ead, **w**rite (via CreateCopy), **s**ubdatasets.

### QGIS, conda, macOS, Windows

QGIS and other end-user packages typically bundle their own GDAL build. The plugin must match that GDAL's version. Either:

1. Install the plugin into *that* GDAL's `gdalplugins/` directory, **or**
2. Set `GDAL_DRIVER_PATH=/path/to/plugins` in the environment QGIS launches under.

For conda-forge installations the easiest route is a recipe parameterised on the `gdal` version pin.

## Usage

### gdalinfo

```bash
# Top-level view: lists subdatasets, one per (param, level)
gdalinfo forecast.sqd

# Open one subdataset (param index 0, level index 0)
gdalinfo 'querydata:"forecast.sqd":0:0'
```

### gdal_translate

```bash
# Convert one (param, level) slice to GeoTIFF (one band per time step)
gdal_translate 'querydata:"forecast.sqd":0:0' temperature.tif

# Or to NetCDF, Zarr, COG, anything GDAL supports
gdal_translate -of NetCDF 'querydata:"forecast.sqd":0:0' temperature.nc

# Export back to QueryData (single-parameter)
gdal_translate -of querydata temperature.tif out.sqd
```

### gdalwarp

```bash
# Reproject from FMI Polar Stereographic to WGS84
gdalwarp -t_srs EPSG:4326 'querydata:"forecast.sqd":0:0' temperature_wgs84.tif
```

### QGIS

**Layer → Add Layer → Add Raster Layer**, pick a `.sqd` file. QGIS uses GDAL for raster I/O, so once the plugin is on the GDAL plugin path it Just Works — no QGIS-side configuration. For multi-parameter files QGIS shows its standard subdataset picker (the same UI it uses for NetCDF / HDF5).

### Python — rasterio (classic 2D)

```python
import rasterio

with rasterio.open('querydata:"forecast.sqd":0:0') as src:
    print(src.profile)
    arr = src.read(1)             # first time step
    times = [src.tags(i)['VALID_TIME'] for i in range(1, src.count + 1)]
```

### Python — xarray (multi-dimensional)

```python
import xarray as xr

# Each parameter (and sub-parameter) becomes its own DataArray with
# (time, level, y, x) dimensions
ds = xr.open_dataset('forecast.sqd', engine='rasterio',
                     open_kwargs={'OF': 'multidim_raster'})
ds.Lampotila.sel(time='2025-01-15T12:00').plot()
```

## Subdataset URL syntax

```
querydata:"<path>":<paramIdx>:<levelIdx>[:<subParamId>]
```

- `<path>` — path to the `.sqd` / `.fqd` file (quoted to allow paths containing colons).
- `<paramIdx>` — zero-based parameter index in the file.
- `<levelIdx>` — zero-based level index.
- `<subParamId>` — *optional* `FmiParameterName` value to activate a sub-parameter of a composite (e.g. `21` = wind speed from a TotalWind parent). When present, the data array returns the unpacked component rather than the raw bit-packed parent value.

The same URL form is what appears in the `SUBDATASET_n_NAME` metadata that `gdalinfo` lists at the top level.

## Sub-parameter unpacking

QueryData encodes a few parameters as bit-packed multi-component floats:

| Parent parameter | Id | Sub-parameters |
|---|---|---|
| `kFmiTotalWindMS` | 19 | wind speed (21), wind direction (20), wind vector (22), max gust (467), U (49), V (50) |
| `kFmiWeatherAndCloudiness` | 326 | total / low / medium / high cloud cover, precipitation 1 h, precipitation form / type, thunder probability, fog intensity |

The driver expands each parent into one subdataset per sub-component. If you only see the parent parameter and want the sub-components, that's a sign the file uses an older non-composite encoding — try opening with the parent index directly.

## Multi-Dimensional Raster (xarray / Zarr / NetCDF)

```bash
# Inspect the MDR view
gdalmdiminfo forecast.sqd

# Translate one variable to NetCDF, with all dimensions preserved
gdalmdimtranslate -array Lampotila forecast.sqd temperature.nc
```

The root group exposes:

- One `GDALMDArray` per parameter (and per sub-parameter for composites), named after the parameter with Finnish accents transliterated (`Lämpötila` → `Lampotila`) so they're valid identifiers in netCDF / Zarr / HDF5.
- Coordinate variables `time` (Int64 epoch seconds), `level` (Float32), `x`, `y` (Float64 in the source SRS).
- The SRS via `GetSpatialRef()` on each data array; nodata = `kFloatMissing` (32700).

## Writing QueryData (CreateCopy)

`gdal_translate -of querydata source out.sqd` works for any GDAL source that satisfies:

- A regular grid (no rotated geotransform).
- A spatial reference recognised by `NFmiArea::CreateFromBBox` — most common projections work; truly exotic ones fall back to `NFmiGdalArea` which round-trips but is less efficient.
- A single parameter (one band per time step).

### Creation options

| Option | Default | Description |
|---|---|---|
| `PARAM_ID` | 4 (Temperature) | FMI parameter id |
| `PARAM_NAME` | `"Data"` | Parameter name |
| `ORIGIN_TIME` | first valid time | ISO 8601 origin / analysis time |
| `START_TIME` | from band metadata | ISO 8601 first valid time |
| `TIMESTEP_MINUTES` | 60 | Interval between consecutive bands |
| `LEVEL_VALUE` | 0 | Level value |
| `LEVEL_TYPE` | 1 (ground/surface) | `FmiLevelType` enum value |

```bash
gdal_translate -of querydata \
  -co PARAM_ID=4 -co PARAM_NAME=Temperature \
  -co ORIGIN_TIME=2025-01-15T00:00:00Z \
  -co START_TIME=2025-01-15T00:00:00Z -co TIMESTEP_MINUTES=60 \
  source.tif forecast.sqd
```

If the source dataset has FMI-style metadata (`PARAM_ID`, `VALID_TIME`, `ORIGIN_TIME`) — for example a GeoTIFF produced by this plugin — those values are picked up automatically and creation options are not needed.

### What write does *not* do

The legacy semantics that a hand-built `.sqd` carries are not reconstructed:

- **Composite parameters** are not re-packed. To write wind, write each component as a separate single-parameter `.sqd` or use a different format.
- **Multiple parameters / multiple levels in one file** are not supported. The C++ `newbase` API can build such files; this plugin does not.
- **Producer / ensemble / hybrid-coordinate metadata** are not exposed via creation options.

For most workflows the right answer is to write to a modern format. Use `.sqd` output only when you specifically need a downstream FMI tool that consumes QueryData.

## Limitations

- Station / point-data `.sqd` files (using `NFmiLocationBag` rather than a regular grid) are explicitly rejected — open them with `newbase` instead.
- Plugin ABI matches one GDAL major.minor; rebuild against your target GDAL version.
- `NFmiGdalArea` (used as the projection fallback when `DetectClassId` doesn't recognise the source SRS) has known quality issues but is ABI-frozen in newbase. Stick to standard projections where possible.
- YKJ (EPSG:2393) is deprecated in modern PROJ (~200 m positional error due to removal of `+towgs84`); the plugin reads existing YKJ files but you should not produce new ones.

## License

MIT — see [LICENSE](LICENSE).

## See also

- [smartmet-library-newbase](https://github.com/fmidev/smartmet-library-newbase) — the QueryData format library this plugin wraps. Its [`docs/querydata.md`](https://github.com/fmidev/smartmet-library-newbase/blob/master/docs/querydata.md) is the canonical format spec.
- [SmartMet Server](https://github.com/fmidev/smartmet-server) — the broader ecosystem.
- [smartmet-qdtools](https://github.com/fmidev/smartmet-qdtools) — command-line tools for QueryData manipulation (predates this plugin; still the right tool for tasks the plugin doesn't cover, like format-specific repacking).

## Contributing

Bug reports and pull requests are welcome on [GitHub](../../issues).
