# ZeroDroid third-party source workspace

This workspace keeps upstream projects as separate Git checkouts. It is not a source-code dump inside the ZeroDroid app modules.

## Full relevant source checkout

Install Git, Python 3 and the official Android `repo` launcher, then run from the repository root:

```bash
ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test \
  bash third_party/scripts/sync_sources.sh
```

The manifest contains 73 projects:
- 64 default projects: Android platform/runtime, AVF/Microdroid, Android Common Kernel and Kleaf, crosvm, Cuttlefish, Android Emulator/QEMU/goldfish/AEMU, graphics, recovery/OTA, and the ROM/image pipeline.
- 6 optional ROM references: TIK, RomTools, standalone simg2img, OEM firmware extractor, Magisk and AnyKernel3.
- 1 optional upstream mainline Linux comparison tree.
- 2 optional Android CTS/VTS test source trees.

## ROM and image pipeline included in the default group

- ZIP/OTA archive codecs and extraction support: AOSP libziparchive, libarchive, AOSP zlib, bzip2, xz, Brotli, zstd and lz4.
- Modern A/B OTA: AOSP update_engine plus payload-dumper-go for partition extraction from payload.bin and OTA ZIP inputs.
- Legacy block OTA: sdat2img, img2sdat, Brotli and imgpatchtools for .dat.br, transfer-list and patch.dat workflows.
- Dynamic partitions: AOSP lpdump, lpunpack and lpmake from system/extras/partition_tools.
- Sparse images: AOSP libsparse simg2img/img2simg/append2simg.
- Filesystems: e2fsprogs, erofs-utils and f2fs-tools for ext4, EROFS and F2FS.
- Boot images: AOSP mkbootimg/unpack_bootimg plus Android_boot_image_editor for boot, vendor_boot, init_boot, recovery, DTB/DTBO, vbmeta and payload format reference.
- DTB/DTBO: libufdt and device-tree tooling references.

The optional ROM reference group adds end-to-end tools and OEM/installer references for broader format discovery. These are not all suitable for direct production integration; inspect their licenses, dependencies, security posture and format coverage first.

## Source groups and resource controls

- `ZERODROID_SYNC_JOBS=4`: parallel sync jobs.
- `ZERODROID_SYNC_DEPTH=1`: default shallow checkout; use 0 for full Git history.
- `ZERODROID_SYNC_GROUPS=default`: choose repo groups.
- Add `zerodroid-rom-reference`, `zerodroid-optional` and `zerodroid-test` to include all 73 manifest entries.

The first sync is network-, disk- and time-intensive. Framework, kernel, QEMU, CTS/VTS and ROM suite repositories can be large. Successful sync emits `SOURCE_LOCK.xml` and `SOURCE_LOCK.json` with resolved revisions.

Downloaded source is not automatically integrated, built or validated. Keep upstream projects separate, preserve licenses and track local changes as auditable patches. See [source matrix](UPSTREAMS.md) and [license governance](../docs/36-upstream-source-checkout-and-license-governance.md).
