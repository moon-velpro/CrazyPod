#!/bin/sh
set -eu

cd "$(dirname "$0")"

detect_jobs() {
    if [ -n "${JOBS:-}" ]; then
        printf '%s\n' "$JOBS"
    elif command -v getconf >/dev/null 2>&1; then
        getconf _NPROCESSORS_ONLN 2>/dev/null || printf '%s\n' 4
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu 2>/dev/null || printf '%s\n' 4
    else
        printf '%s\n' 4
    fi
}

require_tools() {
    missing=0
    for tool in make gcc perl python3 sdl2-config pkg-config; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            echo "Error: missing required simulator tool '$tool' on PATH." >&2
            missing=1
        fi
    done
    [ "$missing" -eq 0 ] || exit 2
    for package in libavformat libavcodec libavutil libswscale libswresample; do
        if ! pkg-config --exists "$package"; then
            echo "Error: missing required FFmpeg package '$package'." >&2
            missing=1
        fi
    done
    [ "$missing" -eq 0 ] || exit 2
}

usage() {
    cat <<'EOF'
Usage: build-sim.sh [-i|--incremental]

Builds the CrazyPod iPod 6G simulator. It does not install Rockbox themes,
skins, fonts, or plugins.

  -i, --incremental   Reuse build-sim/ when already configured.

Environment:
  ROCKPOD_INCREMENTAL=1   same as --incremental
  ROCKPOD_SKIP_DEP=1      skip make dep when make.dep exists
  JOBS=N                  parallel job count
  CRAZYPOD_PRIVATE_ICONS_DIR=/path/to/private-icons
                           compile private source-supplied PNGs into simulator
  CRAZYPOD_PRIVATE_ICONS_ALLOW_PARTIAL=1
                           allow a mixed preview while assets are incomplete
EOF
}

incremental=0
case "${ROCKPOD_INCREMENTAL:-}" in
    1|yes|true|YES|TRUE) incremental=1 ;;
esac

while [ "$#" -gt 0 ]; do
    case "$1" in
        -i|--incremental)
            incremental=1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
    shift
done

private_icons_enabled=0
private_icon_defines=""
if [ -n "${CRAZYPOD_PRIVATE_ICONS_DIR:-}" ]; then
    private_icons_enabled=1
    if [ "${CRAZYPOD_PRIVATE_ICONS_ALLOW_PARTIAL:-}" = 1 ]; then
        python3 tools/prepare-crazypod-private-icons.py \
            --source "$CRAZYPOD_PRIVATE_ICONS_DIR" --allow-partial
    else
        python3 tools/prepare-crazypod-private-icons.py \
            --source "$CRAZYPOD_PRIVATE_ICONS_DIR"
    fi
    private_icon_defines="-DCRAZYPOD_PRIVATE_ICONS"
fi
require_tools
python3 tests/test-crazypod-lvgl-layer-budget.py
# The retired native MiniApps toolchain is not part of this firmware.
builddir="build-sim"
configure_stamp="crazypod simulator ipod6g lvgl sdl-threads"
if [ "$private_icons_enabled" -eq 1 ]; then
    builddir="build-sim-private"
    configure_stamp="$configure_stamp local-only private icons"
fi

configure_build() {
    ../tools/configure --target=ipod6g --type=s --sdl-threads
    printf '%s\n' "$configure_stamp" > .crazypod-configure
}

if [ "$incremental" -eq 0 ]; then
    echo "CrazyPod: clean iPod 6G simulator build"
    rm -rf "$builddir"
    mkdir "$builddir"
    cd "$builddir"
    configure_build
else
    echo "CrazyPod: incremental iPod 6G simulator build"
    mkdir -p "$builddir"
    cd "$builddir"
    if [ ! -f Makefile ] || [ ! -f .crazypod-configure ] ||
       [ "$(cat .crazypod-configure 2>/dev/null || true)" != "$configure_stamp" ]; then
        configure_build
    fi
fi

if [ -z "${ROCKPOD_SKIP_DEP:-}" ] || [ ! -f make.dep ]; then
    if [ "$private_icons_enabled" -eq 1 ]; then
        make EXTRA_DEFINES="$private_icon_defines" dep
    else
        # Preserve the historically passing macOS public simulator build:
        # Make 3.81 can differ when an explicit empty override is passed.
        make dep
    fi
fi

if [ "$private_icons_enabled" -eq 1 ]; then
    make EXTRA_DEFINES="$private_icon_defines" -j"$(detect_jobs)"
else
    make -j"$(detect_jobs)"
fi

codec_dir="lib/rbcodec/codecs"
sim_codec_dir="simdisk/.rockbox/codecs"
sim_font_dir="simdisk/.rockbox/fonts"
codepage_tool="$(cd .. && pwd)/tools/codepages"
runtime_font_builder="$(cd .. && pwd)/tools/build-crazypod-runtime-fonts.sh"
codepage_build_dir="$(pwd)/generated-codepages"
sim_codepage_dir="simdisk/.rockbox/codepages"
if [ ! -x "$codepage_tool" ]; then
    echo "Error: missing Rockbox codepage generator '$codepage_tool'." >&2
    exit 1
fi
mkdir -p "$codepage_build_dir" "$sim_codepage_dir"
(
    cd "$codepage_build_dir"
    "$codepage_tool"
)
cp "$codepage_build_dir/936.cp" "$sim_codepage_dir/936.cp"
if [ ! -x "$runtime_font_builder" ]; then
    echo "Error: missing CrazyPod runtime font builder." >&2
    exit 1
fi
"$runtime_font_builder" "$(pwd)/$sim_font_dir"
mkdir -p "$sim_codec_dir"
find "$sim_codec_dir" -type f -name '*.codec' -delete
for codec in "$codec_dir"/*.codec; do
    [ -f "$codec" ] || continue
    case "$codec" in
        *_enc.codec) continue ;;
    esac
    cp "$codec" "$sim_codec_dir/"
done
icon_dir="simdisk/.rockbox/crazypod/icons"
if [ ! -d ../assets/crazypod-icons ]; then
    echo "Error: missing generated CrazyPod icon assets." >&2
    exit 1
fi
if [ ! -f ../assets/crazypod/default-home.bmp ]; then
    echo "Error: missing generated CrazyPod default wallpaper." >&2
    exit 1
fi
rm -rf "$icon_dir"
mkdir -p "$icon_dir"
cp -R ../assets/crazypod-icons/. "$icon_dir/"
cp ../assets/crazypod/default-home.bmp \
   simdisk/.rockbox/crazypod/default-home.bmp
# Do not build or bundle retired CPK miniapp packages.
app_bundle="CrazyPod Simulator.app"
mkdir -p "$app_bundle/Contents/MacOS"
cp ../tools/crazypod-simulator/Info.plist "$app_bundle/Contents/Info.plist"
cp ../tools/crazypod-simulator/launch.sh \
   "$app_bundle/Contents/MacOS/CrazyPod Simulator"
chmod +x "$app_bundle/Contents/MacOS/CrazyPod Simulator"

echo "CrazyPod: simulator $(pwd)/rockboxui"
echo "CrazyPod: app bundle $(pwd)/$app_bundle"
if [ "$private_icons_enabled" -eq 1 ]; then
    echo "CrazyPod: INTERNAL ARTWORK SIMULATOR — do not redistribute bundle"
fi
