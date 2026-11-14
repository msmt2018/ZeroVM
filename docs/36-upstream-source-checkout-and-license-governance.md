# 36. Upstream source checkout and license governance

## Goal

ZeroDroid maintains an executable, curated upstream source manifest. The official Android `repo` tool checks selected AOSP repositories into a local workspace. The manifest and sync tooling are tracked in Git; large upstream worktrees are not copied into ZeroDroid-owned source modules.

## Source coverage

| Source family | Local checkout examples | Primary purpose |
|---|---|---|
| AOSP core/runtime | `system/core`, `system/extras`, `libbase`, `libutils`, `libcutils`, `libprocessgroup` | init-adjacent utilities, fs_mgr, libsparse, liblp, resource and native foundations |
| GSI/DSU | `system/gsid` | GSI installation and Dynamic System Update service reference |
| Boot/ROM/OTA | `bootable/recovery`, `system/update_engine`, `system/tools/mkbootimg`, AVB, e2fsprogs, f2fs-tools, erofs-utils | boot images, recovery, payload OTA, dynamic partitions and filesystems |
| Security/IPC/HAL | SELinux, system/sepolicy, Binder, HIDL, FMQ, VINTF, hardware/interfaces, libhardware | Guest policy, service IPC, HAL and compatibility contracts |
| Android Guest platform | frameworks/base, frameworks/native, frameworks/av, ADB | Guest Framework, native services, media and debug bridge references |
| AVF / VM | packages/modules/Virtualization, crosvm | AVF, Microdroid, pVM lifecycle, VMM and VirtIO device model references |
| Android kernel build | kernel/build | Kleaf/Bazel build infrastructure for Android Common Kernel |
| Android Guest kernel | kernel/common | Primary Android Common Kernel source including Android KVM/pKVM and VirtIO changes |
| Emulator references | kernel/goldfish, device/generic/goldfish*, tools/emulator, external/qemu, hardware/google/aemu | Official Android Emulator, QEMU, goldfish devices and emulator utility source |
| Virtual device reference | device/google/cuttlefish | Complete Cuttlefish virtual-device configuration and host orchestration |
| Graphics | minigbm, libdrm, gfxstream, ANGLE, SwiftShader, virglrenderer | Virtual GPU, graphics translation and software rendering reference |
| Support | BoringSSL, zstd, lz4, libxml2, protobuf, googletest | Crypto, compression, serialization, XML and test support |
| Android CTS/VTS (optional) | `platform/cts`, `platform/test/vts` | Compatibility, HAL, kernel and system validation suites |
| Mainline Linux (optional) | torvalds/linux | Compare generic upstream ARM64 KVM, VirtIO and driver changes |

The executable source list and per-project revisions are defined in `third_party/manifest/zerodroid.xml`. The default checkout group contains relevant Android platform, VM, kernel, image, emulator and graphics projects. The upstream mainline Linux tree is deliberately optional because it is very large and Android Common Kernel is the primary Guest kernel.

## Sync and source locks

Install Git, Python 3 and the official Android `repo` launcher, then run from the repository root:

    bash third_party/scripts/sync_sources.sh

To also fetch the upstream mainline Linux reference tree:

    ZERODROID_SYNC_GROUPS=default,zerodroid-optional,zerodroid-test bash third_party/scripts/sync_sources.sh

Controls:

- `ZERODROID_SYNC_JOBS`: parallel project sync jobs (default 4).
- `ZERODROID_SYNC_DEPTH`: shallow depth (default 1); set to 0 for full history.
- `ZERODROID_SYNC_GROUPS`: repo groups (default `default`); add `zerodroid-optional` for upstream Linux and `zerodroid-test` for CTS/VTS.
- Successful sync emits `third_party/SOURCE_LOCK.xml` and `third_party/SOURCE_LOCK.json` with resolved revisions.

The selected Android platform baseline is `android16-qpr1-release`. Android Common Kernel uses `android16-6.12-lts`; emulator-only projects use their own maintained branches where needed. Upstream crosvm is pinned to immutable commit `9b63ea5f62f1493b5d2bb1c0da54f090d5e25ebe`. The AOSP crosvm mirror is also checked out for Android-specific integration context.

## Correct architectural interpretation

- Android Common Kernel is the primary Android Guest kernel. It contains Android-specific patches, KMI expectations and virtualization/driver work; generic upstream Linux is comparison material, not a replacement.
- KVM/pKVM are hypervisor technologies. crosvm is a VMM. VirtIO is a device/transport protocol family. Guest VirtIO drivers and host device backends must implement matching features, queues, memory sharing, interrupts and lifecycle semantics.
- AVF, KVM and pKVM source availability does not grant a normal APK `/dev/kvm`, `MANAGE_VIRTUAL_MACHINE`, hypervisor access, protected VM firmware access or privileged system service permissions.
- Android Emulator's QEMU/goldfish/AEMU stack is a valuable source-level reference and test target. It is not automatically the production backend for an ARM64 Android phone.
- Cuttlefish is a complete reference virtual device with its own kernel, Vendor and host environment. Its Vendor image is not universal for arbitrary OEM ROMs.
- GSI is only the Android System image. A compatible image set must account for boot, vendor_boot, init_boot, vbmeta, super/dynamic partitions, kernel, DTB, firmware, Vendor HAL and VINTF.
- Graphics source checkout does not prove accelerated rendering. Host GPU, Guest driver, virtio-gpu/gfxstream protocol, memory sharing, sync fences and compositor integration all need end-to-end validation.
- ROM ingestion and image transformation are not host flashing. Never write guest images to the physical host partitions as part of import/analysis.

## Excluded from bulk source sync

The manifest does not mirror the whole Android Studio/IntelliJ IDE, every AOSP repository, prebuilt SDK/Emulator binary archives, proprietary Vendor blobs, device firmware, secure-world binaries, KeyMint/TEE secrets, DRM keys, Google Play closed-source components or signing keys. These are unrelated to the core Guest runtime, are device/license restricted, or cannot be redistributed as generic source dependencies. The Android Emulator source is included separately because its QEMU/goldfish/AEMU implementation is useful for reference and testing.

## License, provenance and status

- Preserve upstream LICENSE, NOTICE, COPYING and module-level license files.
- Review file-level and transitive licenses before copying or redistributing code or binaries.
- Keep upstream projects in separate worktrees; track local modifications as narrow patches with source commit, rationale and tests.
- Generate an SBOM/license inventory from resolved revisions before distributing binaries.
- AOSP repositories can contain mixed-license subdirectories; repository-level license metadata is insufficient.
- Downloaded source status is `SOURCE_CHECKED_OUT`, not `INTEGRATED`, `BUILT`, `TESTED` or `DEVICE_VALIDATED`.

## Verification status

The manifest and synchronization script are committed project artifacts. GitHub repository editing cannot execute `repo sync` or transfer the multi-gigabyte upstream worktrees into the user's local checkout. Actual source download is pending execution in a machine with network access, Git, Python 3 and the official repo launcher. The generated source lock makes that checkout auditable.
