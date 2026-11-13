# 27. 虚拟手机硬件模型、固件材料与完整启动链

## 1. 设计目标

Guest 应被建模为完整虚拟设备实例（Virtual Phone Instance），而不是一份 system.img。它由虚拟 CPU/SoC、内存、固件、启动链、设备树、Kernel、设备模型、分区拓扑、Vendor/HAL 和安全策略共同构成。

纯软件虚拟化无法普遍 1:1 复刻任意手机的基带、DSP、ISP、GPU、TEE、传感器 Hub 和专有固件。ZeroVM 应定义可验证的虚拟硬件契约，并对不能复刻的能力受控降级。

## 2. Virtual Phone Board Model

层次包括 Compute（ARM64 vCPU、CPU feature mask、拓扑、计时器、中断控制器）、Memory（Guest RAM、地址空间、共享内存）、Firmware、Boot、Device Tree、Storage、Peripherals、Security 和 Platform Services。

设备清单描述设备 ID、模型版本、传输协议、feature、地址/中断分配、Guest driver 要求和 Backend 能力。

## 3. 启动材料

按 ROM 与 Android 版本选择，不能要求所有设备固定包含全部文件：
- boot.img：Kernel 与 ramdisk 的布局依 boot header 版本变化；
- vendor_boot.img：vendor ramdisk、DTB 等材料依版本变化；
- init_boot.img：较新 Android 启动架构中的通用 ramdisk 分区；
- dtbo.img / dtb.img：设备树材料；
- vbmeta.img、vbmeta_system.img 及 chained vbmeta：AVB 元数据；
- super.img：动态分区容器，逻辑分区因设备而异；
- system、system_ext、product、vendor、odm 及厂商自定义逻辑分区；
- userdata、metadata；
- misc、recovery、vendor_dlkm、odm_dlkm 等按启动/更新流程需要；
- fstab、init rc、模块清单、内核命令行、bootconfig、VINTF 和 SELinux policy。

文件名相同不代表布局和内容兼容。必须解析 boot header、分区表、LP metadata、AVB descriptor 和镜像元数据。

## 4. 启动路径

支持两种路径：
1. Bootloader/Firmware：虚拟固件 → 虚拟 Bootloader → 加载 Kernel/DTB/ramdisk → Kernel；
2. VMM direct boot：可信 VMM/Loader 直接加载已匹配的 Kernel、DTB、ramdisk 和参数。

不能把 pvmfw 等保护型虚拟机固件当作所有 Guest 的通用 Bootloader。启动方式由 Backend、安全模型和 Guest Profile 决定。

Android 启动阶段通常包括 Firmware/Loader、Kernel、first-stage init、early mount、AVB/dm-verity setup、second-stage init、SELinux policy、native services/HAL、servicemanager、Zygote、System Server 和 Launcher。具体顺序需按 Android 版本核对。

## 5. 分区拓扑模型

不能假设 super.img 是唯一存储布局。表达 GPT 分区、动态分区组、metadata slot、A/B 或 Virtual A/B、逻辑分区属性、独立镜像、文件系统、挂载点、加密、AVB 关系及每 VM userdata/metadata/snapshot/overlay。

引擎构建 Partition Graph，而不是硬编码 vda/vdb 名称或固定分区清单。

## 6. 虚拟硬件闭环

每个设备完成 Host backend、VMM device model、VirtIO/专用传输、Guest Kernel driver、DTB/设备描述、HAL 和 Framework 闭环。声明 virtio-gpu 或 virtio-blk 不会自动让 ROM 拥有对应驱动。

物理设备透传需单独评估 IOMMU、DMA 隔离、驱动所有权、权限和设备共享安全；手机 Host 通常不提供任意 GPU/ISP/基带 passthrough。

## 7. 32 位 ABI

CPU 是否支持 AArch32、Guest Kernel 是否启用 32 位执行、Android linker 是否包含 32 位运行时、ROM 是否包含 32 位 Bionic/系统库是不同层次。若 Host CPU 已不支持 AArch32，用户态翻译器还需验证指令覆盖、内核接口、信号处理、JIT、许可证和性能。用户态转译器不能替代 Guest Kernel。

## 8. 验收

自动验证材料完整性、分区图、启动链匹配、Kernel config/模块、DTB、VINTF、AVB、设备 ABI 和存储容量，并输出启动依赖图与缺失项。
