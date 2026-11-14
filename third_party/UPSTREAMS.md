# Third-party source intake and checkout plan

The executable manifest is `third_party/manifest/zerodroid.xml`. Run `bash third_party/scripts/sync_sources.sh` to download the 64 default projects into `third_party/aosp/`. The manifest contains 73 projects in total: 62 default, 6 optional ROM-suite references, 1 upstream Linux reference and 2 optional Android compatibility test trees.

## Android ROM and image pipeline source matrix

| Stage / format | Source project | What ZeroDroid can use it for |
|---|---|---|
| ROM ZIP / OTA ZIP | AOSP `system/core/libziparchive`, `external/libarchive`, AOSP zlib, bzip2, xz, Brotli, zstd, lz4 | Safe archive listing and extraction, compression codec support; ZIP metadata and path validation |
| OTA `payload.bin` | AOSP `system/update_engine` | Authoritative payload metadata, operation, partition and delta-OTA semantics; OTA extractor and payload verification reference |
| OTA `payload.bin` | `ssut/payload-dumper-go` | Standalone fast payload partition extraction, ZIP-in-place input, incremental OTA support and hash verification |
| Legacy block OTA `.dat.br` | AOSP Brotli + `xpirt/sdat2img` | Decode Brotli-compressed block data and reconstruct raw partition image from transfer list + data |
| Legacy block OTA generation | `xpirt/img2sdat` | Convert supported ext4 image into legacy Android block OTA data/transfer-list format |
| Legacy incremental patch OTA | `erfanoabdi/imgpatchtools` | Reference implementation for block image update, file patch and patch verification; never run against host block devices |
| Dynamic `super.img` | AOSP `system/extras/partition_tools` | `lpdump`, `lpunpack`, `lpmake`, `lpflash`: inspect metadata, extract logical partitions and build super images |
| Android sparse `.img` | AOSP `system/core/libsparse` | Canonical `simg2img`, `img2simg`, `append2simg`, sparse parsing and validation |
| Android sparse `.img` | `anestisb/android-simg2img` (optional reference) | Standalone portable sparse conversion implementation for comparison and test vectors |
| ext4 | AOSP e2fsprogs + system/core ext4 utilities | Inspect, extract, validate and build ext4 filesystem images |
| EROFS | AOSP erofs-utils | Inspect, extract and build EROFS images |
| F2FS | AOSP f2fs-tools | Inspect, dump, validate and build F2FS images |
| boot / vendor_boot / init_boot / recovery | AOSP `system/tools/mkbootimg` | Official `mkbootimg`, `unpack_bootimg`, repack support and boot header formats |
| boot / vendor_boot / vbmeta / payload | `cfig/Android_boot_image_editor` | Cross-check parsing and unpack/repack of boot images, AVB metadata and payload images |
| DTB / DTBO | AOSP `system/libufdt`, device-tree-compiler source/toolchain references | Flattened device tree parsing and DTBO image operations |
| OEM firmware containers | `erfanoabdi/Firmware_extractor` (optional reference) | Study vendor-specific firmware container extraction; keep device formats isolated |
| Integrated ROM suites | `ColdWindScholar/TIK`, `Danda420/RomTools` (optional references) | Compare end-to-end format detection, unpack/repack workflows and format coverage |
| Boot ramdisk patching | `topjohnwu/Magisk` (optional reference) | Study boot image/ramdisk patch workflow; not a ZeroDroid dependency |
| Kernel installer ZIP | `osm0sis/AnyKernel3` (optional reference) | Study kernel ZIP packaging and installer scripts; never execute untrusted scripts automatically |

## Other upstream source matrix

| Source project | Role in ZeroDroid | Policy |
|---|---|---|
| AOSP `system/gsid` | GSI installation and Dynamic System Update service reference | Android 16 QPR1 baseline |
| AOSP `system/core`, `system/extras` | init-adjacent utilities, fs_mgr, libsparse, liblp and image tools | Android 16 QPR1 baseline |
| AOSP `external/avb` | AVB, vbmeta parsing and verification | Never silently disable verification |
| AOSP `bootable/recovery`, `system/update_engine` | Recovery, OTA payload and update flow reference | Parse/transform images; do not flash the host |
| AOSP SELinux and `system/sepolicy` | Guest MAC policy and policy validation | Never use permissive mode as a default |
| AOSP `libvintf`, `hardware/interfaces`, `hardware/libhardware` | VINTF manifests, HAL contracts and device interfaces | Guest profile must match HAL/VINTF |
| AOSP `frameworks/base`, `frameworks/native`, `frameworks/av` | Guest Framework, native services, media and graphics contracts | Reference/integration source; large checkout |
| AOSP `packages/modules/Virtualization` | AVF, Microdroid, VM service and pVM lifecycle reference | Runtime permission/feature gate remains mandatory |
| Android Kernel build `kernel/build` | Kleaf/Bazel build infrastructure | `main-kernel` branch |
| Android Common Kernel `kernel/common` | Primary Android Guest kernel, KVM/pKVM, VirtIO and Android kernel changes | `android16-6.12-lts` baseline; match the Guest Profile |
| Android goldfish kernel and device projects | Android Emulator guest kernel/device models | Emulator compatibility reference only |
| AOSP Cuttlefish, crosvm, Emulator/QEMU/AEMU | Full virtual-device, VMM and desktop emulator references | Do not mistake them for universal OEM backends |
| AOSP minigbm, libdrm, gfxstream, ANGLE, SwiftShader, virglrenderer | Virtual GPU, graphics translation and software rendering references | Hardware capability and driver support must be probed |
| Android CTS / VTS | Compatibility, HAL, kernel and system validation suites | Optional `zerodroid-test` group; very large checkout |
| Upstream Linux `torvalds/linux` | Compare generic KVM, ARM64 and VirtIO changes | Optional `zerodroid-optional` group; not a substitute for Android Common Kernel |

## Source groups

- `default`: 62 projects, including the core ROM pipeline and primary Android platform/runtime sources.
- `zerodroid-rom-reference`: 6 broad third-party ROM suite/reference projects (TIK, RomTools, standalone sparse converter, OEM firmware extractor, Magisk and AnyKernel3).
- `zerodroid-optional`: upstream mainline Linux.
- `zerodroid-test`: CTS and VTS.

Download everything in the manifest:

```bash
ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test \
  bash third_party/scripts/sync_sources.sh
```

## Important image safety and correctness rules

- A ROM ZIP is an archive, not a single image. List and validate entries before extracting; reject absolute paths, traversal paths, symlinks escaping the workspace, duplicate critical entries, unreasonable expansion ratios and oversized files.
- A/B `payload.bin` is an operation stream. It is not a ZIP containing prebuilt images. Incremental payloads require the correct source/base partitions and operation support; verify payload signatures, metadata and output hashes.
- Legacy `.new.dat.br` and `.patch.dat` OTA flows are different formats from modern payload OTA. Do not treat every `.dat` as a raw image.
- `super.img` contains LP metadata and extents. Inspect slot metadata and logical partition groups with `lpdump`, extract with `lpunpack`, and reconstruct only with a compatible layout using `lpmake`.
- Android sparse images are not filesystem images. Convert sparse to raw using `simg2img` before filesystem inspection; convert back with `img2simg` only when the target format requires it.
- ext4, EROFS and F2FS are distinct filesystem formats. Do not rename one as another. Extract and rebuild with format-aware tools while preserving fs_config, file_contexts, capabilities, ownership, timestamps and SELinux labels where applicable.
- Boot images are versioned container formats. Preserve header version, page size, cmdline, kernel, ramdisk, vendor ramdisk fragments, DTB/DTBO and bootconfig. A repacked boot image may invalidate vbmeta/AVB descriptors; verify the complete matched image set.
- Never execute updater-script, shell scripts, installer ZIP scripts or ROM-supplied binaries during import. Parse them as untrusted data in a sandbox.
- Image conversion and repacking must never write to physical host partitions. Outputs are guest artifacts in a bounded workspace.

## Architectural policy

The official AOSP tools and formats are the normative implementation references. Third-party utilities are compatibility references and independent test oracles; do not make them unconditional runtime dependencies. Keep their licenses and transitive dependencies separate. The manifest is curated rather than a mirror of all AOSP.

AVF/KVM source availability does not grant ordinary APKs privileged VM access. crosvm is a VMM, not a hypervisor. GSI is only the system image; boot, vendor_boot, init_boot, vbmeta, super, kernel, Vendor HAL, VINTF and firmware must form a compatible Guest Image Set.

Each successful sync exports `SOURCE_LOCK.xml` and `SOURCE_LOCK.json`; review and commit resolved revisions. Source checkout is not integration, build success or device validation.
