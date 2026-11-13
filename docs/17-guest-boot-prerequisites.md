# 17. 启动 Android Guest 所需的完整条件清单

本文回答一个核心工程问题：ZeroVM 要具备什么条件，才能把 Android 的 `system.img`、`boot.img`、Linux Kernel 和其他镜像组合成一台真正可启动的 Android Guest 设备。

> 本文描述启动所需的输入、依赖和验证条件，不代表这些模块已经在 ZeroVM 中实现。镜像文件名并不能证明其内容、版本或相互兼容。

## 1. 先明确：启动的不是一个 IMG，而是一整套设备平台

Android 启动至少涉及以下关系：

```text
Host Android
  └─ ZeroVM Runtime
      └─ Virtualization Backend / Hypervisor
          ├─ vCPU + Guest RAM + Interrupt Controller
          ├─ Virtual Devices (Block / Net / Console / Vsock / GPU ...)
          ├─ Guest Boot Firmware or Direct Kernel Loader
          └─ Guest ARM64 Linux Kernel
               ├─ Device Tree / Bootconfig / Kernel Command Line
               ├─ First-stage init + early drivers
               ├─ system / vendor / product / system_ext / odm
               ├─ userdata / metadata
               └─ Android init → servicemanager → HAL → Zygote
                    └─ System Server → SurfaceFlinger → Launcher
```

`system.img` 通常只提供 Android System 分区内容。它不会自动提供适用于 ZeroVM 虚拟硬件的 Guest Kernel、DTB、Vendor HAL、启动 Ramdisk、设备驱动和 VMM。

因此必须把“镜像是否齐全”和“镜像是否能组成兼容的 Guest 设备”分开检查。

## 2. Host 真机必须先具备的条件

### 2.1 CPU 与硬件虚拟化

- 宿主 SoC/CPU 架构与 Guest 架构匹配；本项目首选 ARM64 Guest。
- 设备固件和 Host Kernel 实际启用了可用的硬件虚拟化能力。
- 存在可使用的虚拟化后端，例如受支持的 Android Virtualization Framework（AVF）路径，或经过平台验证的 KVM/crosvm 路径。
- 后端能创建 Guest vCPU、Guest Physical Memory、虚拟中断和 Guest 地址空间。
- Guest 的 CPU 特性必须是后端可安全暴露的交集，不能把 Host 所有 CPU 特性无条件透传。

CPU 有 ARM64 指令集不等于具备可供普通 App 使用的硬件虚拟化能力。

### 2.2 Android 系统服务与权限

- 检测 AVF/VirtualizationService 是否存在、版本是否满足要求、服务是否可用。
- 检查应用调用路径是否具有系统授予的权限和授权身份。
- 检查后端要求的镜像格式、VM 类型、内存上限和设备配置限制。
- 对普通第三方 APK，必须把权限不可用视为真实平台限制；不能假设声明 Manifest 权限、Rootless API 或 Shizuku 就能取得系统级 AVF/KVM 权限。
- 如果使用 Root 或系统集成方案，必须单独设计其安全边界、启动方式和设备兼容性，不能把它作为所有用户设备的默认前提。

### 2.3 可用资源

根据设备当前状态计算 Guest 资源，而不是使用固定配置：

- 可用内存、内存压力和 Host 系统保留量；
- CPU 核心拓扑、调度负载和热状态；
- GPU 后端及共享 Buffer 能力；
- 可用存储空间、文件系统和 I/O 状态；
- 电量、省电模式、温度和并发 VM 数量。

资源不足时应在启动前拒绝或降级可选设备，不得让 Host 因 Guest 启动而发生 OOM 或持续过热。

## 3. 启动 Android Guest 所需的镜像和材料

| 材料 | 作用 | 是否总是独立文件 |
|---|---|---|
| Guest Kernel | 启动独立 Linux 内核 | 必须有可加载的 Kernel；可能来自 boot.img，也可能单独提供 |
| boot.img | 常包含 Kernel、Generic Ramdisk、启动头信息 | 依 Android 版本和构建方案 |
| vendor_boot.img | Vendor Ramdisk、相关启动元数据、部分设备树信息等 | 取决于 Boot Header 版本和设备方案 |
| init_boot.img | Generic Ramdisk（适用版本/设备） | 取决于 Android 版本和设备方案 |
| dtb / dtbo | 描述 Guest 虚拟 CPU、内存、中断和设备拓扑 | 必须向 Guest 提供正确设备描述；可由后端生成或打包提供 |
| system.img | Android Framework、系统应用和系统 Native 组件 | GSI System Import 的核心输入 |
| vendor.img | Vendor HAL、Vendor 配置和厂商/虚拟设备实现 | 必须有兼容的 Vendor 内容，可能位于 super 内 |
| product.img / system_ext.img / odm.img | 额外系统组件、产品定制和设备扩展 | 视分区布局而定 |
| super.img | Dynamic Partitions 的容器，保存逻辑分区和 LP metadata | 仅适用于采用动态分区布局的镜像集 |
| vbmeta.img | AVB 验证元数据和分区描述符 | 按验证链和镜像方案提供 |
| userdata.img | Guest 应用、用户设置和数据 | 可由 ZeroVM 创建独立虚拟数据盘 |
| metadata.img | 部分设备的元数据/加密相关存储 | 取决于 Guest 加密和 fstab 方案 |
| dtbo.img | 设备树覆盖集合 | 仅在相应启动方案需要时提供 |
| recovery / vendor_dlkm / system_dlkm | Recovery 环境或 Kernel 模块 | 按 Guest Profile 和驱动加载方式决定 |

**注意：** 不要求每台 Guest 都有上表中的每一个独立文件。必须解析实际启动镜像、fstab、分区表、LP metadata 和启动配置，确定文件的真实职责与位置。

## 4. Linux Kernel 必须满足的条件

Guest Kernel 是 Android 虚拟设备的基础。它必须针对 VMM 暴露的虚拟硬件构建，而不是随便拿一台实体手机的 Kernel 来启动。

### 4.1 CPU、内存和中断

- ARM64 架构和正确的 Image 格式。
- 与 VMM 匹配的 Guest Physical Address 布局。
- 虚拟中断控制器、定时器和 SMP/vCPU 启动支持。
- 正确处理 Guest 内存区域和保留内存。
- 必需的内存管理、页表和 DMA/IOMMU 约束。

### 4.2 虚拟设备驱动

按实际暴露的设备启用并验证：

- VirtIO MMIO 或 VirtIO PCI（由后端和设备拓扑决定）；
- VirtIO Block：启动分区、系统分区和数据分区；
- VirtIO Console：早期内核控制台和诊断；
- VirtIO Net：Guest 网络；
- VirtIO Vsock：Host/Guest 控制与桥接；
- VirtIO Input：触控、键盘等输入；
- VirtIO GPU/DRM：图形设备（若采用该方案）；
- 其他明确实现的 VirtIO 设备。

设备驱动必须在 Guest 首次需要它之前可用。启动早期访问根文件系统所需的 Block 驱动，不能只放在一个尚未挂载的模块分区里。

### 4.3 Android Kernel 配置

按目标 Android 版本核对：

- Binder、Binder IPC、BinderFS（按系统配置）；
- Android 相关内存和同步机制；
- Device Mapper、dm-verity、dm-linear 等实际需要的目标；
- EXT4、EROFS、F2FS 等实际文件系统；
- Overlay/加密相关内核功能（按 Guest 数据方案）；
- SELinux、security hooks、seccomp 等安全能力；
- Android Common Kernel/GKI 对应的 ABI 和模块要求。

不能只因为 Kernel 能打印启动日志，就认为它已经具备运行 Android Framework 的内核条件。

### 4.4 Kernel 与模块版本匹配

- Guest Kernel、GKI 模块、Vendor 模块必须来自兼容的构建基线。
- 模块的 vermagic、符号版本、KMI 和依赖必须匹配。
- 模块加载路径必须在 first-stage init 可访问的分区或 Ramdisk 中。
- 模块签名和加载策略必须符合 Guest Kernel 安全配置。

优先以 Android Common Kernel 和 Cuttlefish 对应 Guest 平台为起点，构建 ZeroVM 专用 Kernel 配置和必要补丁；不要直接把任意手机的 vendor kernel 当作通用 Guest Kernel。

## 5. boot.img、vendor_boot.img 和 init_boot.img 的解析条件

### 5.1 必须解析 Boot Image Header

Boot Image Parser 至少要识别：

- Header version；
- Kernel size、Ramdisk size 和相关偏移；
- Page size（仅适用于对应格式字段）；
- Header、OS version、patch level 等元数据；
- DTB/DTBO 的实际存放方式；
- Vendor Ramdisk table 和 fragment 信息（适用版本）；
- Bootconfig 的存放和传递方式；
- 各段长度、对齐、校验和文件边界。

不能用固定偏移截取所有 Android 版本的 boot.img。Android 不同版本的 Boot Header 结构存在变化，解析器必须按 Header version 分派。

### 5.2 Ramdisk 不能遗漏

Ramdisk 可能包含：

- first-stage init；
- init 配置文件和早期服务；
- fstab；
- ueventd 配置；
- 模块加载清单；
- 早期 SELinux policy 或相关加载材料；
- 设备初始化脚本和挂载配置。

必须确定 Generic Ramdisk、Vendor Ramdisk、Recovery Ramdisk 的组合规则，并按目标 Guest Profile 组装。不能把不同设备的 Ramdisk 直接拼接。

### 5.3 启动参数

Boot Orchestrator 必须生成与 Guest 平台一致的：

- Kernel command line；
- Bootconfig；
- Android boot properties；
- Console/earlycon（用于诊断时）；
- 根设备、分区标识和启动模式参数；
- AVB/verity 验证链相关参数；
- Guest 虚拟设备标识和硬件 ABI 信息。

所有参数都必须由 Guest Profile、镜像元数据和实际 VMM 设备拓扑推导。禁止通过猜测固定写入 `root=/dev/vda`、固定分区号或其他设备专属参数。

## 6. DTB / Device Tree 必须与虚拟硬件一致

ARM64 Guest 通常需要设备树描述运行平台。DTB 至少应正确表达：

- CPU 节点、核数量和拓扑；
- Guest RAM 地址范围和保留区域；
- 中断控制器与中断号；
- Timer；
- VirtIO 设备类型、地址、中断和总线；
- Console；
- 其他实际暴露的虚拟设备。

DTB 不能照搬宿主手机的设备树：宿主物理设备地址、中断号、IOMMU、GPIO 和驱动节点不代表 Guest 虚拟设备。

ZeroVM 应实现或集成版本化 DTB Builder，并从同一份 Virtual Hardware Manifest 生成 VMM 设备配置和 Guest DTB，避免两边描述不一致。

## 7. system.img 与 Android 分区必须满足的条件

### 7.1 System 镜像格式

镜像导入时识别它究竟是：

- Android Sparse Image；
- Raw block image；
- EXT4 filesystem image；
- EROFS filesystem image；
- 其他明确支持的格式。

先解析再决定挂载/映射方式。不能仅凭 `.img` 扩展名判断格式，也不能把 Sparse 文件直接当作原始块设备内容。

### 7.2 分区和文件系统布局

需要明确：

- system 是独立分区还是动态逻辑分区；
- system 是否依赖 system-as-root/first-stage mount 方案；
- vendor、product、system_ext、odm 的存在和挂载关系；
- super 的 LP metadata、group、partition、extent 和容量；
- A/B slot、Virtual A/B 等适用布局；
- userdata、metadata、加密和 fs_mgr 配置；
- 分区 UUID、PARTUUID、逻辑分区名称与设备映射。

如果 system 位于 super.img 内，必须解析 LP metadata 后再映射逻辑分区。不能将 system.img 误当作 super.img。

### 7.3 GSI 特殊条件

导入 GSI 时还必须检查：

- GSI CPU ABI 与 Guest vCPU 架构一致；
- GSI 的 Android release/API 与 Vendor 接口兼容；
- Treble/VINTF 约束满足；
- 所需 VNDK/系统接口版本可用；
- Guest Kernel 满足 GSI 所需内核功能；
- Guest Vendor HAL 和虚拟设备驱动与 ZeroVM 虚拟硬件匹配；
- fstab、分区布局和 AVB 验证策略一致。

**GSI 不是通用完整 ROM。** 仅有 GSI system.img 时，ZeroVM 必须提供经过验证的 Guest Platform Base（Kernel + Boot/Ramdisk + Vendor/HAL + VINTF + 虚拟硬件支持）。不能宣称一份 Cuttlefish vendor.img 可以兼容所有 GSI。

## 8. Vendor、HAL 和 VINTF 必须满足的条件

Android Framework 启动后，需要找到与 Guest System 版本匹配的 HAL 和 Vendor 服务。

至少要明确：

- Vendor Manifest；
- Framework Compatibility Matrix；
- 需要的 HAL service/instance 和版本；
- HIDL/AIDL 接口版本；
- init rc 服务启动配置；
- SELinux domain、service_contexts 等安全策略；
- Kernel 驱动和 HAL 之间的设备接口；
- HAL 所依赖的 VirtIO 设备或 Host Bridge。

首个可启动基线可以先选择最小必需 HAL 集合，并明确不支持的功能。不能注册一个没有真实实现的 HAL 来假装设备可用。

软件 KeyMint 也不能宣称具备 StrongBox 或真实 TEE 的安全等级。

## 9. AVB、SELinux 与启动安全条件

- 解析 vbmeta、descriptor、验证链和 rollback metadata。
- 确定 Guest 使用何种可信根和密钥策略。
- 让 Guest Kernel、Ramdisk、System、Vendor 和逻辑分区遵循一致的验证策略。
- Guest SELinux policy 必须与服务、设备节点和 HAL 配置匹配。
- 记录验证成功、验证失败、签名未知、缺失元数据或验证关闭等状态。
- 不能把 `androidboot.selinux=permissive`、关闭 dm-verity 或忽略 AVB 错误作为通用启动修复。

若开发阶段需要测试密钥，也必须把它定义为明确的测试信任链，不能伪装成生产可信启动。

## 10. VMM 必须提供的最低运行环境

无论底层采用 AVF 还是其他已验证后端，Guest 启动前必须确认：

1. Guest vCPU 可创建并运行；
2. Guest RAM 已按预算分配且映射合法；
3. 中断控制器、Timer 和 CPU 拓扑正确；
4. Guest Kernel 加载地址、入口和架构符合后端要求；
5. DTB、Ramdisk、Bootconfig 和命令行正确传递；
6. Guest 需要的 Block 设备已创建并映射到正确镜像；
7. Console/日志通道可用；
8. Vsock/控制通道（若设计需要）可用；
9. 设备地址、中断号、feature negotiation 和 Guest 驱动一致；
10. VM 生命周期、退出状态和错误原因可由 Host 获取。

不同后端的启动 API 和镜像输入要求不同。AVF 的 VM 配置与 crosvm 命令行不能假定完全相同，必须由 Backend Adapter 转换。

## 11. 从 Kernel 到 Android 桌面的启动阶段

| 阶段 | 必须成功的条件 | 典型失败原因 |
|---|---|---|
| Backend Create | VM、vCPU、内存和设备创建成功 | 权限、后端、资源不足 |
| Kernel Entry | Guest Kernel 开始执行 | Kernel 格式/入口/架构错误 |
| Kernel Init | CPU、内存、中断、驱动初始化 | DTB、内存布局、驱动不匹配 |
| Early Userspace | first-stage init 启动 | Ramdisk、init、SELinux 问题 |
| First-stage Mount | 必需分区可访问 | fstab、Block 映射、文件系统问题 |
| Second-stage Init | 属性、服务和安全策略加载 | init rc、SELinux、属性配置 |
| VINTF/HAL | 必需 HAL 服务满足兼容性 | Vendor/HAL/VINTF 不匹配 |
| Zygote | Android Runtime 启动 | Framework、ART、内核能力问题 |
| System Server | Android 核心服务启动 | 系统服务依赖、资源不足 |
| SurfaceFlinger | 显示合成服务启动 | Gralloc/Composer/GPU 路径问题 |
| Launcher Ready | 桌面可显示和交互 | 显示输出、输入桥接问题 |

只有达到预先定义的 Guest Ready 标准，才可把 VM 标记为 RUNNING。仅仅看到 Kernel Console 或 init 日志不算 Android 系统启动完成。

## 12. ZeroVM 应实现的启动前检查器

建议实现 `GuestBootPreflight`，在真正创建 VM 前生成结构化报告：

| 检查器 | 输入 | 输出 |
|---|---|---|
| HostVirtualizationCheck | Host 系统、后端、权限 | 可用后端及限制 |
| HostResourceCheck | 内存、CPU、热、电量、存储 | 本次可分配资源预算 |
| ImageFormatInspector | 所有导入文件 | 格式、版本、哈希、结构 |
| BootImageInspector | boot/vendor_boot/init_boot | Header、Kernel、Ramdisk、Bootconfig |
| KernelCompatibilityCheck | Kernel、配置、模块 | 架构、驱动、KMI、功能缺口 |
| DeviceTreeValidator | DTB 和虚拟硬件 Manifest | 地址、中断、设备拓扑一致性 |
| PartitionLayoutAnalyzer | GPT、super、LP、fstab | 逻辑分区和挂载图 |
| AVBVerifier | vbmeta、分区镜像、密钥策略 | 验证链结果 |
| VintfCompatibilityCheck | System、Vendor、Matrix | HAL/接口兼容性 |
| GuestProfileValidator | 上述所有检查结果 | 可启动等级和阻塞项 |

检查结果不要只返回一个 boolean。建议输出：

- `PASS`：该项已验证通过；
- `FAIL`：明确不满足；
- `UNKNOWN`：材料不足或无法探测；
- `WARNING`：可启动但有已知功能限制；
- `NOT_APPLICABLE`：当前 Guest Profile 不需要该项。

最终给出启动等级：镜像可解析、Kernel 可启动、Android init 可启动、Framework 可启动、桌面可交互、声明功能通过。

## 13. 最小可行 Guest 平台建议

ZeroVM 不应第一步就尝试启动“任意 GSI”。建议先固定一套可追溯的参考 Guest Platform：

1. 选定一个目标 Android release/API 和 ARM64 ABI。
2. 选用与该版本匹配的 Cuttlefish Guest Kernel、Boot/Ramdisk、DTB 和 Vendor/HAL 作为参考材料。
3. 在 Linux/KVM/Cuttlefish 环境先验证完整 Guest 设备可启动。
4. 在 ZeroVM 目标 Android 设备上验证实际可用的 AVF/KVM 后端、权限和 Guest 启动能力。
5. 建立与 VMM 一致的 VirtIO 设备拓扑和 DTB。
6. 先启动 Guest Kernel 和 Console，再推进 first-stage init、分区挂载、Android Framework。
7. 先保证参考 Guest System 完整启动，再替换为一个明确版本的 GSI system.img。
8. 替换后逐项验证 VINTF、HAL、AVB、SELinux、图形和应用运行。
9. 将通过验证的组合保存为版本化 Guest Profile 和兼容性记录。

这条路线能把“VMM/Kernel 启动问题”和“GSI/Vendor 兼容问题”分开定位。

## 14. 启动输入清单模板

每个 Guest Profile 至少登记：

- Host backend 类型、版本和必要权限；
- Guest 架构、Android release/API；
- Kernel 来源、commit、配置、哈希和模块清单；
- boot/vendor_boot/init_boot 的 Header version 和哈希；
- DTB/DTBO 来源和校验结果；
- Ramdisk 组成、fstab、init rc 和 Bootconfig；
- system/vendor/product/system_ext/odm 等分区的格式、大小和哈希；
- super/GPT/LP metadata（若适用）；
- vbmeta/AVB 验证链和安全策略；
- Vendor Manifest、Framework Matrix 和 VINTF 检查结果；
- 虚拟硬件 ABI、设备 manifest 和 Guest 驱动清单；
- Guest RAM/vCPU 预算及其 Host 探测依据；
- 已通过的启动阶段、真机设备范围和已知限制。

没有这些可追溯信息的镜像组合，只能标记为未验证组合，不能宣称支持。

## 15. 相关文档

- [Guest Kernel 与启动链](06-guest-kernel-boot.md)
- [GSI、镜像与分区](07-gsi-images-partitions.md)
- [Vendor HAL 与 VINTF](08-vendor-hal-vintf.md)
- [虚拟硬件平台与 ABI](05-virtual-hardware.md)
- [宿主硬件发现与自适应资源](04-host-discovery-adaptive.md)
- [VM 完整生命周期](12-vm-lifecycle.md)
- [兼容性与测试](13-compatibility-testing.md)
