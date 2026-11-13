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
18. [工程架构与技术选型总规范](18-engineering-architecture-and-technology-selection.md)
19. [Android App、应用框架与 Runtime](19-android-app-framework-and-runtime.md)
20. [Native Core、JNI 与虚拟化 Backend](20-native-core-jni-and-backend.md)
21. [镜像仓库、分区引擎与 Guest Profile](21-image-storage-and-guest-profile-framework.md)
22. [虚拟硬件与设备模型框架](22-virtual-hardware-and-device-frameworks.md)
23. [Guest Platform：Kernel、HAL、Framework 与图形](23-guest-platform-kernel-hal-graphics.md)
24. [安全、资源管理与 Host/Guest 桥接](24-security-resource-and-device-bridge-framework.md)
25. [构建、CI、测试与可观测性](25-build-ci-testing-observability.md)
26. [ROM 家族兼容性与全品类 ROM 适配架构](26-rom-family-compatibility-architecture.md)
27. [虚拟手机硬件模型、固件材料与完整启动链](27-physical-device-model-firmware-and-boot-chain.md)
28. [厂商扩展、专有 HAL 与系统服务适配框架](28-vendor-extension-and-proprietary-service-adaptation.md)
29. [ROM 导入、OTA 解析与分区转换流水线](29-rom-ingestion-ota-and-partition-transformation.md)
30. [Host 虚拟化能力门控、权限与后端选择](30-host-virtualization-and-capability-gating.md)
31. [跨层 API、JNI、C ABI 与 Backend 接口契约](31-cross-layer-api-and-abi-contracts.md)
32. [VM、ROM、Guest Profile 与虚拟硬件数据结构规范](32-versioned-data-schemas-and-manifests.md)
33. [Runtime 状态机、事件流与统一错误模型](33-runtime-state-events-and-error-model.md)
34. [机器可读 Schema 与契约验证](34-machine-readable-schemas-and-validation.md)
35. [Implementation foundation 与工具链基线](35-implementation-foundation-and-toolchain.md)

## 设计基线

- ZeroVM 目标是真正的系统级虚拟机：Guest 拥有独立 Kernel、地址空间、系统服务、虚拟设备和持久化数据。
- GSI 是 Android 通用 System 镜像，不是完整虚拟设备镜像。运行 GSI 仍需匹配的 Guest Kernel、Vendor HAL、VINTF、启动配置和分区布局。
- 宿主硬件配置必须按设备实际能力和实时资源状态计算，不使用适用于所有手机的固定 CPU/RAM/GPU 配置。
- 虚拟硬件 ABI 尽量稳定；设备能力可以协商，资源预算可以动态变化，但不能破坏 Guest 已建立的设备契约。
- 优先复用 AVF、KVM、crosvm、Linux VirtIO、AOSP 和 Cuttlefish 的成熟实现。
- 不将容器、chroot、proot 或普通应用进程隔离描述为完整 Android VM。
- 不默认关闭 AVB 或 SELinux 来换取启动成功。
- 所有能力必须区分设计目标、代码实现、构建通过、模拟环境验证和真机验证。
