# ZeroVM

ZeroVM aims to provide an independent Android Guest system environment on Android devices. The intended Guest has its own kernel, Android userspace, virtual hardware, storage and security boundary; it is not a container or process sandbox.

## Repository layout

- app/: Android application shell.
- core/: Kotlin common, model, runtime API and JNI bridge modules.
- native/: public C ABI and C++ runtime foundation.
- rust/: memory-safe image parsing primitives.
- schemas/: versioned machine-readable contracts.
- third_party/manifest/: selected upstream source manifest.
- third_party/scripts/: reproducible source checkout and lock generation.
- docs/: architecture, security, interfaces and validation specifications.

## Pull upstream core source

Install Git, Python 3 and the official Android repo launcher, then run:

    bash third_party/scripts/sync_sources.sh

This checks out selected AOSP image/partition libraries, AVF, Android Common Kernel, Cuttlefish reference sources, minigbm, libdrm, gfxstream, crosvm, filesystem tools and support libraries into third_party/aosp/. The manifest pins crosvm to an immutable commit and records exact resolved revisions in generated source-lock files.

This is a selective source intake, not a full AOSP mirror. Source checkout does not mean ZeroVM has integrated, built or validated those components. See docs/36-upstream-source-checkout-and-license-governance.md.

## Toolchain baseline

AGP 9.5.0 / Gradle 9.6.0, JDK 25, Android API 37, NDK r30 30.0.16248370, C++20 and Rust 2024. Android bytecode targets Java 17 while Gradle runs on JDK 25.

## Runtime status

The current application and native code are foundation modules only. No production AVF/crosvm/KVM backend is wired. Ordinary APKs are not assumed to possess privileged virtualization permissions. No arbitrary OEM ROM compatibility, universal Cuttlefish Vendor, or default AVB/SELinux bypass is claimed.

See docs/README.md for the full technical documentation index.
