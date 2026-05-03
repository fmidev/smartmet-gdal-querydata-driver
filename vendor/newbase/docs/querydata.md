# QueryData File Format and Usage Guide

QueryData (`.sqd`) is the native binary format used by the FMI SmartMet system for storing
meteorological and geospatial data. It is a 4-dimensional dataset covering parameters,
horizontal locations, vertical levels, and times.

## Data Structure

QueryData is organized as a **4-dimensional array** with these axes:

| Dimension | Description | Descriptor Class |
|-----------|-------------|-----------------|
| Parameters | Physical quantities (temperature, wind, pressure, ...) | `NFmiParamDescriptor` |
| Horizontal locations | Grid points or named stations | `NFmiHPlaceDescriptor` |
| Vertical levels | Pressure or height levels (or a single surface level) | `NFmiVPlaceDescriptor` |
| Times | Valid times of the forecast or observation | `NFmiTimeDescriptor` |

All values are stored as `float32`. Missing values use the sentinel constant `kFloatMissing`.

### Horizontal location types

Depending on the data, horizontal locations can be one of:

- **Grid** (`IsGrid()` returns true): a regular 2D grid with an associated projection.
  Spatial interpolation is available.
- **Point locations** (`IsLocation()` returns true): a bag of named stations or
  observation points. No spatial interpolation.

### Memory layout

Data is stored in a flat array. Time varies fastest, then level, then location, then
parameter:

```
index = (paramIndex * sizeLocations * sizeLevels * sizeTimes)
      + (locationIndex * sizeLevels * sizeTimes)
      + (levelIndex * sizeTimes)
      + timeIndex
```

Sequential iteration over times is therefore the most cache-friendly access pattern.

## Key Classes

- **`NFmiQueryData`** — the on-disk container. Owns the raw data and the descriptors.
- **`NFmiQueryInfo`** — an iterator/accessor that wraps an `NFmiQueryData`. Use this for
  straightforward navigation through the dimensions.
- **`NFmiFastQueryInfo`** — a subclass of `NFmiQueryInfo` with index-based access,
  bulk-read methods, and cached interpolation. Prefer this for performance-sensitive code.

## Reading and Writing Files

```cpp
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"

// Read from a file (supports plain .sqd, .sqd.gz, .sqd.bz2)
NFmiQueryData qd("mydata.sqd");
NFmiFastQueryInfo info(&qd);

// Read via stream
NFmiQueryData qd2;
std::ifstream input("mydata.sqd");
input >> qd2;

// Write to a file
qd.Write("output.sqd");

// Write via stream
std::ofstream output("output.sqd");
output << qd;
```

## Iterating Through Data

All dimensions follow the same **reset/next** iterator pattern. After `Reset<Dim>()` the
iterator is positioned *before* the first element; each call to `Next<Dim>()` advances it
by one and returns `true` while there are more elements, `false` when the end is reached.

### Iterating over all times at fixed parameter, level, and location

```cpp
NFmiFastQueryInfo info(&qd);

// Fix the three outer dimensions
info.Param(kFmiTemperature);        // select a parameter by id
info.FirstLevel();                  // or info.Level(someLevel)
info.NearestPoint(NFmiPoint(25.0, 60.0));  // lon/lat, nearest grid point

for (info.ResetTime(); info.NextTime(); )
{
    float value = info.FloatValue();
    if (value != kFloatMissing)
    {
        NFmiMetTime t = info.Time();
        // ... use value and t
    }
}
```

### Iterating over all grid locations at fixed parameter, level, and time

```cpp
info.Param(kFmiTemperature);
info.FirstLevel();
info.FirstTime();   // or info.Time(someTime)

for (info.ResetLocation(); info.NextLocation(); )
{
    float value   = info.FloatValue();
    NFmiPoint ll  = info.LatLon();  // current point as (lon, lat)
    // ...
}
```

### Iterating over all parameters at fixed location, level, and time

```cpp
info.FirstTime();
info.FirstLevel();
info.NearestPoint(NFmiPoint(25.0, 60.0));

for (info.ResetParam(); info.NextParam(); )
{
    FmiParameterName id = static_cast<FmiParameterName>(info.Param().GetParamIdent());
    float value         = info.FloatValue();
    // ...
}
```

### Full nested iteration (all parameters × all times × all locations)

Arrange the loops so that **time is the innermost loop** to match the memory layout:

```cpp
for (info.ResetParam(); info.NextParam(); )
for (info.ResetLevel(); info.NextLevel(); )
for (info.ResetLocation(); info.NextLocation(); )
for (info.ResetTime(); info.NextTime(); )
{
    float value = info.FloatValue();
    // ...
}
```

## Reading a Complete 2D Field

For grid data, `Values()` returns an entire spatial slice as an `NFmiDataMatrix<float>`:

```cpp
info.Param(kFmiTemperature);
info.FirstLevel();
info.FirstTime();

NFmiDataMatrix<float> field = info.Values();
// field[x][y] — column-major indexing, x in [0, NX), y in [0, NY)
```

With time interpolation:

```cpp
NFmiMetTime targetTime(2024, 1, 15, 12, 30);
NFmiDataMatrix<float> field = info.Values(targetTime);
```

## Setting Values

```cpp
info.Param(kFmiTemperature);
info.FirstLevel();
info.FirstTime();
info.NearestPoint(NFmiPoint(25.0, 60.0));

info.FloatValue(273.15f);   // write to current position
```

Replace an entire 2D field:

```cpp
NFmiDataMatrix<float> field(nx, ny, 273.15f);
info.Param(kFmiTemperature);
info.FirstLevel();
info.FirstTime();
info.SetValues(field);
```

## Spatial Interpolation

For grid data, `InterpolatedValue()` performs bilinear interpolation at any lon/lat point:

```cpp
info.Param(kFmiTemperature);
info.FirstLevel();
info.FirstTime();

NFmiPoint lonlat(25.5, 60.3);  // (longitude, latitude)
float value = info.InterpolatedValue(lonlat);
```

## Temporal Interpolation

```cpp
info.Param(kFmiTemperature);
info.FirstLevel();
info.NearestPoint(NFmiPoint(25.0, 60.0));

NFmiMetTime targetTime(2024, 1, 15, 12, 30);
float value = info.InterpolatedValue(targetTime);
```

## Combined Spatial and Temporal Interpolation

```cpp
NFmiPoint lonlat(25.5, 60.3);
NFmiMetTime targetTime(2024, 1, 15, 12, 30);

float value = info.InterpolatedValue(lonlat, targetTime);
```

## Vertical Interpolation

### Interpolation to a pressure level (uses log-pressure)

```cpp
info.Param(kFmiTemperature);

float pressureHPa = 850.0f;
NFmiPoint lonlat(25.0, 60.0);
NFmiMetTime t(2024, 1, 15, 12, 0);

float value = info.PressureLevelValue(pressureHPa, lonlat, t);
```

### Interpolation to a height level (metres above ground/sea level)

```cpp
float heightM = 1500.0f;
float value   = info.HeightValue(heightM, lonlat, t);
```

## High-Performance Interpolation with Cached Indices

When interpolating the same location and/or time many times across different parameters or
levels, pre-compute the fractional indices once and reuse them:

```cpp
NFmiPoint lonlat(25.5, 60.3);
NFmiMetTime t(2024, 1, 15, 12, 30);
double xInd, yInd, tInd;

// These calls compute and cache the fractional grid/time indices
bool ok = info.GetLocationIndex(lonlat, xInd, yInd);
ok     &= info.GetTimeIndex(t, tInd);

if (ok)
{
    for (info.ResetParam(); info.NextParam(); )
    for (info.ResetLevel(); info.NextLevel(); )
    {
        float value = info.InterpolatedValue(lonlat, t); // still valid but slower
        // ... or use bulk methods with the cached indices
    }
}
```

For pressure-level interpolation with cached indices:

```cpp
double xInd, yInd, tInd, pInd;
double pressureHPa = 850.0;

info.GetLocationIndex(lonlat, xInd, yInd);
info.GetTimeIndex(t, tInd);
info.GetLevelIndex(lonlat, t, pressureHPa, pInd);

float value = info.FastPressureLevelValue(xInd, yInd, tInd, pInd);
```

## Cross-Sections

### Vertical cross-section along a path (fixed time)

```cpp
std::vector<NFmiPoint> path = { NFmiPoint(24.0, 60.0),
                                NFmiPoint(25.0, 60.5),
                                NFmiPoint(26.0, 61.0) };
std::vector<float> pressureLevels = { 1000.f, 925.f, 850.f, 700.f, 500.f };
NFmiMetTime t(2024, 1, 15, 12, 0);

// rows = pressure levels, columns = path points
NFmiDataMatrix<float> section = info.CrossSectionValuesLogP(t, pressureLevels, path);
```

### Time cross-section at a fixed location

```cpp
NFmiPoint station(25.0, 60.0);
NFmiTimeBag times(startTime, endTime, 60 /* step in minutes */);

NFmiDataMatrix<float> section = info.TimeCrossSectionValuesLogP(
    pressureLevels, station, times);
```

## Important Notes

### Missing values

Every read operation can return `kFloatMissing` to indicate absent or invalid data.
Always test before using a value:

```cpp
float v = info.FloatValue();
if (v != kFloatMissing)
    process(v);
```

### Checking dimension validity after navigation

Navigation methods (`Param(id)`, `Time(t)`, `Level(l)`, `NearestPoint(ll)`, ...) return
`false` when the requested position is not in the data. Always check the return value or
use the `IsXxxUsable()` guards before reading:

```cpp
if (info.Param(kFmiTemperature) && info.IsParamUsable())
    float v = info.FloatValue();
```

### NFmiQueryInfo vs NFmiFastQueryInfo

Use `NFmiFastQueryInfo` for all new code. It provides:

- Direct index-based access (`ParamIndex(i)`, `LocationIndex(i)`, ...)
- Bulk read/write: `GetValues()`, `SetValues()`, `GetLevelToVec()`, `GetCube()`
- Cached interpolation indices: `GetLocationIndex()`, `GetTimeIndex()`, `GetLevelIndex()`
- Faster iterator implementation

`NFmiQueryInfo` is the base class and is retained for compatibility.

### Thread safety

`NFmiFastQueryInfo` instances share the underlying `NFmiQueryData` (which is read-only
after construction). Create a separate `NFmiFastQueryInfo` per thread — the iterators
are not thread-safe, but multiple iterators over the same read-only `NFmiQueryData` are
safe.

## Reference

| Class | Header |
|-------|--------|
| `NFmiQueryData` | `newbase/NFmiQueryData.h` |
| `NFmiQueryInfo` | `newbase/NFmiQueryInfo.h` |
| `NFmiFastQueryInfo` | `newbase/NFmiFastQueryInfo.h` |
| `NFmiParamDescriptor` | `newbase/NFmiParamDescriptor.h` |
| `NFmiTimeDescriptor` | `newbase/NFmiTimeDescriptor.h` |
| `NFmiHPlaceDescriptor` | `newbase/NFmiHPlaceDescriptor.h` |
| `NFmiVPlaceDescriptor` | `newbase/NFmiVPlaceDescriptor.h` |
| `NFmiDataMatrix<T>` | `newbase/NFmiDataMatrix.h` |
| `NFmiQueryDataUtil` | `newbase/NFmiQueryDataUtil.h` |

Regression tests with working code examples can be found in `test/NFmiQueryInfoTest.cpp`
and `test/NFmiFastQueryInfoTest.cpp`.
