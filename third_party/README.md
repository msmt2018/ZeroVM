# ZeroDroid third-party source workspace

Install Git, Python 3 and the official Android repo launcher, then run from the repository root:

    bash third_party/scripts/sync_sources.sh

The script checks out selected AOSP projects into third_party/aosp, including system/core (libsparse, liblp and fs_mgr), AVB, SELinux, VINTF, AVF, Cuttlefish reference sources, Android Common Kernel, minigbm, libdrm, gfxstream, crosvm, filesystem tools and support libraries.

The AOSP baseline is android16-qpr1-release, selected from the android-latest-release manifest at the time this baseline was prepared. crosvm is pinned to an immutable commit. The first sync is network- and disk-intensive, especially for the Guest Kernel and graphics sources.

Set ZEROVM_SYNC_JOBS to tune concurrency. The default is a shallow checkout; set ZEROVM_SYNC_DEPTH=0 for full Git history. After a successful sync, SOURCE_LOCK.xml and SOURCE_LOCK.json contain the resolved revisions. Review and commit those generated lock files when accepting an upstream baseline.

Downloaded source is not automatically integrated, built or validated. Keep each upstream's native build system and license files. Cuttlefish is a reference platform, not a universal Vendor. AVF source does not grant ordinary APKs privileged permissions. See docs/36-upstream-source-checkout-and-license-governance.md.
