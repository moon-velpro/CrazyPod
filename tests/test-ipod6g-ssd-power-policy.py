#!/usr/bin/env python3
"""Guard against re-enabling unvalidated iPod 6G SSD power-rail removal.

This is a static source-policy check, not a physical storage-integrity test.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
source = (
    root
    / "firmware/target/arm/s5l8702/ipod6g/storage_ata-6g.c"
).read_text(encoding="utf-8")

# Neither a user-selected SSD mode nor a successful ATA cache flush proves
# that every aftermarket adapter survives removal of the AUTOLDO power rail.
assert re.search(
    r"static\s+const\s+bool\s+ata_platform_power_cut_capable"
    r"\s*=\s*false\s*;",
    source,
), "Unvalidated deep power-cut authorization must remain disabled"

# The initial check and the post-mutex recheck both need the guard; checking
# only once leaves a race if the policy ever becomes configurable.
checks = re.findall(
    r"\bif\s*\(\s*ata_ssd_mode\s*&&"
    r"\s*ata_platform_power_cut_capable\s*&&"
    r"\s*ssd_flush_succeeded\b",
    source,
)
assert len(checks) == 2, (
    f"Expected the deep-sleep authorization in both checks; found {len(checks)}"
)
assert source.count('logf("ata SSD DEEP') == 1, (
    "Review new SSD power-off paths before authorizing them"
)
assert source.count("ide_power_enable(false);") == 2, (
    "Review any new platform power-rail shutdown call sites"
)

print("iPod 6G SSD rail power-cut guard: source-policy checks passed")
