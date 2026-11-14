# 01. 项目目标与范围

## 项目定位

ZeroDroid 是面向 Android 宿主设备的完整 Android 系统虚拟机项目。目标是在支持的 Android 手机上创建隔离的 ARM64 Guest 设备，运行独立 Android Kernel 和完整 Android 用户空间，并允许导入符合兼容条件的 Android GSI。

ZeroDroid 的产品对象是虚拟 Android 设备，不是 Android 应用容器，也不是把另一个系统的界面嵌入当前应用。

## 核心目标

- 运行独立 Guest Kernel、Android init、System Server、Framework 和应用进程。
- 支持导入官方或第三方 GSI，并在兼容性检查通过后启动。
- 提供一致、版本化的虚拟硬件 ABI。
- 根据宿主设备实际能力生成虚拟硬件配置和资源预算。
- 每台 VM 拥有独立系统盘、数据盘、密钥空间、设备身份和运行状态。
- 提供创建、启动、运行、暂停、恢复、重启、关机、销毁等完整生命周期。
- 提供图形显示、触控输入、网络和可选宿主设备桥接。
- 对不支持的能力显式降级和诊断，不伪造成功状态。

## 真机体验的定义

真机体验是产品体验目标，不意味着虚拟设备在物理硬件、TEE、基带、摄像头 ISP 或 GPU 驱动层面与任意真实手机完全等价。

目标分层：
1. 系统完整性：Guest 启动完整 Android Framework 和应用运行时。
2. 设备一致性：Guest 看到稳定的虚拟设备拓扑和匹配 HAL。
3. 交互完整性：显示、触控、键盘、网络和音频可用。
4. 应用兼容性：普通 APK 在 Guest API、ABI 和权限范围内运行。
5. 安全隔离：Guest 与 Host 的内存和执行环境隔离，桥接权限最小化。
6. 性能适配：利用宿主资源，同时保护宿主系统稳定。

## 非目标

- 不重新实现 Android Framework、ART、Package Manager 等完整 AOSP 组件。
- 不自行开发 Hypervisor；优先使用设备提供的 AVF/KVM/pKVM。
- 不承诺所有 Android 手机都能创建 VM。
- 不承诺任意 GSI 可直接启动。
- 不将宿主真实分区刷写作为 VM 刷机功能。
- 不把 Root、Shizuku 或普通应用权限等同于 AVF 系统权限。
- 不以关闭 SELinux、AVB 或绕过安全策略作为通用兼容方案。

## 关键原则

**宿主适配**：每台手机单独探测能力，按真实可用资源生成配置。

**Guest 独立**：Guest Kernel、系统服务、Binder 域、数据目录和身份独立。

**兼容性诚实**：GSI、Kernel、Vendor、VINTF、HAL 和虚拟硬件必须共同兼容。

**后端解耦**：VM 管理、镜像、Guest Profile 不依赖某个特定 VMM。

**可验证**：每项能力都有自动化测试和明确验收条件。

## 成功标准

指定 Host 和 Guest Profile 必须完成：创建真正的硬件虚拟化 Guest、独立 Kernel 启动、Android init/Zygote/System Server 正常、Launcher 显示并可交互、应用安装运行、userdata 持久化、正常重启关机、异常恢复，以及 Host/Guest 隔离测试。