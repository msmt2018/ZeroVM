# 15. 开发路线、风险与技术决策

## 阶段路线

### Phase 0：平台可行性调查
选择代表性 ARM64 Android 测试设备；检查 AVF、VirtualizationService、权限和 Guest 支持；确认可用后端和最小 VM；建立 Linux/KVM/Cuttlefish 参考测试环境；形成 Host Capability 报告。

退出条件：至少一个目标真机能通过受支持路径创建真正 Guest VM，或明确记录平台阻塞和替代路线。

### Phase 1：最小 Guest Kernel
准备 ARM64 Guest Kernel、DTB、Console；建立 vCPU、RAM、VirtIO Console/Block/Vsock；完成 Kernel 启动和关闭。

退出条件：独立 Guest Kernel 稳定启动，Host/Guest 通信可用。

### Phase 2：完整 Android Guest Base
使用匹配的 Cuttlefish/虚拟设备 Guest Platform；完成 Boot、Vendor、分区和 userdata；启动 Android init、Zygote、System Server；提供 ADB/Console 诊断。

退出条件：完整 Android 用户空间进入可验证 Ready 状态。

### Phase 3：GSI 导入与兼容性
实现 Sparse/Raw、Boot、Super、LP、AVB 解析；建立 GSI System Import；选择匹配 Guest Profile；完成 VINTF、fstab、Kernel 和 Vendor 检查。

退出条件：至少一个明确版本和架构的 GSI 在指定 Guest Platform 上通过 L5 验收。

### Phase 4：显示与输入
完成 Guest Gralloc/Composer/GPU 路径；实现帧传输和 Surface 生命周期；完成触控、键盘、旋转和窗口尺寸变化。

退出条件：Guest Launcher 稳定显示并可交互。

### Phase 5：设备桥接
网络、音频、相机、传感器、电源和振动按优先级实现，并加入权限、隐私和断连恢复。

退出条件：每个声明支持的 Bridge 有端到端测试。

### Phase 6：存储、快照和多 VM
实现 CoW 磁盘、Factory Reset、Flash、Snapshot、多 VM 资源协调、崩溃恢复和数据一致性。

退出条件：重复操作和异常场景不会破坏其他 VM 或 Host 数据。

### Phase 7：兼容性与产品化
扩展 Android Profile，执行适用的 CTS/VTS 和自定义测试，完成性能、热、内存和稳定性优化，形成设备支持矩阵。

## 主要风险

1. 宿主虚拟化不可用：设备不提供 AVF、KVM 或所需权限。必须尽早验证。
2. Guest GSI 不匹配：GSI 与 Kernel、Vendor、VINTF 或分区布局不兼容。
3. 图形链路复杂：VirtIO GPU 不自动提供完整 Gralloc/Composer。
4. 普通 App 权限不足：系统虚拟化 API 和设备节点可能受限。
5. 资源压力：Host 与 Guest 竞争内存、GPU 和热预算。
6. Android 版本差异：Boot Image、HAL、VINTF、Kernel ABI 和分区规则演进。
7. 设备桥接安全：Guest 请求可能滥用 Host 摄像头、麦克风、文件或网络。
8. 上游依赖复杂：AOSP/crosvm 构建依赖庞大，许可证和补丁维护成本高。

## 关键技术决策

- 优先 AVF 作为 Android 上可用的官方后端，但必须做能力/权限检测。
- crosvm 作为 VMM 参考和可复用实现，不默认能作为普通 App 自行启动的进程。
- 先完成完整虚拟设备 Guest Base，再尝试替换 GSI System。
- 统一虚拟硬件 ABI，Guest Profile 按 Android 版本管理。
- 先 Console/ADB，再完整图形和设备桥接。
- 所有镜像写入只针对 VM 虚拟磁盘。
- 不默认关闭 AVB/SELinux。
- 不承诺任意 GSI、任意手机和所有外设都兼容。

## 研发优先级

最高优先级不是 UI、快照或多 VM，而是验证目标真机是否能合法、稳定地创建并运行独立 Guest Kernel。这是整个产品的先决条件。