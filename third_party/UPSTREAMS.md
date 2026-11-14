# Third-party source intake and checkout plan

The executable manifest is `third_party/manifest/zerodroid.xml`. Run `bash third_party/scripts/sync_sources.sh` to download the default source groups into `third_party/aosp/`. Add `ZERODROID_SYNC_GROUPS=default,zerodroid-optional,zerodroid-test` to include upstream mainline Linux plus CTS/VTS test sources.

## Source matrix

| Source project | Role in ZeroDroid | Policy |
|---|---|---|
| AOSP `system/gsid` | GSI installation and Dynamic System Update service reference | Android 16 QPR1 baseline |
| AOSP `system/core`, `system/extras` | init-adjacent utilities, fs_mgr, libsparse, liblp and image tools | Android 16 QPR1 baseline |
| AOSP `external/avb` | AVB, vbmeta parsing and verification | Never silently disable verification |
| AOSP `bootable/recovery`, `system/update_engine` | Recovery, OTA payload and update flow reference | Parse/transform images; do not flash the host |
| AOSP `system/tools/mkbootimg` | boot/vendor_boot/init_boot image tooling | Preserve boot header/version semantics |
| AOSP e2fsprogs, f2fs-tools, erofs-utils | ext4/F2FS/EROFS image inspection and creation | Keep filesystem tools isolated |
| AOSP SELinux and `system/sepolicy` | Guest MAC policy and policy validation | Never use permissive mode as a default |
| AOSP `libvintf`, `hardware/interfaces`, `hardware/libhardware` | VINTF manifests, HAL contracts and device interfaces | Guest profile must match HAL/VINTF |
| AOSP `frameworks/base`, `frameworks/native`, `frameworks/av` | Guest Framework, native services, media and graphics contracts | Reference/integration source; large checkout |
| AOSP libbase, logging, libutils, libcutils, libbinder, libhidl, libfmq, libprocessgroup | Native foundations, Binder/HIDL IPC and resource controls | Reuse only where API/ABI compatible |
| AOSP `packages/modules/Virtualization` | AVF, Microdroid, VM service and pVM lifecycle reference | Runtime permission/feature gate remains mandatory |
| Android Kernel build `kernel/build` | Kleaf/Bazel kernel build infrastructure | `main-kernel` branch |
| Android Common Kernel `kernel/common` | Primary Android Guest kernel, KVM/pKVM, VirtIO and Android kernel changes | `android16-6.12-lts` baseline; match the Guest Profile |
| Android goldfish kernel and goldfish device projects | Android Emulator guest kernel/device models | Emulator compatibility reference only |
| AOSP `device/google/cuttlefish` | Complete virtual Android device configuration and launch reference | Not a universal Vendor or generic phone profile |
| AOSP `platform/tools/emulator`, `external/qemu`, `hardware/google/aemu` | Official Android Emulator, QEMU device models and emulator support libraries | Desktop emulator reference; not the production VMM |
| AOSP `external/crosvm` and upstream crosvm | VMM, VM lifecycle, VirtIO devices and backend design | AOSP mirror plus immutable upstream commit |
| AOSP minigbm, libdrm, gfxstream, ANGLE, SwiftShader, virglrenderer | Virtual GPU, graphics translation and software rendering references | Hardware capability and driver support must be probed |
| AOSP adb | Guest debugging and host/guest development bridge | Explicitly gate and isolate debug access |
| BoringSSL, zstd, lz4, libxml2, protobuf, googletest | Crypto/compression/serialization/XML/test support | Keep license and dependency metadata |
| Android CTS / VTS | Compatibility, HAL, kernel and system validation suites | Optional `zerodroid-test` group; very large checkout |
| Upstream Linux `torvalds/linux` | Compare generic KVM, ARM64 virtualization and VirtIO changes | Optional `zerodroid-optional` group; not a substitute for Android Common Kernel |

## Important boundaries

- Android Common Kernel is the correct primary kernel source for an Android Guest. The upstream Linux checkout is for comparison and cross-reference, because Android Common Kernel includes Android-specific patches and ABI expectations.
- AVF, KVM and pKVM source availability does not imply an ordinary third-party APK can access `/dev/kvm`, privileged VM management APIs, protected VM firmware, or hypervisor capabilities on a retail device.
- crosvm is a VMM, not a hypervisor. KVM/pKVM/Gunyah/other supported hypervisors are separate host capabilities and must be discovered at runtime.
- VirtIO device models, guest drivers and host backends must agree on feature negotiation, queue layout, transport, interrupts, memory sharing and lifecycle. Merely downloading a GPU or block driver does not make it usable on Android.
- Android Emulator's QEMU/goldfish stack is valuable for source-level device-model and test references, but it is not the target production backend for an ARM64 Android phone.
- Cuttlefish is a reference virtual device with its own kernel, vendor and host setup. Do not combine its Vendor with an arbitrary GSI and claim universal compatibility.
- GSI is only the system image. Boot, vendor_boot, init_boot, vbmeta, super/dynamic partitions, Vendor HAL, VINTF, firmware and kernel compatibility must be treated as a matched image set.
- Do not mirror proprietary Vendor blobs, firmware, secure-world binaries, DRM keys, KeyMint/TEE secrets or device-specific signing material into this public repository.

The manifest is a curated dependency set, not a mirror of all AOSP. A project is added only when it provides a concrete implementation or reference needed by the runtime, image pipeline, guest platform, graphics or validation. Each successful sync exports `SOURCE_LOCK.xml` and `SOURCE_LOCK.json`; review and commit those locks when accepting a source baseline.

Do not bulk-copy upstream code into ZeroDroid-owned modules. Keep upstream repositories in their own checkouts, preserve license files and carry local changes as auditable patches. Repository-level license metadata is not a substitute for file-level and transitive license review.
