#!/bin/bash
# Smoke test: verify that the plugin loads, identifies a .sqd file, enumerates
# subdatasets (with composite-parameter expansion and UTF-8 names), exposes the
# Multi-Dimensional Raster facade, and round-trips through CreateCopy.

set -eu  # no -o pipefail: head/grep closing early triggers SIGPIPE on producers

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PLUGIN_DIR="$REPO_DIR/plugins"
OUTPUT_DIR="$REPO_DIR/test/output"
GDAL_BIN="${GDAL_BIN:-/usr/gdal312/bin}"

mkdir -p "$PLUGIN_DIR" "$OUTPUT_DIR"
cp -f "$REPO_DIR/gdal_querydata.so" "$PLUGIN_DIR/"
export GDAL_DRIVER_PATH="$PLUGIN_DIR"

SAMPLE="$REPO_DIR/../newbase/test/data/hiladata.sqd"
if [[ ! -f "$SAMPLE" ]]; then
  echo "Skipping: sample file $SAMPLE not found"
  exit 0
fi

echo "== Driver advertised capabilities =="
"$GDAL_BIN/gdalinfo" --formats | grep -i querydata

echo
echo "== Subdatasets (composite params expanded, UTF-8) =="
INFO=$("$GDAL_BIN/gdalinfo" "$SAMPLE")
echo "$INFO" | grep -aE "SUBDATASET_(1|3|6)_DESC"

echo
echo "== Open subdataset (classic 2D + bands) =="
SUB_INFO=$("$GDAL_BIN/gdalinfo" "querydata:\"$SAMPLE\":0:0")
echo "$SUB_INFO" | head -3

echo
echo "== Translate one band to GeoTIFF =="
"$GDAL_BIN/gdal_translate" -q -b 1 "querydata:\"$SAMPLE\":0:0" "$OUTPUT_DIR/temperature.tif"
TIF_INFO=$("$GDAL_BIN/gdalinfo" -stats "$OUTPUT_DIR/temperature.tif")
echo "$TIF_INFO" | grep -aE "^(Size|Driver|Band 1 )" | head -3

echo
echo "== Sub-parameter unpacking (wind speed from packed TotalWind) =="
WIND_INFO=$("$GDAL_BIN/gdalinfo" -stats "querydata:\"$SAMPLE\":2:0:21")
WIND_STATS=$(echo "$WIND_INFO" | grep -aE "  Min" | head -1)
echo "$WIND_STATS"
case "$WIND_STATS" in *"Minimum=0"*"Maximum=15"*) ;;
  *) echo "FAIL: wind speed stats not in 0..15 m/s range"; exit 1 ;;
esac

echo
echo "== Multi-Dimensional Raster facade (xarray/Zarr/NetCDF target) =="
MDR=$("$GDAL_BIN/gdalmdiminfo" -array Lampotila -stats "$SAMPLE")
echo "$MDR" | grep -aE "(\"min\"|\"max\"|\"valid_sample_count\")"

echo
echo "== CreateCopy: round-trip .sqd → .sqd via gdal_translate =="
"$GDAL_BIN/gdal_translate" -q -of querydata "querydata:\"$SAMPLE\":0:0" \
  "$OUTPUT_DIR/roundtrip.sqd"
RT_INFO=$("$GDAL_BIN/gdalinfo" -stats "$OUTPUT_DIR/roundtrip.sqd")
echo "$RT_INFO" | grep -aE "^(Driver|Size is|Pixel Size|^  Min)" | head -4

echo
echo "All smoke checks passed."
