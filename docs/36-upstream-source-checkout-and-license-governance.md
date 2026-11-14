# 36. Upstream source checkout and license governance

## Goal

ZeroDroid maintains an executable, curated upstream source manifest. The official Android `repo` tool checks selected AOSP repositories into a local workspace. The manifest and sync tooling are tracked in Git; large upstream worktrees are not copied into ZeroDroid-owned source modules.

## Complete ROM and image pipeline source set

The default group includes the normative AOSP image tools plus focused independent implementations. This is necessary because ZeroDroid must inspect and transform factory ROM archives, OTA ZIPs, payload streams, logical partitions, sparse images, filesystem images and boot containers.

| Stage | Canonical AOSP source | Independent / supplementary source | Required operations |
|---|---|---|---|
| ZIP / archive intake | system/core/libziparchive, external/libarchive | zlib, bzip2, xz, Brotli, zstd, lz4 | List entries, bounded extraction, compression decoding and path safety |
| Modern payload OTA | system/update_engine | ssut/payload-dumper-go | Parse CrAU metadata, operation streams, full and incremental OTA, extract partition images and verify hashes |
| Legacy block OTA | system/update_engine / recovery code | xpirt/sdat2img, xpirt/img2sdat, erfanoabdi/imgpatchtools | .dat.br decode, transfer-list reconstruction, block-level and file-level patch apply/verify |
| Dynamic partitions | system/extras/partition_tools (lpdump/lpunpack/lpmake) | optional lpunpack implementations for cross-checking | Read LP metadata, slot/group/extents, extract logical partitions and rebuild super.img |
| Sparse image | system/core/libsparse (simg2img/img2simg/append2simg) | anestisb/android-simg2img reference | Validate sparse chunks/CRC and convert sparse to raw and raw to sparse |
| ext4 | external/e2fsprogs, system/core ext4 utilities | — | Inspect, extract, validate, create and resize ext4 images |
| EROFS | external/erofs-utils | — | Inspect, extract, validate and create EROFS images |
| F2FS | external/f2fs-tools | — | Inspect, dump, validate and create F2FS images |
| Boot containers | system/tools/mkbootimg (mkbootimg/unpack_bootimg) | cfig/Android_boot_image_editor | Parse/repack boot, vendor_boot, init_boot, recovery, kernel, ramdisk, vendor ramdisk fragments and bootconfig |
| AVB / vbmeta | external/avb | cfig/Android_boot_image_editor as a secondary parser | Parse descriptors, verify hash/hashtree metadata and preserve signing relationships |
| DTB / DTBO | system/libufdt and AOSP device-tree tooling | — | Parse flattened device tree and device-tree overlays |
| OEM firmware | — | erfanoabdi/Firmware_extractor (optional) | Reference for vendor-specific container extraction; never assume universal format support |
| End-to-end ROM suites | AOSP tools remain normative | ColdWindScholar/TIK and Danda420/RomTools (optional) | Compare format detection and unpack/repack coverage; audit licenses before integration |
| Boot ramdisk / kernel package | AOSP boot tooling | Magisk and AnyKernel3 (optional) | Study boot patch and kernel installer package workflows; never auto-execute imported scripts |

AOSP `system/extras/partition_tools` is the source of `lpdump`, `lpunpack` and `lpmake`; AOSP `system/core/libsparse` provides `simg2img` and `img2simg`; `system/tools/mkbootimg` includes `unpack_bootimg.py` and repack support. `system/update_engine` is the authoritative implementation reference for OTA payload semantics. Independent tools are additional implementations and compatibility oracles, not replacements for the platform specifications.

## Other source families

| Source family | Local checkout examples | Primary purpose |
|---|---|---|
| AOSP core/runtime | system/core, system/extras, libbase, libutils, libcutils, libprocessgroup | init-adjacent utilities, fs_mgr, libsparse, liblp, resource and native foundations |
| Security/IPC/HAL | SELinux, system/sepolicy, Binder, HIDL, FMQ, VINTF, hardware/interfaces, libhardware | Guest policy, service IPC, HAL and compatibility contracts |
| Android Guest platform | frameworks/base, frameworks/native, frameworks/av, ADB | Guest Framework, native services, media and debug bridge references |
| AVF / VM | packages/modules/Virtualization, crosvm | AVF, Microdroid, pVM lifecycle, VMM and VirtIO device model references |
| Android Guest kernel | kernel/common, kernel/build | Primary Android Guest kernel including KVM/pKVM/VirtIO and Kleaf build infrastructure |
| Emulator references | kernel/goldfish, device/generic/goldfish*, tools/emulator, external/qemu, hardware/google/aemu | Official Android Emulator, QEMU, goldfish devices and emulator utility source |
| Virtual device reference | device/google/cuttlefish | Complete Cuttlefish virtual-device configuration and host orchestration |
| Graphics | minigbm, libdrm, gfxstream, ANGLE, SwiftShader, virglrenderer | Virtual GPU, graphics translation and software rendering reference |
| Mainline Linux (optional) | torvalds/linux | Compare generic upstream ARM64 KVM, VirtIO and driver changes |
| Android CTS/VTS (optional) | platform/cts, platform/test/vts | Compatibility, HAL, kernel and system validation suites |

## Source groups and full checkout

The manifest currently contains 71 project entries:

- `default`: 62 projects, including the core ROM/image pipeline and primary Android platform/runtime sources.
- `zerodroid-rom-reference`: 6 optional references (TIK, RomTools, standalone sparse converter, OEM firmware extractor, Magisk and AnyKernel3).
- `zerodroid-optional`: 1 upstream mainline Linux reference tree.
- `zerodroid-test`: 2 large CTS/VTS test trees.

Install Git, Python 3 and the official Android `repo` launcher, then run from the repository root to download all 71 projects:

    ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test bash third_party/scripts/sync_sources.sh

Controls:

- `ZERODROID_SYNC_JOBS`: parallel project sync jobs (default 4).
- `ZERODROID_SYNC_DEPTH`: shallow depth (default 1); set to 0 for full history.
- `ZERODROID_SYNC_GROUPS`: repo groups (default `default`).
- Successful sync emits `third_party/SOURCE_LOCK.xml` and `third_party/SOURCE_LOCK.json` with resolved revisions.

## Image processing safety and correctness

- ROM ZIP is an archive, not one image. Validate archive entry paths and sizes before extraction; reject absolute paths, traversal, escaping symlinks, duplicate critical entries, decompression bombs and unexpected oversized files.
- A/B `payload.bin` is an operation stream, not a ZIP of partition images. Incremental payloads require the correct base images and supported operations. Validate metadata/signatures and verify resulting partition hashes.
- Legacy `.new.dat.br`, transfer-list and `.patch.dat` are a separate OTA format. Never treat arbitrary `.dat` files as raw images.
- `super.img` contains LP metadata and extents. Inspect slot, groups and partition metadata; extract with lpunpack and rebuild only with a compatible layout using lpmake.
- Android sparse images are not filesystem images. Convert sparse to raw before filesystem inspection; convert raw to sparse only when required by the target format.
- ext4, EROFS and F2FS are different filesystem formats. Use format-aware readers/writers and preserve fs_config, file_contexts, capabilities, ownership, timestamps and SELinux labels.
- Boot containers are versioned structures. Preserve header version, page size, kernel, ramdisk, vendor ramdisk fragments, DTB/DTBO, cmdline and bootconfig. Repacking may invalidate vbmeta/AVB descriptors; verify the complete matched image set.
- Never execute updater-script, shell scripts, installer ZIP scripts or ROM-supplied binaries during import. Parse them as untrusted input in an isolated sandbox.
- Image conversion and repacking must only write guest artifacts inside a bounded workspace, never physical host partitions.

## Architectural and license boundaries

- Android Common Kernel is the primary Android Guest kernel; upstream Linux is comparison material.
- KVM/pKVM are hypervisor technologies; crosvm is a VMM; VirtIO is a device/transport protocol.
- AVF/KVM source availability does not grant a normal APK privileged VM permissions or /dev/kvm access.
- Android Emulator/QEMU and Cuttlefish are references, not universal production phone backends or Vendor images.
- GSI is only the System image. Boot, vendor_boot, init_boot, vbmeta, super, kernel, Vendor HAL, VINTF and firmware must form a compatible image set.
- Keep upstream projects separate, preserve LICENSE/NOTICE/COPYING and transitive dependency metadata, and carry local changes as narrow auditable patches.
- Third-party ROM tools may have different licenses and security posture. Do not link them into production runtime without license, maintenance, dependency and security review.
- Do not mirror proprietary Vendor blobs, firmware, secure-world binaries, DRM keys, KeyMint/TEE secrets or device-specific signing material into this public repository.

## Download status

The manifest and sync tooling are committed artifacts. The 71 upstream worktrees have not yet been downloaded by GitHub repository editing. Execute the command above in a networked development environment with Git, Python 3 and the official repo launcher. A successful sync generates exact resolved revisions in SOURCE_LOCK files. Downloaded source status is `SOURCE_CHECKED_OUT`, not `INTEGRATED`, `BUILT`, `TESTED` or `DEVICE_VALIDATED`.
