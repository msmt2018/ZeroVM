# 24. 安全、资源管理与 Host/Guest 桥接框架

## 1. 安全边界

安全模型覆盖 Android App、Native Runtime、VMM/Hypervisor、Guest Kernel、Guest Android、虚拟磁盘、共享内存、Host Bridge 和日志。每条跨边界数据路径都必须定义身份、授权、验证、限流和失败行为。

## 2. 隔离原则

- Guest 使用独立地址空间和虚拟设备，不直接访问 Host 内核对象。
- 只暴露明确允许的共享内存和设备通道。
- Guest 数据、密钥、userdata 和快照按 VM 隔离。
- Host 权限由 Host Service 管理，Guest 请求不能直接扩大权限。
- 设备桥接默认最小权限，逐设备授权。
- 不将 Host TEE/StrongBox 能力无条件继承给 Guest。

## 3. 密钥和信任

为 VM 分配独立身份。区分 Guest AVB 信任根、Guest Keystore/KeyMint、Host 应用加密密钥和备份加密密钥。密钥不能写入普通日志、配置明文或共享目录。软件 KeyMint 只能声明软件安全级别。

## 4. 资源管理框架

`HostResourceSnapshot` 采集可用 RAM、内存压力、CPU topology/load、温度、电量、省电状态、存储容量/I/O、GPU 能力、后端限制和其他 VM 负载。

`ResourcePlanner` 根据：
- Host 实时可用资源；
- Android 系统保留量；
- VM 数量和优先级；
- Guest Profile 最低需求；
- Backend 硬限制；
- 热/电量/内存压力策略；
生成资源预算和降级建议。

`ResourceGovernor` 运行期间持续观察并按策略调整可调整资源；不可热调整的资源必须在启动前确定。每个动态参数都要有硬上限、变化速率限制和安全回退。

## 5. 内存管理

区分 Host process memory、Guest physical memory、共享缓冲区和文件页缓存。记录分配、映射、回收和峰值。内存预算必须防止 Host OOM；压力状态下优先停止可选缓存/设备能力，再按用户策略暂停或停止低优先级 VM。不能在不确定后端语义时随意 balloon。

## 6. 温度、电量和调度

使用系统可获得的 Thermal/Power 状态；不可用时标记 UNKNOWN。过热时减少可选渲染/后台工作，必要时安全关机。不能假设所有设备都允许持续高负载运行 VM。

## 7. Device Bridge 框架

每种桥接设备包含：
- Capability declaration；
- Permission gate；
- Protocol/version；
- Request/response/event；
- Identity and authorization；
- Timeout/cancellation；
- Rate limit/backpressure；
- Reconnect/revocation；
- Audit and diagnostics。

Camera、Microphone、Location、Sensors、USB、Clipboard 和 Network 都有独立隐私/权限要求，不能通过一个通用“桥接已连接”授权全部设备。

## 8. 共享内存

共享内存仅用于明确用途，记录 buffer 大小、访问方向、同步机制、生命周期和清零策略。检查边界、偏移、长度和整数溢出。Guest 输入视为不可信；不能直接信任 Guest 提交的 buffer metadata。

## 9. 快照与恢复安全

快照可能包含 Guest 内存、密钥、用户数据和会话令牌。需要加密、访问控制、完整性校验、版本兼容检查和安全删除策略。仅当 Backend 明确支持一致性快照时开放内存快照。

## 10. 安全测试

测试越权设备访问、恶意镜像解析、Guest 输入畸形、共享内存越界、Bridge 断连、权限撤销、日志泄密、快照篡改、磁盘路径穿越、Host 进程崩溃和多 VM 数据串扰。
