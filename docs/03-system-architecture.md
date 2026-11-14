# 03. ZeroDroid 总体软件架构

## 分层

1. Android Application：VM 管理、镜像导入、显示容器、输入、日志和设置。
2. Application Service Layer：前台服务、权限管理、持久化任务、通知和 UI 生命周期解耦。
3. JNI / Native Runtime：Kotlin 与 Native Core 的稳定边界，提供句柄、事件和错误码。
4. VM Core：VM 配置、状态机、生命周期编排、资源协调和诊断。
5. Virtualization Backend：AVF Backend，以及经验证的 Native crosvm/KVM Backend。
6. Virtual Hardware Platform：CPU、RAM、Block、Net、Vsock、GPU、Input、Audio 等设备模型。
7. Guest Platform：Kernel、DTB、Boot、Vendor HAL、VINTF、SELinux。
8. Image & Storage Engine：镜像解析、分区布局、虚拟磁盘、快照和刷写。
9. Host Device Bridge：经过权限控制的网络、音频、相机、传感器等桥接。

安全、日志、监控、兼容性和测试横切所有层。

## 模块边界

- UI 不直接操作 VMM 或设备节点。
- JNI 不承担长期 VM 状态的唯一存储。
- VM Core 不依赖 Activity 实例。
- Image Engine 不依赖某个 VMM。
- Guest Profile 描述设备和系统要求，不执行宿主特权操作。
- Backend 负责把统一 VM 配置转换为具体 AVF/crosvm 配置。
- Device Bridge 不允许 Guest 获得任意 Host 文件或设备访问。

## 核心接口能力

统一后端接口至少覆盖 capability discovery、create、start、pause/resume（若支持）、shutdown、forceStop、reboot、destroy、state query、console/event stream、metrics 和 snapshot（仅在后端明确支持时）。

接口返回结构化错误，包括错误域、错误码、可恢复性、用户提示和诊断上下文。不能用布尔值掩盖权限错误、资源不足、镜像错误和后端不支持等原因。

## VM Handle 与并发

每台 VM 使用独立稳定 VM ID 和 Native Handle。Handle 必须防止悬空引用、重复释放和跨 VM 混用。JNI 回调不能在持有 Native 内部锁时直接调用任意 UI 代码。

事件建议包括 VM_STATE_CHANGED、BOOT_PROGRESS、CONSOLE_OUTPUT、DEVICE_EVENT、RESOURCE_PRESSURE、GUEST_HEALTH、ERROR 和 EXITED。事件带 VM ID、单调递增序号、时间戳和结构化 payload。

## 配置模型

配置至少分为 HostCapabilitySnapshot、VirtualHardwareProfile、ResourceBudget、GuestBootProfile、GuestSystemProfile、StorageLayout、SecurityProfile 和 BackendConfig。每份配置包含 schema_version、校验规则和迁移策略。

## 持久化与运行态

持久化配置与瞬时运行态分开保存。App 被系统回收或 Activity 重建时，管理服务应重新查询真实后端状态并恢复 UI 展示，不能根据旧数据库状态假设 VM 仍在运行。

## 可观测性

每个模块输出结构化日志、指标和诊断事件。日志区分 Host、VM、Guest、Backend 和 Bridge，并避免记录密钥、认证数据或敏感内容。