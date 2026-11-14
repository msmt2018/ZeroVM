# 37. Android ARM64 宿主移植、硬件适配与上游源码改造规范

## 1. 强制目标

ZeroVM 的生产宿主是 Android 手机/平板 ARM64 SoC，不是 x86_64 PC Linux。上游源码支持 Linux、ARM64 或交叉编译，不代表它能在 Android App/Android 系统中运行。不得仅把项目编译成 `arm64-v8a` 就宣称完成移植。

每个上游项目必须分别审计：CPU ISA、Android ABI/Bionic、Linux 用户态依赖、Kernel ioctl/driver、SoC/板级硬件假设、Guest 设备 ABI、Android 权限/SELinux、构建与运行依赖。

## 2. Host 与 Guest 硬件基线

### Android Host
- 主目标 ABI：`arm64-v8a`。x86_64 只可用于开发机工具/CI，不得成为 APK 运行依赖。
- CPU：AArch64；运行时读取实际 feature，不能假设所有 ARMv8.x/ARMv9 扩展都存在。
- 虚拟化：检查 EL2 所有权、ARM64 KVM/pKVM、Stage-2、GIC virtualization、虚拟 Timer、固件/安全监控器、AVF 服务和系统授权。
- 中断：ARM GICv3/v4 按真实能力探测，不得使用 x86 APIC/IOAPIC。
- 内存：按 Android 可用内存、页大小、连续内存、DMA 限制、IOMMU/SMMU、共享内存和内存压力动态预算。
- 手机 GPU、显示、音频、相机、传感器、UFS/eMMC、Wi-Fi/蓝牙由 Android 驱动/HAL 管理；不得假设 PC PCI 设备可任意透传。
- OS：Bionic、linker namespace、SELinux、Binder、Android init/property、UID 沙箱、前台服务限制。

### Android Guest
Guest vCPU feature 必须是 Host 硬件、Hypervisor 和 VMM 能力的交集。虚拟 GIC、Timer、PSCI、RAM map、VirtIO transport、DTB 和 Guest Kernel 驱动必须一致。不得向 Guest 暴露 Host 不支持的指令扩展。

## 3. 上游源码分级

| 类别 | 内容 | 处理 |
|---|---|---|
| A | 镜像格式、压缩、哈希、分区元数据等纯算法 | 优先复用，NDK 构建、fuzz 和边界测试 |
| B | 支持 ARM64 的 POSIX/Linux Native 库 | 审计 Bionic、mmap、线程、socket、信号、动态库和 linker namespace；必要时做 Android shim |
| C | Linux Host 服务/VMM，依赖 KVM、cgroup、seccomp、设备节点或宿主服务 | 拆分 Portable Core 与 Android Host Adapter，重写 Host Adapter |
| D | x86/PCI/ACPI/APIC/NUMA/BIOS/PC GPU 等硬件后端 | 禁用 PC 路径，只保留通用协议/算法，按 ARM 手机硬件重写 |
| E | AOSP/Android Guest 源码 | 按 Guest Kernel、产品配置、VINTF、HAL、SELinux、分区和启动链适配，不直接链接进 Host APK |
| F | 离线参考/测试工具 | 与生产运行时隔离，不进入 APK |
| X | 不满足架构、安全、许可证或性能要求 | 排除生产构建并记录原因 |

禁止将整个 Linux PC 上游仓库直接复制进 Android App。先识别可复用核心，再隔离 PC/Linux 专属代码。

## 4. VMM 三层结构

1. **Portable VMM Core**：VM 配置、vCPU 生命周期抽象、Guest memory map、设备 ABI、启动编排、事件和错误模型；不依赖 Activity、PC 设备或 Linux 发行版服务。
2. **Android Host Adapter**：Android 文件描述符、内存、线程、日志、权限、SELinux、服务集成、设备发现和资源预算；只调用系统真实提供且授权的接口。
3. **Hypervisor Adapter**：AVF、授权 KVM/pKVM 或经独立验证的厂商 Hypervisor；实现 VM/vCPU fd、ioctl、内存注册、GIC/IRQ、Timer、PSCI 等差异。

crosvm 必须逐 crate/模块裁剪审计：保留可移植 VMM core 和必要 VirtIO 设备；替换/禁用 Linux 桌面或服务器专属服务；检查 libc、系统调用、seccomp、cgroup、用户/组切换、设备发现、GPU 后端、动态库和构建脚本。把 crosvm 编译成 ARM64 不等于它能在 Android App 运行。

AVF 提供受支持能力时优先使用授权接口，不重复实现其保护 VM 管理。厂商 Hypervisor 必须独立 Adapter，私有 ioctl 不得伪装成 Android 通用接口。

## 5. PC 假设到手机平台的替换

| PC 假设 | Android ARM64 适配 |
|---|---|
| x86_64、CPUID、MSR | AArch64 feature/ID register 受控虚拟视图 |
| VT-x/AMD-V、VMX/SVM | ARM EL2/KVM/pKVM/厂商 Hypervisor |
| EPT/NPT | ARM Stage-2 |
| APIC/IOAPIC | ARM GIC |
| TSC/HPET | ARM Generic/Hypervisor Timer |
| PCI/ACPI/BIOS | 优先 VirtIO-MMIO + ARM DTB；PCI 仅在两端均实现时启用 |
| PC BIOS/UEFI 默认启动 | 可信 Direct Kernel Boot 或明确固件路径，匹配 DTB/ramdisk/bootconfig |
| NUMA/服务器大页/热插拔 | 按手机 SoC、页大小、内存压力实现；不支持则禁用 |
| KVM 节点默认可用 | 检查节点、SELinux、DAC、服务身份和 ioctl |
| systemd/udev/dbus | Android Service、init、Binder 和明确设备发现 |
| glibc/libudev/libsystemd | Bionic/NDK 替换或最小 shim，不打包宿主系统库 |
| PC GPU/virgl 直通 | Android 图形桥接；验证 AHardwareBuffer、DMA-BUF、Fence、Gralloc/HWC |
| ALSA/PulseAudio/PipeWire | Android Audio HAL/AudioTrack/受控音频桥 |
| PC Linux bridge | Android Connectivity/VpnService/授权 TUN 或受控网络桥 |
| 任意 PCI passthrough | 默认禁止；需 IOMMU/SMMU、DMA 隔离、驱动所有权和恢复证明 |

## 6. 拦截、适配、重写的边界

不得用全局 hook、任意 syscall 拦截或侵入式替换 Host Framework 伪造兼容性。改造只能发生在明确边界：

- Build-time feature gate：按 Android API、ABI、NDK、SoC feature 和内核头文件选择实现。
- Runtime capability gate：检查 AVF 服务、权限、KVM ioctl、GIC/Timer、设备节点和桥接能力；不满足即拒绝对应功能。
- OS shim：只为可替换模块提供小型 Bionic/Android 适配层，不伪造缺失的内核能力。
- Device backend interception：在 VMM 设备后端边界拦截 PC 专属设备创建请求，映射为真实实现的 VirtIO/虚拟设备；没有等价设备则返回 unsupported。
- Guest ABI：通过版本化 Device Manifest、DTB 和驱动协商提供设备，不可只改硬件 ID 欺骗 Guest。
- Security Broker：Host 文件、网络、音频、输入、图形都经最小权限 Broker；Guest 不可任意访问 Host 路径/设备。

## 7. ARM64 Guest Board 与启动链

每个 Guest Board Profile 必须定义 ARM64 vCPU feature mask、虚拟 GIC、Timer、PSCI、Guest RAM/保留区/共享内存、MMIO 区、VirtIO-MMIO base/size/IRQ、DTB CPU/memory/interrupt-controller/timer/psci/chosen/reserved-memory/virtio 节点、Kernel/modules/DTB/ramdisk/bootconfig/cmdline、分区/AVB/VINTF/HAL/SELinux，以及每个设备的 Host backend → VMM model → Guest driver → HAL/Framework 消费者闭环。

x86 ACPI、MP table、APIC 配置不能塞入 ARM64 Guest 启动链。ARM64 Guest 通常依赖 DTB 和 PSCI；具体协议必须按目标 Kernel/Android 版本验证。

## 8. 构建与自动测试

- Android NDK/Bionic 构建与 Linux Host 工具构建分离；APK 生产 Native 首先锁定 `arm64-v8a`。
- 每项依赖记录 source target、Android target、ABI、最低 API、编译器、链接库、许可证、patch set、权限和测试状态。
- 扫描 x86 汇编、CPUID/MSR/VMX/SVM/EPT/APIC/HPET/ACPI/PCI、glibc/systemd/udev/dbus/PulseAudio、硬编码 /dev、/sys、/run、/etc、cgroup/seccomp 和 Host 工具误入 APK。
- 对生产模块执行 NDK arm64-v8a 编译/链接、ELF machine/动态依赖/未解析符号/API/ABI 检查。
- 交叉编译不等于运行通过；还要执行 Android 设备运行、权限、ioctl、内存注册、GIC/Timer、设备闭环和压力测试。
- Guest Boot Ladder：Loader、Kernel console、first-stage init、mount/AVB、HAL/VINTF、Zygote、System Server、SurfaceFlinger。
- 压测低内存、温控降频、前后台切换、锁屏、进程回收、存储不足和设备重启。

## 9. 组件完成状态

每个上游项目必须标记为：`unreviewed`、`source-reviewed`、`android-buildable`、`device-runnable`、`integrated`、`end-to-end-verified` 或 `excluded`。所有状态必须附构建/设备/日志证据。没有具体 SoC、Android build fingerprint 和测试日志，不得声称适配所有手机或 Guest 已启动。
