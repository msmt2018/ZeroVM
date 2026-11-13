# Third-party source intake plan

Every upstream must be pinned to an exact tag/commit and have a license inventory, patch queue, build recipe, security review and integration boundary. AOSP and kernel trees remain dedicated source checkouts.

| Upstream | Purpose | Intake strategy |
|---|---|---|
| AOSP platform/system/core | Android base libraries and host tools | Select libraries individually; preserve Soong rules |
| Android Common Kernel | Guest ARM64 kernel | Dedicated checkout pinned per Guest Profile |
| crosvm | VMM and VirtIO device models | Dedicated Cargo checkout; KVM permission gate |
| Android Virtualization Framework | Platform VM management | Adapter gated by service and caller authorization |
| Cuttlefish | Reference virtual Android platform | Reference profile; Vendor is not universal |
| libsparse/liblp/libavb/libfs_mgr | Image and partition primitives | Pin AOSP revision and transitive dependencies |
| minigbm/gfxstream | Virtual graphics | Separate stage requiring matching Guest and Host support |

No upstream implementation is vendored by this foundation change. Next, pin exact revisions and licenses before adding submodules or reproducible fetch scripts. A URL alone is not a dependency lock.
