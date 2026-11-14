# ZeroDroid

ZeroDroid aims to provide an independent Android Guest system environment on Android devices. The intended Guest has its own kernel, Android userspace, virtual hardware, storage and security boundary; it is not a container or process sandbox.

## Repository layout

- `app/`: Android application shell.
- `core/`: Kotlin common, model, runtime API and JNI bridge modules.
- `native/`: public C ABI and C++ runtime foundation.
- `rust/`: memory-safe image parsing primitives.
- `schemas/`: versioned machine-readable contracts.
- `third_party/manifest/`: selected upstream source manifest.
- `third_party/scripts/`: reproducible source checkout and lock generation.
- `docs/`: architecture, security, interfaces and validation specifications.

## Pull upstream source projects

Install Git, Python 3 and the official Android `repo` launcher, then run:

```bash
bash third_party/scripts/sync_sources.sh
```

The manifest contains 73 upstream projects. Its default group includes selected AOSP platform/runtime and archive codec sources, Android Common Kernel (KVM/pKVM/VirtIO source), AVF/Microdroid, crosvm, Cuttlefish, Android Emulator QEMU/goldfish/AEMU sources, virtual graphics, GSI/DSU, recovery/OTA, payload.bin extraction, legacy .dat/.patch OTA reconstruction, super.img/lpunpack/lpmake, sparse/raw conversion, ext4/EROFS/F2FS, boot/vendor_boot/init_boot unpack/repack, AVB/vbmeta and DTB/DTBO tools. Optional groups add broader ROM suites, OEM firmware extractors, mainline Linux and CTS/VTS.

To additionally fetch the optional upstream mainline Linux comparison tree and Android CTS/VTS validation sources:

```bash
ZERODROID_SYNC_GROUPS=default,zerodroid-rom-reference,zerodroid-optional,zerodroid-test bash third_party/scripts/sync_sources.sh
```

This is a curated source intake, not a complete AOSP or Android Studio mirror. The script creates separate upstream worktrees under `third_party/aosp/` and generates `SOURCE_LOCK.xml` / `SOURCE_LOCK.json` with exact resolved revisions. Source checkout does not mean ZeroDroid has integrated, built or validated those components. See [source matrix](third_party/UPSTREAMS.md) and [source governance](docs/36-upstream-source-checkout-and-license-governance.md).

## Toolchain baseline

AGP 9.5.0 / Gradle 9.6.0, JDK 25, Android API 37, NDK r30 30.0.16248370, C++20 and Rust 2024. Android bytecode targets Java 17 while Gradle runs on JDK 25.

## Runtime status

The current application and native code are foundation modules only. No production AVF/crosvm/KVM backend is wired. Ordinary APKs are not assumed to possess privileged virtualization permissions. No arbitrary OEM ROM compatibility, universal Cuttlefish Vendor, or default AVB/SELinux bypass is claimed.

See [technical documentation index](docs/README.md).
