#!/usr/bin/env python3
"""Regression guard for the dedicated lock-screen font warm worker.

This source invariant does not replace hardware stack-watermark measurement
or a first-scan stress test.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "apps/crazypod/ui/app/crazypod_playback.c").read_text(
    encoding="utf-8"
)


def check() -> None:
    assert (
        "#define LOCK_METADATA_WARM_STACK_SIZE "
        "(DEFAULT_STACK_SIZE + 0x3000)"
    ) in SOURCE, "lock-screen font warm worker stack reserve decreased"

    worker_start = SOURCE.index("static void lock_metadata_warm_thread(void)")
    worker_end = SOURCE.index("static void playback_command_thread(void)")
    worker = SOURCE[worker_start:worker_end]

    # Dedicated worker owns both large snapshots; never allocate them in
    # its stack frame during first CJK glyph load.
    assert "static struct crazypod_lock_playback_cache cached;" in worker
    assert "static struct crazypod_track track;" in worker
    assert "cached = lock_playback;" in worker
    assert "copy_track_at_path(path, &track)" in worker
    assert "crazypod_runtime_font_prewarm_text(" in worker
    assert "mutex_lock(&lock_playback_mutex);" in worker


if __name__ == "__main__":
    check()
    print("Lock-screen font worker stack invariants passed")
