# 06. Guest Kernel、固件与 Android 启动链

## 启动目标

ZeroVM 必须启动独立 ARM64 Guest Kernel，并由该 Kernel 启动 Android 用户空间。不能把 Host Kernel 上的 chroot/proot 进程称作 Guest Kernel。

## 启动材料

按 Android 版本和设备配置，可能涉及 Kernel Image、DTB/DTBO、boot.img、vendor_boot.img、init_boot.img、Generic/Vendor Ramdisk、Bootconfig、fstab、AVB metadata/vbmeta.img、分区布局和动态分区 metadata。

并非每个版本都同时具有上述独立镜像。Boot Image Header 版本、Ramdisk 放置位置和 Vendor Boot 结构必须按版本解析。

## 启动流程

1. Host 检查虚拟化后端和资源预算。
2. Image Engine 校验镜像、分区布局和安全策略。
3. Boot Orchestrator 解析 Guest Boot Profile。
4. Backend 创建 Guest 内存、vCPU 和虚拟设备。
5. VMM 加载 Kernel、DTB、Ramdisk 和启动参数，或按后端规定交给固件。
6. Guest Kernel 初始化 CPU、内存、中断、驱动和设备总线。
7. Kernel 启动 first-stage init。
8. First-stage init 读取 fstab，初始化必要的 Device Mapper/逻辑分区并挂载早期分区。
9. Second-stage init 加载属性、SELinux policy、服务配置并启动系统服务。
10. VINTF/Service Manager 完成 HAL 和服务发现。
11. Zygote、System Server、SurfaceFlinger 等关键组件启动。
12. Launcher 就绪，VM 进入 RUNNING。

每一步需要阶段标记、超时策略、日志采集和可诊断错误。

## Kernel 基线

优先研究 Android Common Kernel、GKI 和 Cuttlefish 对应版本的 Kernel 配置与补丁。Kernel 版本必须与目标 Guest Android、HAL 和启动材料匹配。不能简单认定某个新版本 Kernel 可无条件启动所有旧 GSI。

## Kernel 配置与驱动

至少评估 ARM64、VirtIO、Block、Net、Console、Vsock、Input、DRM/GPU、DMA-BUF、Sync、EXT4、EROFS、F2FS、Device Mapper、dm-verity、Binder 和必要 Android 内核配置。

驱动编入内核还是作为模块，取决于 first-stage init 加载时机和分区位置。启动早期所需驱动不能放在尚未挂载的分区中。

## Guest Boot Profile

Profile 描述 Android release/API level、Kernel 版本来源和哈希、Boot Image Header 版本、DTB/DTBO、Ramdisk 组成、Bootconfig/cmdline、fstab、分区布局、AVB 策略、SELinux policy、所需 VirtIO 设备和 feature、VINTF/HAL 约束。

Profile 必须可校验、可追溯，不允许仅根据文件名猜测参数。

## Bootloader 与固件边界

VMM 可以直接加载 Kernel、DTB 和 Ramdisk，因此 ZeroVM 不必模拟任意手机的 Boot ROM 和物理 Bootloader。若使用 AVF pVM，还必须遵循其固件、安全启动和镜像格式约束。pvmfw 不是所有普通 Guest 的通用 Bootloader。

## 启动失败分类

至少区分后端创建失败、Kernel 未启动、DTB 不匹配、驱动缺失、initramfs 错误、fstab 错误、分区挂载失败、AVB 验证失败、SELinux 拒绝、VINTF 不兼容、HAL 启动失败、Zygote/System Server 崩溃和图形服务失败。