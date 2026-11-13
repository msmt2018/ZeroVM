# ZeroVM 技术文档

本目录定义 ZeroVM 的产品目标、虚拟化概念、系统架构、宿主硬件探测、自适应资源管理、虚拟硬件、Guest Kernel、启动链、GSI 镜像、Vendor HAL、图形设备、设备桥接、安全隔离、VM 生命周期、兼容性和开发路线。

> 本文档描述目标设计，不代表仓库已经实现对应功能。实现状态以源码、自动化测试和目标真机验证为准。

## 文档索引

1. [项目目标与范围](01-project-goals.md)
2. [Android 虚拟化概念](02-virtualization-concepts.md)
3. [总体软件架构](03-system-architecture.md)
4. [宿主硬件发现与自适应资源](04-host-discovery-adaptive.md)
5. [虚拟硬件平台与 ABI](05-virtual-hardware.md)
6. [Guest Kernel 与启动链](06-guest-kernel-boot.md)
7. [GSI、镜像与分区](07-gsi-images-partitions.md)
8. [Vendor HAL 与 VINTF](08-vendor-hal-vintf.md)
9. [图形与显示系统](09-graphics-display.md)
10. [Host/Guest 设备桥接](10-device-bridges.md)
11. [安全模型与隔离](11-security-isolation.md)
12. [VM 完整生命周期](12-vm-lifecycle.md)
13. [兼容性与测试](13-compatibility-testing.md)
14. [Native 构建与第三方依赖](14-native-build-dependencies.md)
15. [开发路线与风险](15-roadmap-risks.md)
16. [术语表](16-glossary.md)
17. [Android Guest 完整启动条件清单](17-guest-boot-prerequisites.md)

## 设计基线

- ZeroVM 目标是真正的系统级虚拟机：Guest 拥有独立 Kernel、地址空间、系统服务、虚拟设备和持久化数据。
- GSI 是 Android 通用 System 镜像，不是完整虚拟设备镜像。运行 GSI 仍需匹配的 Guest Kernel、Vendor HAL、VINTF、启动配置和分区布局。
- 宿主硬件配置必须按设备实际能力和实时资源状态计算，不使用适用于所有手机的固定 CPU/RAM/GPU 配置。
- 虚拟硬件 ABI 尽量稳定；设备能力可以协商，资源预算可以动态变化，但不能破坏 Guest 已建立的设备契约。
- 优先复用 AVF、KVM、crosvm、Linux VirtIO、AOSP 和 Cuttlefish 的成熟实现。
- 不将容器、chroot、proot 或普通应用进程隔离描述为完整 Android VM。
- 不默认关闭 AVB 或 SELinux 来换取启动成功。
- 所有能力必须区分设计目标、代码实现、构建通过、模拟环境验证和真机验证。