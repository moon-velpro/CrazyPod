#!/usr/bin/env python3

from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
PHOTOS = ROOT / "apps/crazypod/crazypod_photos.c"
CACHE = ROOT / "apps/crazypod/photos/crazypod_photo_cache.c"
SYSTEM_PROMPTS = (
    ROOT / "apps/crazypod/ui/shell/crazypod_system_prompts.c"
)


def function_body(source: str, name: str) -> str:
    match = re.search(
        rf"\b{name}\s*\([^)]*\)\s*\{{(?P<body>.*?)^\}}",
        source,
        re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise SystemExit(f"missing function: {name}")
    return match.group("body")


photos_source = PHOTOS.read_text(encoding="utf-8")
cache_source = CACHE.read_text(encoding="utf-8")
prompts_source = SYSTEM_PROMPTS.read_text(encoding="utf-8")

invalidate_body = function_body(
    photos_source, "crazypod_photos_invalidate_catalog"
)
if "crazypod_photo_catalog_invalidate();" not in invalidate_body:
    raise SystemExit("photo catalog invalidation no longer refreshes the catalog")
if "crazypod_photo_cache_invalidate" in invalidate_body:
    raise SystemExit("USB catalog invalidation must preserve decoded photo caches")
# Simulated/legacy Photos remains a source-validated cache implementation.
# The current music-only iPod firmware excludes this engine completely,
# so its USB lifecycle must not reference it or touch its old catalog.
manifest = (ROOT / "apps/SOURCES").read_text(encoding="utf-8")
retired_guard = manifest.split(
    "/* Retired Notes/Photos/Organizer/Customize/MiniApps compatibility:", 1
)
if len(retired_guard) == 2:
    guarded_sources = retired_guard[1].split("#endif", 1)[0]
    if "crazypod/crazypod_photos.c" not in guarded_sources:
        raise SystemExit("retired Photos source escaped simulator-only guard")
    if "crazypod_photos_invalidate_catalog();" in prompts_source:
        raise SystemExit("music-only USB flow must not load retired Photos")
    expected_result = (
        "Photo cache identity checks pass; music-only USB skips Photos"
    )
else:
    if "crazypod_photos_invalidate_catalog();" not in prompts_source:
        raise SystemExit("legacy USB flow no longer invalidates photo catalog")
    expected_result = "Photo USB refresh preserves source-validated decoded caches"

load_body = function_body(cache_source, "crazypod_photo_cache_load")
for identity_check in (
    "entry->key != key",
    "entry->source_size != source_size",
    "entry->source_mtime != source_mtime",
):
    if identity_check not in load_body:
        raise SystemExit(
            "retained photo caches require source identity check: "
            f"{identity_check}"
        )

print(expected_result)
