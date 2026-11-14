# 04. 宿主硬件发现与自适应资源管理

## 目标

ZeroDroid 不采用适用于所有手机的固定 CPU、RAM、GPU 和磁盘配置。每台 Host 基于真实能力生成 VM 配置，并在运行期间根据负载、内存压力、温度、电量和多 VM 竞争调整资源预算。

## 能力分层

每项能力分别记录：
1. PhysicalPresent：物理硬件是否存在。
2. KernelDriverAvailable：宿主内核驱动是否可用。
3. FrameworkAvailable：Android Framework/API 是否提供能力。
4. PermissionGranted：当前应用是否获准使用。
5. BackendSupported：VM 后端是否支持虚拟化该设备。
6. GuestImplementationAvailable：Guest Kernel/HAL 是否实现。
7. Validated：端到端测试是否通过。

不能将“宿主有 GPU”直接等同于“Guest GPU 加速可用”。

## 探测维度

- CPU：架构、核心拓扑、频率范围、异构核心、调度策略和虚拟化能力。
- 内存：总量、可用量、MemAvailable、压力、低内存阈值和系统保留。
- GPU：厂商/型号、驱动、EGL/Vulkan 版本、扩展、设备节点权限和共享 Buffer 能力。
- 存储：文件系统、剩余空间、配额、I/O 特征和镜像格式支持。
- Kernel：版本、配置、模块、设备节点、Binder、KVM、VirtIO 能力。
- 虚拟化：AVF 服务、API 级别、权限、支持的 Guest 类型和实际创建能力。
- 显示：分辨率、密度、刷新率、色彩能力、旋转和窗口尺寸。
- 热与电源：Thermal Status、可获得温度、电池、充电和省电模式。
- 外设：输入、音频、相机、传感器、蓝牙和网络。

普通应用不能读取所有底层传感器或内核信息。探测器必须遵守公开 API、权限和厂商限制；不可读取应标记 Unknown，而不是推测。

## 配置生成

配置生成分为静态能力协商和动态资源调度。

静态能力协商在 VM 创建前确定设备拓扑、Guest 可见设备、最大 vCPU、最大 RAM、显示模式和可用 Bridge。动态调度在运行期间调整 CPU 配额、调度权重、可回收内存、图形负载、I/O 限速和后台优先级。

启动后不能任意改变所有虚拟硬件。设备拓扑、CPU 数量、RAM 容量、GPU 模式等可能被 Guest 启动流程或驱动初始化锁定。

## 资源预算

预算必须为 Host Android、系统服务、ZeroDroid Runtime、VM Backend 和安全余量保留空间。不能按 Host 总 RAM 的固定百分比盲目分配。

预算考虑 MemAvailable 和内存压力、同时运行的 VM 数量、Guest 最低启动需求、后端开销、Host 负载、热状态、电量、图形 Buffer 和共享内存。所有预算设置硬上限和硬下限。超过预算时拒绝创建、降低可选功能或回收可回收资源，不得无限增长。

## CPU 调度

使用后端支持的 vCPU 数量和调度控制。考虑大小核异构、前台/后台状态、宿主调度器和热限制。不能假定应用能够将 vCPU 固定绑定到指定物理核心，也不能把宿主 CPU 频率控制当作普通应用能力。

## 内存动态调整

Guest RAM 动态回收必须依赖 VMM 和 Guest Kernel 的明确支持，例如 VirtIO Balloon 或 VirtIO Mem。回收前确认 Guest 可回收页面和驱动状态。若后端不支持热调整，则运行期间只调整调度预算，不改变已建立的 RAM 映射。

## GPU 与显示降级

优先使用验证通过的硬件加速虚拟 GPU，其次使用可用 gfxstream/兼容后端，再考虑软件渲染，最后提供 headless/console 模式。降级必须明确告知用户，不能把软件渲染伪装成 GPU 直通。

## 热与功耗

根据 Android Thermal API 和可访问的电源状态调节 VM 资源预算。温度数据不可用时使用系统 Thermal Status 等公开信号。高热状态降低后台 VM 调度权重和可选图形负载；严重状态可暂停或要求关机。不能通过读取不到的温度值推断设备安全。

## 输出与更新

HostCapabilitySnapshot 带时间戳、来源、可信度和版本。静态硬件变化较少，动态资源状态按需更新并去抖。每次策略决策记录原因、旧值、新值和是否成功。