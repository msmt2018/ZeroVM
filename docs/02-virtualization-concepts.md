# 02. Android 虚拟化概念

## Host 与 Guest

- Host：运行 ZeroDroid App 和虚拟化服务的真实 Android 设备。
- Guest：由 VMM 创建、运行独立 Kernel 和 Android 系统的虚拟设备。
- Hypervisor：提供 CPU 虚拟化、二阶段地址转换和隔离的底层机制。
- VMM：创建 vCPU、分配 Guest 内存、构建设备模型并管理 VM 运行。
- Guest Kernel：Guest 自己启动的 Linux/Android Kernel，不是 Host Kernel 的另一个进程。
- Virtual Device：Guest 可发现和驱动的虚拟硬件，例如 VirtIO Block、Net、GPU、Input。
- Device Bridge：在 Host 与 Guest 之间传输受控数据的桥接服务。

## VM 与容器的区别

| 特性 | 完整 VM | 容器/chroot/proot |
|---|---|---|
| 独立 Guest Kernel | 是 | 否 |
| 独立虚拟 CPU/内存设备模型 | 是 | 通常否 |
| 独立启动链和 Android init | 是 | 通常否 |
| 独立内核版本 | 是 | 否 |
| Hypervisor 隔离 | 可具备 | 通常使用宿主内核隔离机制 |

ZeroDroid 的产品定义要求真正的 VM。设备不支持所需虚拟化能力时，应报告不支持，不能悄悄退化为容器仍称作虚拟机。

## KVM、pKVM、AVF 与 crosvm

- KVM：Linux 内核提供的虚拟化基础设施，使用硬件虚拟化能力运行 Guest。
- pKVM：面向保护型虚拟机的 KVM 扩展，强化 Host 与 Guest 的内存隔离。
- AVF：Android 提供的虚拟化框架、管理 API、服务和相关组件。
- VirtualizationService：Android 系统侧虚拟化管理服务。
- crosvm：Rust 实现的 VMM，管理 VM、vCPU、内存和虚拟设备。
- Cuttlefish：AOSP 完整虚拟 Android 设备参考平台，包含设备配置、Kernel、Vendor、HAL 和工具链。

这些概念处于不同层次，不可互换。AVF 不是 Hypervisor，crosvm 不是 Guest Kernel，VirtIO 也不是虚拟化后端。

## Android GSI 的含义

GSI 是面向 Treble 设备的通用 Android System 实现。它主要提供 System 分区内容，不自动提供适用于任意虚拟硬件的 Kernel、Vendor HAL、DTB、Bootloader、设备树或完整磁盘布局。

运行 GSI 仍需匹配 CPU 架构、Guest Kernel、Vendor/HAL、VINTF、Boot 材料、分区布局、fstab、AVB 和 SELinux 策略。

## AVF 可用性限制

AVF 能否使用取决于设备硬件、系统版本、厂商配置、服务状态和调用权限。框架 API 可能受到系统权限和预装应用限制。普通第三方 App 不应假定可以直接调用所有 AVF 接口，也不能假定 /dev/kvm 对应用可见。

ZeroDroid 必须检测 AVF 版本、VirtualizationService、应用权限、Guest 类型支持和实际 VM 创建结果。

## 关键边界

- Hypervisor 隔离不自动提供 Guest 图形加速。
- VirtIO GPU 不自动等于可用 Android Composer/Gralloc。
- Host 有某个物理设备不代表 Guest 可以直接访问它。
- Guest HAL 必须与 Guest System 的接口版本兼容。
- 运行时资源动态调整受 Guest Kernel 和 VMM 能力约束。