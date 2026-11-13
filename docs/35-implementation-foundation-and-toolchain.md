# 35. Implementation foundation and toolchain baseline

## Module layout

This slice establishes app, core:common, core:model, core:runtime-api and core:native-bridge modules, a native C ABI/C++ library, and a Rust image parsing primitives crate. The app is the composition root; UI and application code depend on interfaces rather than a specific virtualization backend.

## Toolchain

The pinned baseline is AGP 9.5.0, Gradle 9.6.0, JDK 25, API 37, NDK r30 30.0.16248370, C++20 and Rust 2024. NDK r29 has been superseded by r30 LTS. Android bytecode targets Java 17 while Gradle runs on JDK 25. Versions must be updated together with CI, SDK packages and build metadata.

## Native ABI

The public C ABI uses fixed-width C types, explicit struct sizes, opaque 64-bit handles, stable status numbers and exported zvm-prefixed symbols. C++ exceptions, STL and Rust-owned values do not cross the ABI.

The first native implementation provides ABI discovery and Runtime handle creation/destruction only. It does not create or start a VM. Those operations remain unavailable until an authorized real backend adapter is integrated.

## Kotlin Runtime API

The Kotlin interface defines suspend control operations, snapshots and Flow event observation. It is a contract only; no production Runtime implementation or backend adapter is registered in this slice.

## Third-party source policy

Upstream projects are pinned and integrated selectively. AOSP, Android Common Kernel and crosvm have independent build ecosystems; bulk-copying them into the app repository harms reproducibility and license tracking. No upstream implementation is claimed to be vendored by this change.

## Validation status

C ABI and host contract test sources, Rust bounded primitives and unit test sources, and Android module declarations are present. GitHub file/tree APIs do not execute CMake, Cargo or Gradle. These are source-level deliverables only until CI runs; they are not claimed BUILT or UNIT_TESTED.
