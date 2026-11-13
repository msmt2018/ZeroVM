# 36. Upstream source checkout and license governance

## Goal

The repository has an executable source-intake manifest. Selected sources are checked out into the local ZeroVM workspace by the official Android repo tool. The manifest and sync script are tracked; downloaded upstream worktrees are not copied into ZeroVM source blobs.

## Selected source groups

| Source | Local checkout | Main use |
|---|---|---|
| AOSP platform/system/core | third_party/aosp/system/core | libsparse, liblp, fs_mgr, image and boot primitives |
| AOSP external/avb | third_party/aosp/external/avb | libavb and AVB metadata |
| AOSP libbase/logging/libfmq/libvintf | third_party/aosp/system/* | Native base, logging, IPC and VINTF |
| AOSP SELinux + system/sepolicy | third_party/aosp/external/selinux and system/sepolicy | Guest policy/security reference |
| AOSP AVF | third_party/aosp/packages/modules/Virtualization | Platform VM API/service reference |
| Android Common Kernel | third_party/aosp/kernel/common | Guest ARM64 kernel baseline |
| Cuttlefish | third_party/aosp/device/google/cuttlefish | Complete virtual-device reference |
| minigbm/libdrm/gfxstream | third_party/aosp/external/* and hardware/google/gfxstream | Graphics source reference |
| crosvm | third_party/aosp/external/crosvm | VMM and VirtIO device source |
| e2fsprogs/f2fs-tools/erofs-utils | third_party/aosp/external/* | Guest filesystem tooling |
| BoringSSL/zstd/lz4/libxml2/protobuf/googletest | third_party/aosp/external/* | Support libraries and tests |

The manifest is selective and does not sync the entire AOSP tree. Add projects only when a concrete build or source dependency is identified.

## Sync and lock workflow

Run:

    bash third_party/scripts/sync_sources.sh

The script requires Git, Python 3 and the official Android repo launcher. It creates a persistent local manifest Git repository from the tracked XML, initializes third_party/aosp, syncs selected Android Git projects and the pinned GitHub crosvm commit, then exports a fully resolved Repo manifest and JSON source lock.

The selected Android release baseline is android16-qpr1-release, matching the android-latest-release manifest at the time this baseline was prepared. Android Common Kernel is selected from the Android 16 / 6.12 maintenance line. crosvm is pinned to immutable commit 9b63ea5f62f1493b5d2bb1c0da54f090d5e25ebe. The generated SOURCE_LOCK.xml records exact resolved SHAs for all checked-out projects. Review and commit both generated lock files when updating the baseline.

The default checkout is shallow. Set ZEROVM_SYNC_DEPTH=0 for full history and ZEROVM_SYNC_JOBS to control parallelism.

## License and provenance rules

- Preserve upstream LICENSE, NOTICE, COPYING and module-level license files.
- Review file-level and transitive licenses before copying or redistributing any code or binary.
- Track local modifications as narrow patch files with upstream commit, rationale and tests.
- Generate an SBOM/license inventory from resolved revisions before distributing binaries.
- AOSP repositories may contain mixed-license subdirectories; repository-level metadata is not enough.
- Preserve crosvm's upstream license and Cargo dependency metadata.

## Capability and compatibility boundaries

- AVF source is not an authorization mechanism. A regular APK is not assumed to have MANAGE_VIRTUAL_MACHINE, KVM access or privileged service access.
- The selected Kernel branch is a source baseline only. A usable Guest Kernel requires matching config, modules, DTB, boot artifacts, VINTF and Guest Profile evidence.
- Cuttlefish is a reference virtual platform. Its Vendor image and HAL set are not generic Vendor for arbitrary OEM ROMs.
- minigbm, libdrm and gfxstream do not independently provide Android Guest graphics acceleration.
- Downloaded source status is SOURCE_CHECKED_OUT, not INTEGRATED, BUILT, TESTED or DEVICE_VALIDATED.

## Verification status

The manifest and synchronization script are repository artifacts. This GitHub editing session cannot execute repo sync or download large upstream worktrees into the user's local checkout. Actual checkout completion is pending the user's build environment running the script. The script emits resolved source locks to make the result auditable.
