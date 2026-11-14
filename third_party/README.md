# ZeroDroid third-party source workspace

This workspace keeps upstream projects as separate Git checkouts. It is not a source-code dump inside the ZeroDroid app modules.

## Full relevant source checkout

Install Git, Python 3 and the official Android `repo` launcher, then run from the repository root:

```bash
ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test \
  bash third_party/scripts/sync_sources.sh
```

The manifest contains 80 projects:
- 69 default projects: Android platform/runtime, AVF/Microdroid, Android Common Kernel and Kleaf, crosvm, Cuttlefish, Android Emulator/QEMU/goldfish/AEMU, DTB compiler, audio stack references, Mesa/Vulkan graphics sources, recovery/OTA, and the ROM/image pipeline.
- 6 optional ROM references: TIK, RomTools, standalone simg2img, OEM firmware extractor, Magisk and AnyKernel3.
- 3 optional platform/firmware references: upstream Linux, EDK II UEFI firmware and AOSP Bluetooth guest stack.
- 2 optional Android CTS/VTS test source trees.

## ROM and image pipeline included in the default group

- ZIP/OTA archive codecs and extraction support: AOSP libziparchive, libarchive, AOSP zlib, bzip2, xz, Brotli, zstd and lz4.
- Modern A/B OTA: AOSP update_engine plus payload-dumper-go for partition extraction from payload.bin and OTA ZIP inputs.
- Legacy block OTA: sdat2img, img2sdat, Brotli and imgpatchtools for .dat.br, transfer-list and patch.dat workflows.
- Dynamic partitions: AOSP lpdump, lpunpack and lpmake from system/extras/partition_tools.
- Sparse images: AOSP libsparse simg2img/img2simg/append2simg.
- Filesystems: e2fsprogs, erofs-utils and f2fs-tools for ext4, EROFS and F2FS.
- Boot images: AOSP mkbootimg/unpack_bootimg plus Android_boot_image_editor for boot, vendor_boot, init_boot, recovery, DTB/DTBO, vbmeta and payload format reference.
- DTB/DTBO: libufdt plus dtc/libfdt for DTS/DTB compilation and structural inspection.
- Audio: system/media and tinyalsa references.
- Virtual GPU: Mesa and Vulkan-Headers complement virglrenderer/gfxstream and the existing graphics stack.
- Optional UEFI boot path: EDK II; not required when using direct-kernel loading.
- Optional Bluetooth stack: AOSP Fluoride; requires a real virtual-controller/Host bridge implementation to be useful.

The optional ROM reference group adds end-to-end tools and OEM/installer references for broader format discovery. These are not all suitable for direct production integration; inspect their licenses, dependencies, security posture and format coverage first.

## Source groups and resource controls

- `ZERODROID_SYNC_JOBS=4`: parallel sync jobs.
- `ZERODROID_SYNC_DEPTH=0`: default full history for the selected branch of every upstream repository. Set to `1` only when intentionally requesting a shallow checkout.
- `ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test`: default includes every one of the 80 manifest entries, including ROM references, Linux/UEFI/Bluetooth references and CTS/VTS trees.
- Override `ZERODROID_SYNC_GROUPS` only for an intentional partial checkout; do not use it for the canonical full source acquisition.

The first sync is network-, disk- and time-intensive. Framework, kernel, QEMU, CTS/VTS and ROM suite repositories can be large. Successful sync emits `SOURCE_LOCK.xml` and `SOURCE_LOCK.json` with resolved revisions. The lock-generation step refuses to publish a lock unless all 80 projects are present and revision-pinned.

Downloaded source is not automatically integrated, built or validated. Keep upstream projects separate, preserve licenses and track local changes as auditable patches. See [source matrix](UPSTREAMS.md) and [license governance](../docs/36-upstream-source-checkout-and-license-governance.md).
