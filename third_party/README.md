# ZeroDroid third-party source workspace

This workspace keeps upstream projects as separate Git checkouts. It is not a source-code dump inside the ZeroDroid app modules.

## Full relevant source checkout

Install Git, Python 3 and the official Android `repo` launcher, then run from the repository root:

```bash
bash third_party/scripts/sync_sources.sh
```

The default `default` group now includes the selected Android 16 AOSP platform foundations, ROM/recovery/OTA/image tooling, Android framework and HAL contracts, AVF/Microdroid sources, Android Common Kernel, emulator/QEMU/goldfish references, Cuttlefish, crosvm, VirtIO and graphics source projects, and filesystem libraries.

The upstream mainline Linux tree is a large comparison checkout, not the primary Android Guest kernel. To include it too:

```bash
ZERODROID_SYNC_GROUPS=default,zerodroid-optional bash third_party/scripts/sync_sources.sh
```

The Android Common Kernel is the primary Android kernel baseline because it carries Android-specific changes and the KVM/pKVM/VirtIO implementation. The pinned branch is `android16-6.12-lts`. The emulator goldfish kernel and emulator device trees are compatibility/reference material, not the production phone backend.

## Resource controls and source locks

- `ZERODROID_SYNC_JOBS=4` controls parallel sync jobs.
- `ZERODROID_SYNC_DEPTH=1` is the default shallow checkout; use `0` for full Git history.
- `ZERODROID_SYNC_GROUPS=default` selects the source groups; add `zerodroid-optional` for upstream mainline Linux.
- The first sync is network-, disk- and time-intensive. Kernel, Android framework, QEMU and graphics projects are large.
- After a successful sync, `SOURCE_LOCK.xml` and `SOURCE_LOCK.json` record resolved revisions. Review and commit these generated lock files when accepting an upstream baseline.

Downloaded source is not automatically integrated, built or validated. Keep each upstream's native build system and license files. Cuttlefish is a reference platform, not a universal Vendor. Android Emulator/QEMU is a desktop development reference and is not the production phone VMM. AVF source does not grant ordinary APKs privileged permissions.

We intentionally do not mirror all Android Studio/IntelliJ source, every AOSP project, prebuilt SDK archives, proprietary Vendor blobs, device firmware, signing keys, TEE/KeyMint secrets, or closed-source Google Play components. Those are either unrelated to the guest runtime, legally/device-specific restricted, or cannot be redistributed as general source dependencies.

See [the source matrix](UPSTREAMS.md) and [license governance](../docs/36-upstream-source-checkout-and-license-governance.md).
