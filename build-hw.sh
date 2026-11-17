#!/bin/sh
set -eu

cd "$(dirname "$0")"

detect_jobs() {
    if [ -n "${JOBS:-}" ]; then
        echo "$JOBS"
    elif command -v getconf >/dev/null 2>&1; then
        getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu 2>/dev/null || echo 4
    else
        echo 4
    fi
}

require_tools() {
    missing=0
    for tool in make perl python3 zip gcc "${CROSS_COMPILE}gcc" \
        "${CROSS_COMPILE}objcopy" "${CROSS_COMPILE}nm"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            echo "Error: missing required tool '$tool' on PATH." >&2
            missing=1
        fi
    done
    [ "$missing" -eq 0 ] || exit 2
}

prepare_generated_headers() {
    builddir_unix=$(pwd)
    # rbversion.h is phony when the UTC date changes. Build it separately so
    # GNU Make 3.81 cannot schedule generated files twice through other goals.
    make EXTRA_DEFINES="$CRAZYPOD_BUILD_DEFINES" \
        -j1 "$builddir_unix/rbversion.h"
    make EXTRA_DEFINES="$CRAZYPOD_BUILD_DEFINES" \
        -j1 "$builddir_unix/apps/core_asmdefs.h"
    make EXTRA_DEFINES="$CRAZYPOD_BUILD_DEFINES" \
        -j1 "$builddir_unix/ram.link"
}

verify_stack_alignment() {
    for symbol in stackbegin _stackbegin stackend _stackend \
        _irqstackbegin _irqstackend _fiqstackbegin _fiqstackend; do
        address=$("${CROSS_COMPILE}nm" -n rockbox.elf |
            awk -v target="$symbol" '$3 == target { print $1; exit }')
        if [ -z "$address" ]; then
            echo "Error: missing stack symbol '$symbol' in rockbox.elf." >&2
            exit 1
        fi
        if [ $((0x$address % 8)) -ne 0 ]; then
            echo "Error: stack symbol '$symbol' is not 8-byte aligned: 0x$address." >&2
            exit 1
        fi
    done
}

verify_removed_runtime_absent() {
    forbidden='quickjs|mquickjs|crazypod_js|crazypod_script|solid_renderer|ui_command_batch'
    for binary in rockbox.elf; do
        matches=$("${CROSS_COMPILE}nm" -a "$binary" 2>/dev/null |
            awk '{ print $3 }' |
            grep -E -i "$forbidden" || true)
        if [ -n "$matches" ]; then
            echo "Error: removed script runtime symbol in $binary:" >&2
            echo "$matches" >&2
            exit 1
        fi
    done
}

# Fail closed if a future source-list change links removed MPEG/video code
# into the music-only iPod. These symbol families belong only to the retired
# player and must be absent from rockbox.elf, not merely unused in the UI.
verify_retired_video_absent() {
    matches=$("${CROSS_COMPILE}nm" -a rockbox.elf 2>/dev/null |
        awk '{ print $3 }' |
        grep -E 'crazypod_video_engine_|crazypod_video_catalog_|mpeg2_' || true)
    if [ -n "$matches" ]; then
        echo "Error: retired video decoder symbols in music firmware:" >&2
        echo "$matches" >&2
        exit 1
    fi
}

INCREMENTAL=0
case "${CRAZYPOD_INCREMENTAL:-}" in
    1|yes|true|YES|TRUE) INCREMENTAL=1 ;;
esac

while [ $# -gt 0 ]; do
    case "$1" in
        -i|--incremental)
            INCREMENTAL=1
            ;;
        -h|--help)
            cat <<'EOF'
Usage: build-hw.sh [-i|--incremental]

Builds CrazyPod exclusively for the Rockbox iPod 6G target.

Environment:
  CRAZYPOD_INCREMENTAL=1  reuse the selected variant build directory
  CRAZYPOD_SKIP_DEP=1     skip make dep when make.dep exists
  CRAZYPOD_REPRO_DIAGNOSTICS=1
                           build the one-shot harness in build-hw-ipod6g-repro/
  CRAZYPOD_IAP_DIAGNOSTICS=1
                           capture raw iAP frames in build-hw-ipod6g-iap/
  CROSS_COMPILE=prefix-   default arm-none-eabi-
  JOBS=N                  parallel job count
EOF
            exit 0
            ;;
        *)
            echo "Error: CrazyPod supports only iPod 6G; unsupported argument '$1'." >&2
            exit 2
            ;;
    esac
    shift
done

CROSS_COMPILE="${CROSS_COMPILE:-arm-none-eabi-}"
export CROSS_COMPILE
CRAZYPOD_BUILD_DEFINES=""
CRAZYPOD_BUILD_VARIANT="production"
repro_diagnostics="${CRAZYPOD_REPRO_DIAGNOSTICS:-}"
iap_diagnostics="${CRAZYPOD_IAP_DIAGNOSTICS:-}"
repro_enabled=0
iap_enabled=0
case "$repro_diagnostics" in
    1|yes|true|YES|TRUE) repro_enabled=1 ;;
esac
case "$iap_diagnostics" in
    1|yes|true|YES|TRUE) iap_enabled=1 ;;
esac
if [ "$repro_enabled" -eq 1 ] && [ "$iap_enabled" -eq 1 ]; then
    echo "Error: select only one CrazyPod diagnostic build variant." >&2
    exit 2
fi
case "$iap_diagnostics" in
    1|yes|true|YES|TRUE)
        CRAZYPOD_BUILD_DEFINES="-DCRAZYPOD_IAP_DIAGNOSTICS"
        CRAZYPOD_BUILD_VARIANT="iap"
        ;;
esac
if [ "$repro_enabled" -eq 1 ]; then
    case "$repro_diagnostics" in
        1|yes|true|YES|TRUE)
            CRAZYPOD_BUILD_DEFINES="-DCRAZYPOD_REPRO_DIAGNOSTICS"
            CRAZYPOD_BUILD_VARIANT="repro"
            ;;
    esac
fi
require_tools
python3 tests/test-crazypod-lvgl-layer-budget.py
# No CPKs or plugin payloads are generated for the music-only build.
if [ "$CRAZYPOD_BUILD_VARIANT" = "repro" ]; then
    BUILDDIR="build-hw-ipod6g-repro"
    STAMP="crazypod hardware ipod6g lvgl repro"
elif [ "$CRAZYPOD_BUILD_VARIANT" = "iap" ]; then
    BUILDDIR="build-hw-ipod6g-iap"
    STAMP="crazypod hardware ipod6g lvgl iap diagnostics"
else
    BUILDDIR="build-hw-ipod6g"
    STAMP="crazypod hardware ipod6g lvgl production"
fi

configure_build() {
    ../tools/configure --target=ipod6g --type=n
    printf '%s\n' "$STAMP" > .crazypod_configure_stamp
}

if [ "$INCREMENTAL" -eq 0 ]; then
    echo "CrazyPod: clean iPod 6G hardware build"
    rm -rf "$BUILDDIR"
    mkdir "$BUILDDIR"
    cd "$BUILDDIR"
    configure_build
else
    echo "CrazyPod: incremental iPod 6G hardware build"
    mkdir -p "$BUILDDIR"
    cd "$BUILDDIR"
    if [ ! -f Makefile ] ||
       [ ! -f .crazypod_configure_stamp ] ||
       [ "$(cat .crazypod_configure_stamp)" != "$STAMP" ]; then
        configure_build
    fi
fi

if [ -n "${CRAZYPOD_SKIP_DEP:-}" ] && [ -f make.dep ]; then
    echo "CrazyPod: reusing make.dep"
else
    make EXTRA_DEFINES="$CRAZYPOD_BUILD_DEFINES" dep
fi

prepare_generated_headers
make EXTRA_DEFINES="$CRAZYPOD_BUILD_DEFINES" \
    -j"$(detect_jobs)"

if [ ! -f rockbox.ipod ]; then
    echo "Error: hardware build did not produce rockbox.ipod." >&2
    exit 1
fi
verify_stack_alignment
verify_removed_runtime_absent
verify_retired_video_absent
# MiniApps/CPK packages are retired from the music-only distribution.
# The native audio firmware is still built using the existing SDK build.
PACKAGE_DIR="$(mktemp -d)"
trap 'rm -rf "$PACKAGE_DIR"' EXIT HUP INT TERM
if [ ! -d ../assets/crazypod-icons ]; then
    echo "Error: missing generated CrazyPod icon assets." >&2
    exit 1
fi
if [ ! -f ../assets/crazypod/default-home.bmp ]; then
    echo "Error: missing generated CrazyPod default wallpaper." >&2
    exit 1
fi
mkdir -p "$PACKAGE_DIR/.rockbox/codecs"
mkdir -p "$PACKAGE_DIR/.rockbox/codepages"
mkdir -p "$PACKAGE_DIR/.rockbox/fonts"
mkdir -p "$PACKAGE_DIR/.rockbox/crazypod/icons"
for content_directory in Music Podcasts; do
    mkdir -p "$PACKAGE_DIR/$content_directory"
done
CODEPAGE_TOOL="$(cd .. && pwd)/tools/codepages"
CODEPAGE_BUILD_DIR="$PACKAGE_DIR/generated-codepages"
if [ ! -x "$CODEPAGE_TOOL" ]; then
    echo "Error: missing Rockbox codepage generator '$CODEPAGE_TOOL'." >&2
    exit 1
fi
mkdir -p "$CODEPAGE_BUILD_DIR"
(
    cd "$CODEPAGE_BUILD_DIR"
    "$CODEPAGE_TOOL"
)
cp "$CODEPAGE_BUILD_DIR/936.cp" \
   "$PACKAGE_DIR/.rockbox/codepages/936.cp"
RUNTIME_FONT_BUILDER="$(cd .. && pwd)/tools/build-crazypod-runtime-fonts.sh"
if [ ! -x "$RUNTIME_FONT_BUILDER" ]; then
    echo "Error: missing CrazyPod runtime font builder." >&2
    exit 1
fi
"$RUNTIME_FONT_BUILDER" "$PACKAGE_DIR/.rockbox/fonts"
python3 ../tools/crazypod_runtime_font_audit.py \
    --font-dir "$PACKAGE_DIR/.rockbox/fonts/crazypod-aot"
cp rockbox.ipod "$PACKAGE_DIR/.rockbox/rockbox.ipod"
[ ! -f rockbox-info.txt ] || cp rockbox-info.txt "$PACKAGE_DIR/.rockbox/rockbox-info.txt"
cp -R ../assets/crazypod-icons/. \
    "$PACKAGE_DIR/.rockbox/crazypod/icons/"
cp ../assets/crazypod/default-home.bmp \
    "$PACKAGE_DIR/.rockbox/crazypod/default-home.bmp"
for codec in lib/rbcodec/codecs/*.codec; do
    [ -f "$codec" ] || continue
    case "$codec" in
        *_enc.codec) continue ;;
    esac
    cp "$codec" "$PACKAGE_DIR/.rockbox/codecs/"
done
rm -f CrazyPod-6G.zip
(
    cd "$PACKAGE_DIR"
    zip -q -r "$PACKAGE_DIR/../CrazyPod-6G.zip" \
        .rockbox Music Podcasts
)
mv "$PACKAGE_DIR/../CrazyPod-6G.zip" CrazyPod-6G.zip

echo "CrazyPod: firmware bytes $(wc -c < rockbox.ipod)"
echo "CrazyPod: built $(pwd)/rockbox.ipod"
echo "CrazyPod: packaged $(pwd)/CrazyPod-6G.zip"
