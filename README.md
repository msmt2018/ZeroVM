# ZeroVM

ZeroVM is an Android-on-Android system virtual machine project. The intended Guest has its own kernel, Android userspace, virtual hardware, storage and security boundary; it is not a container or process sandbox.

## Repository layout

- app: Android application shell.
- core: Kotlin models, common contracts, Runtime API and JNI bridge.
- native: public C ABI and C++ runtime foundation.
- rust: memory-safe parsing primitives.
- schemas: versioned machine-readable data contracts.
- third_party: upstream source intake records.
- docs: architecture, security and validation specifications.

A module existing does not imply a usable VM backend. AVF/KVM availability and caller authorization are runtime capabilities; ordinary APKs are not assumed to possess privileged virtualization permissions.

## Toolchain baseline

AGP 9.5.0 / Gradle 9.6, JDK 25, API 37, NDK r30 30.0.16248370, C++20 and Rust 2024. Android bytecode targets Java 17. Versions are pinned for reproducibility.

Gradle wrapper properties are present, but wrapper scripts/JAR are not yet included; the Android build is not self-contained.

## Validation

Native host tests: cmake -S native -B build/native -DZEROVM_BUILD_TESTS=ON; cmake --build build/native; ctest --test-dir build/native --output-on-failure.
Rust tests: cargo test --workspace.
Android build requires JDK 25, Android SDK Platform 37, NDK r30 and CMake. The app is currently a shell; no production virtualization backend is wired.

See docs/README.md for technical documentation.
