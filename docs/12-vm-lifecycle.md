# 12. Android Guest VM 完整生命周期

## 生命周期目标

生命周期覆盖镜像导入前能力检查、VM 创建、启动、运行、暂停、恢复、重启、关机、崩溃恢复、快照和销毁。UI 生命周期不能决定 VM 生命周期。

## 状态模型

NEW、INSPECTING、PREPARING、READY、CREATING、STARTING、BOOTING、RUNNING、PAUSING、PAUSED、RESUMING、REBOOTING、SHUTTING_DOWN、STOPPING、STOPPED、CRASHED、RECOVERING、DESTROYING、DESTROYED、ERROR。

状态转换由集中式状态机管理，禁止 UI 任意设置状态。每次转换记录原因、触发者、时间、后端结果和状态序号。后端不支持的状态操作必须返回 UNSUPPORTED，不伪造成功。

## 创建流程

1. 分配稳定 VM ID。
2. 读取 Host Capability Snapshot。
3. 检查虚拟化后端、权限和设备支持。
4. 选择 Guest Profile。
5. 检查 GSI/完整镜像格式和哈希。
6. 解析 Boot、分区、AVB、VINTF 和文件系统。
7. 计算硬件能力和资源预算。
8. 生成虚拟硬件 manifest、DTB 和启动配置。
9. 创建 VM 专属磁盘、userdata、metadata 和密钥空间。
10. 保存配置快照和镜像来源信息。
11. 校验成功后进入 READY。

创建流程支持取消和失败回滚。

## 启动流程

1. 重新检查 Host 当前能力和资源预算。
2. 验证 VM 配置与 Guest Profile。
3. 验证磁盘和快照一致性。
4. 初始化 Bridge 和安全通道。
5. 创建 VMM/AVF VM。
6. 分配 Guest RAM、vCPU 和设备。
7. 挂载虚拟磁盘并加载启动材料。
8. 启动 Guest Kernel。
9. 监测 Kernel、init、HAL、Zygote、System Server。
10. 满足 Guest Ready 条件后进入 RUNNING。

VMM start 成功不等于 Android 启动成功。必须定义 Guest Ready 探针，例如系统服务状态、启动属性、Launcher/SurfaceFlinger 可用性和健康检查。

## 运行阶段

持续采集 VM 状态和退出原因、vCPU 时间（可获得时）、Guest/Host 内存预算和压力、磁盘容量/I/O 错误、GPU/显示状态、Bridge 连接、Guest 健康探针和 Host Thermal/Power 状态。监控频率必须受控，避免自身造成明显性能损耗。

## 暂停与恢复

暂停可能表示后端暂停 vCPU/VM，也可能表示 Guest 内部通过 Android 系统停止活动，两者不能混淆。若 AVF 后端不提供任意暂停/恢复能力，API 应返回 UNSUPPORTED。恢复前检查后端句柄、磁盘、共享资源和 Host 能力。

## 重启

优先通过 Guest Android 正常重启接口触发。超时后按策略停止并重新创建。重启保留 userdata 和 VM 配置，记录 reboot reason，并检测启动循环。

## 正常关机与强制停止

正常关机请求 Guest 完成服务停止、文件系统同步和卸载。超时后进入 STOPPING 并调用后端强制停止。强制停止可能造成文件系统损坏，下一次启动前执行一致性检查或恢复流程。

## 崩溃恢复

Backend 意外退出、Host 进程死亡或设备重启后，查询真实后端状态、标记退出原因、检查磁盘锁和写入状态、校验 overlay 一致性、清理遗留资源，并决定进入 STOPPED、RECOVERING 或 ERROR。不能根据本地数据库旧状态推断 VM 仍在运行。

## 快照与克隆

区分磁盘快照、内存快照和应用层备份。只有后端明确支持且验证一致性时才提供 VM 内存快照。磁盘快照处理写缓存、Guest quiesce、overlay 合并和崩溃恢复。克隆时生成新的 VM ID、设备身份、密钥和独立可写数据层。

## 销毁

先停止 VM，再释放后端设备、共享内存、临时文件和锁；之后按用户选择删除或保留磁盘、快照、日志和导入镜像。清理操作必须可重试且幂等。

## 幂等性与并发

同一 VM 的 start/stop/reboot/destroy 操作需要串行化或使用操作令牌。重复请求返回当前操作结果或明确冲突，不得创建两个后端实例。状态转换和资源清理必须幂等。