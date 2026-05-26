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

### Debian / Ubuntu

Build a binary `.deb`:

```bash
sudo apt-get install -y \
    build-essential pkg-config dpkg-dev debhelper \
    libgdal-dev libgeos++-dev libproj-dev libfmt-dev libicu-dev \
    libdouble-conversion-dev \
    libboost-chrono-dev libboost-iostreams-dev libboost-regex-dev \
    libboost-serialization-dev libboost-thread-dev
make deb
sudo dpkg -i ../smartmet-gdal-querydata-driver_*.deb
```

`make deb` is a thin wrapper around `dpkg-buildpackage -us -uc -b`; the resulting `.deb` files land in the parent directory (standard `dpkg-buildpackage` behaviour). Runtime dependencies are resolved automatically by `dh_shlibdeps` from the libraries the `.so` actually links against, so the package picks the correct `libgdalXX`, `libfmtX`, etc. for the host.

**GDAL version requirement:** the driver source uses headers and types added in GDAL 3.10 (`gdal_dataset.h`, `gdal_geotransform.h`, the `GDALGeoTransform` class). That means:

- **Ubuntu 26.04 LTS or newer** — works out of the box; stock `libgdal-dev` is recent enough.
- **Ubuntu 24.04 LTS (noble)** — stock `libgdal-dev` is 3.8.x and is too old. Enable the [ubuntugis](https://launchpad.net/~ubuntugis/+archive/ubuntu/ubuntugis-unstable) PPA before installing build-deps:
  ```bash
  sudo add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable
  sudo apt-get update
  ```
  then proceed with the `apt-get install` above. The PPA ships a newer libgdal that exposes the headers the driver needs.
- **Debian** — no LTS-style guarantees here; check that your distribution's `libgdal-dev` advertises GDAL ≥ 3.10. If not, build GDAL from source or pull it from `bookworm-backports` / `trixie`.

CircleCI builds the `.deb` on `ubuntu:26.04` only — see `.circleci/config.yml`'s `build-ubuntu26` job.

### macOS (Homebrew, Apple Silicon)

```bash
brew tap fmidev/smartmet
brew install fmidev/smartmet/smartmet-gdal-querydata-driver
```

The plugin is installed at `$(brew --prefix)/lib/gdalplugins/gdal_querydata.dylib`, where the Homebrew GDAL picks it up automatically. Verify with:

```bash
gdalinfo --formats | grep querydata
```

The tap currently only ships Apple Silicon (`arm64_tahoe`) bottles — upstream Homebrew core has dropped Intel macOS bottles for several of our dependencies (`boost`, `fmt`, `howard-hinnant-date`, `libpq`, `libpqxx`). On Intel Macs you can still build from source, but it's unsupported.

For QGIS, see the section below — QGIS bundles its own GDAL and needs its plugin path pointed explicitly.

### conda

For conda-forge environments the easiest route is a recipe parameterised on the `gdal` version pin (none currently published; PRs to [conda-forge/staged-recipes](https://github.com/conda-forge/staged-recipes) welcome).

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

QGIS bundles its own GDAL build, so it doesn't see plugins installed for the system / Homebrew GDAL automatically. Two one-time setup steps:

**1. Tell QGIS's GDAL where the plugin lives** — through QGIS's own settings (no shell env needed):

1. **Settings → Options → System → Environment**
2. Tick **Use custom variables (restart required)**
3. Click **+** and add a row:
   - **Apply:** `Overwrite`
   - **Variable:** `GDAL_DRIVER_PATH`
   - **Value:** the directory containing `gdal_querydata.so` / `.dylib`. On Homebrew (Apple Silicon) that's `/opt/homebrew/lib/gdalplugins`; on a Linux RPM install it's typically `/usr/lib64/gdalplugins`.
4. **OK**, then fully quit QGIS (Cmd-Q on macOS, *not* just close the window) and relaunch.

Verify in **Plugins → Python Console**:

```python
from osgeo import gdal
print(gdal.GetDriverByName('querydata'))
```

A `Driver` object means the plugin loaded; `None` means the path is wrong or QGIS wasn't fully restarted.

**2. Loading a `.sqd` file.** Every QueryData file exposes its parameters as GDAL **subdatasets**, so opening one through **Layer → Add Raster Layer** (or drag-and-drop from Finder / Explorer) gives you a subdataset picker — pick the parameter and level you want and it loads as a regular raster. Same UX QGIS uses for NetCDF and HDF5 multi-variable files.

If you need scripted access, the equivalent in **Plugins → Python Console** is:

```python
# subdataset URI is querydata:"<path>":<paramIdx>:<levelIdx>
# — paramIdx / levelIdx are zero-based indexes into the file, not parameter
# IDs. Run `gdalinfo forecast.sqd` (or open in QGIS once and check the
# subdataset picker) to see which index maps to which parameter.
uri = 'querydata:"/path/to/forecast.sqd":3:0'
QgsRasterLayer(uri, 'my-layer-name')
```

The full subdataset URIs are listed under `SUBDATASET_n_NAME` in `gdalinfo forecast.sqd`.

**3. Switch the renderer to Singleband gray (one-time per layer).** A loaded subdataset has one band per timestep — typically dozens — and QGIS unconditionally picks the **Multiband color** renderer for any raster with ≥3 bands, mapping bands 1/2/3 to R/G/B. For a time series this is meaningless: you'll see the first three forecast hours blended into a coloured mush. (Same QGIS behaviour applies to multi-band NetCDF and GRIB files; it's not specific to this driver.)

Fix it:

- **Layer Properties → Symbology → Render type:** change to **Singleband gray** (or **Singleband pseudocolor** for a colour ramp).
- **Gray band:** pick the timestep you want as the "static" view (band 1 = first time step).
- **Apply.**

If you do this often, save it as the default for new raster layers: right-click the styled layer → **Styles → Save as Default → Datasource Database**. That sets your preferred renderer for any future raster QGIS opens.

**4. Animate through timesteps.** Each band carries `time` and `NETCDF_DIM_time` metadata so the QGIS Temporal Controller can drive animation automatically:

- **Layer Properties → Temporal:** tick **Temporal**, set **Configuration: Fixed temporal range per band**, click **Calculate** — it auto-fills the per-band start/end times from the metadata.
- Top toolbar → click the 🕒 **Temporal Controller** icon → enable "Animated temporal navigation" → press play. QGIS walks through all timesteps in order.

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
