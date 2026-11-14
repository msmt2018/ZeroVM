# ZeroDroid upstream source matrix

The executable manifest is `third_party/manifest/zerodroid.xml`. It contains **73 upstream projects** after removing all emulator-only source trees. The default sync selects the core hardware-virtualization, Android Guest platform, kernel, VMM candidate, virtual-device, boot and image pipeline. Optional groups include ROM format references, mainline Linux comparison, UEFI/Bluetooth references and CTS/VTS.

## Explicit exclusions

The following are deliberately not checked out or used as runtime backends:

- QEMU (including TCG CPU emulation)
- Android Emulator
- Goldfish kernel/device models and Goldfish OpenGL
- AEMU
- Cuttlefish emulator/device images

ZeroDroid does not run a software CPU emulator and then boot Android images inside it. Guest ARM64 code must execute on the host CPU through hardware-assisted virtualization. A host without a usable AVF or authorized KVM/pKVM path is unsupported for full VM execution; there is no QEMU-style fallback.

## Required low-level virtualization and Guest sources

| Layer | Upstream source | Intended use |
|---|---|---|
| Android virtualization framework | AOSP `packages/modules/Virtualization` | AVF API/service integration, Microdroid and protected-VM lifecycle/security reference |
| Android Guest kernel | AOSP `kernel/common` (Android Common Kernel) | ARM64 Guest kernel, KVM/pKVM interfaces, VirtIO drivers and Android kernel configuration |
| Kernel build | AOSP `kernel/build` | Kleaf/Bazel kernel build definitions and reproducible kernel configuration |
| VMM | AOSP `external/crosvm` and upstream `google/crosvm` | Candidate Rust VMM for vCPU, guest memory, virtual interrupt/device model and VirtIO; not a Hypervisor and not a CPU emulator |
| Host/Guest kernel interfaces | AOSP `system/core`, `system/libbase`, `libbinder`, `libvintf`, `sepolicy` | Android service integration, IPC, policy, Guest boot/runtime support |
| Guest HAL contracts | AOSP `hardware/interfaces`, `hardware/libhardware`, `system/libvintf` | Guest HAL/VINTF compatibility |
| Guest Framework | AOSP `frameworks/base`, `frameworks/native`, `frameworks/av` | Independent Guest Android userspace and system services |
| Virtual device transport | Linux/Android Kernel VirtIO implementations in `kernel/common` | Block, net, console, vsock, input and other paravirtual devices |
| Boot and firmware material | AOSP `system/tools/mkbootimg`, `system/libufdt`, `external/dtc`, AVB | Kernel/ramdisk/DTB parsing, boot image handling and verified boot metadata |
| GSI and partition stack | AOSP `system/gsid`, `system/extras`, `system/core`, `external/avb` | Dynamic partitions, sparse images, AVB, GSI and storage preparation |

## ROM and image ingestion sources

| Stage / format | Source | Purpose |
|---|---|---|
| ZIP/OTA archive | AOSP libziparchive/libarchive, zlib, bzip2, xz, Brotli, zstd, lz4 | Safe archive listing/extraction and codecs |
| OTA payload | AOSP `system/update_engine`, `ssut/payload-dumper-go` | Payload metadata, operation semantics, full/incremental extraction and verification |
| Legacy block OTA | `xpirt/sdat2img`, `xpirt/img2sdat`, `erfanoabdi/imgpatchtools` | transfer-list, dat and block-patch handling |
| Dynamic partitions | AOSP `system/extras/partition_tools` | lpdump/lpunpack/lpmake and LP metadata |
| Sparse images | AOSP `system/core/libsparse` | simg2img/img2simg and sparse validation |
| Filesystems | AOSP e2fsprogs, erofs-utils, f2fs-tools | ext4/EROFS/F2FS inspect, extract and build |
| Boot images | AOSP `system/tools/mkbootimg`, `cfig/Android_boot_image_editor` | boot/vendor_boot/init_boot/recovery parsing and repacking |
| Device tree | AOSP `system/libufdt`, `external/dtc` | DTB/DTBO/DTS processing |
| Optional ROM references | TIK, RomTools, Firmware_extractor, Magisk, AnyKernel3, android-simg2img | Format compatibility study and test oracle only; not production runtime dependencies |

## Other platform references

- `platform/system/media` and `external/tinyalsa`: Guest audio formats and low-level audio interfaces; actual audio needs a working Guest HAL and Host bridge.
- `external/minigbm`, `external/libdrm`, `hardware/google/gfxstream`, `external/angle`, `external/swiftshader`, `external/virglrenderer`, `external/mesa3d`, `external/vulkan-headers`: graphics API/driver references. These do not imply a working accelerated GPU path.
- `platform/packages/modules/Bluetooth`: Guest Bluetooth stack reference; requires a separately implemented virtual controller or authorized Host bridge.
- `torvalds/linux`: optional mainline Linux comparison tree; Android Common Kernel remains the Guest kernel baseline.
- `tianocore/edk2`: optional UEFI firmware reference. Direct Kernel loading does not require generic UEFI firmware.
- CTS/VTS: optional compatibility validation source trees.

## Source groups and checkout

Run `bash third_party/scripts/sync_sources.sh` to fetch every project in the manifest with full Git history by default. Optional group filtering and shallow history are deliberate overrides only. The script refuses to generate a complete source lock if the expected project count or resolved revisions do not match.

```bash
ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test \
  bash third_party/scripts/sync_sources.sh
```

Source checkout is not integration, build success or device validation. Keep `SOURCE_LOCK.xml` and `SOURCE_LOCK.json` tied to the actual checked-out revisions. Every source must pass the Android ARM64 host portability review before production use; see [porting and adaptation policy](../docs/37-android-arm64-host-porting-and-adaptation.md). Do not assume that an upstream Linux/x86_64 build or generic AArch64 build is Android-compatible.

## Image safety requirements

- Reject archive traversal, absolute paths, symlink escapes, duplicate critical entries, decompression bombs and oversized files.
- Treat payload.bin as an operation stream; incremental OTA requires matching base partitions and output hash verification.
- Parse LP metadata and logical partition groups instead of treating super.img as a raw filesystem.
- Convert Android sparse images before filesystem inspection.
- Preserve filesystem metadata, ownership, capabilities and SELinux labels when rebuilding.
- Preserve boot header, page size, cmdline, ramdisk fragments, DTB/DTBO and bootconfig; verify AVB after repacking.
- Never execute ROM-supplied updater scripts or installer binaries.
- Write transformed artifacts only into bounded Guest workspaces, never Host physical partitions.
