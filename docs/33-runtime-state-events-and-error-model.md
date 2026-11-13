# 33. Runtime 状态机、事件流与统一错误模型

## 1. 目标

本文定义跨 Kotlin/JNI/Native/Backend 的状态、操作事件和错误语义，使 UI、Service、诊断和自动化测试观察到同一事实。Backend API 返回成功不等于 Guest Android 已启动。

## 2. VM 生命周期状态

主状态建议为 CREATED、PREFLIGHT、PREPARED、STARTING、RUNNING、STOPPING、STOPPED、FAILED、DESTROYING、DESTROYED。

PAUSED 仅在 Backend 明确支持且状态一致性验证通过后启用。SUSPENDED、SNAPSHOTTED、RESTORING 作为独立能力状态/操作，不假定所有后端支持。

Guest 健康度与 VM 生命周期分离：
- UNKNOWN；
- KERNEL_BOOTING；
- ANDROID_BOOTING；
- FRAMEWORK_READY；
- UI_READY；
- DEGRADED；
- UNHEALTHY。

RUNNING + ANDROID_BOOTING 是合法组合；Backend RUNNING 不等于 UI_READY。

## 3. 状态转换约束

- create：CREATED → PREFLIGHT → PREPARED；失败回到 CREATED 或进入 FAILED，并保留原因。
- start：PREPARED/STOPPED → STARTING → RUNNING；失败后重新查询 Backend，确认最终实例状态。
- graceful shutdown：RUNNING → STOPPING → STOPPED。
- force stop：RUNNING/STOPPING → STOPPED，记录强制终止原因和数据一致性风险。
- destroy：STOPPED/FAILED/CREATED → DESTROYING → DESTROYED。
- 运行中 destroy 必须先 stop，除非 Backend 提供经验证的原子 destroy。
- 并发操作必须串行化或明确拒绝；start 与 destroy 不得并行修改同一 VM。
- 状态变化使用单调递增 state_revision，客户端忽略旧 revision 事件。

状态机以 transition table 和单元测试为准，不能只依赖 UI 按钮禁用。

## 4. Operation 状态

耗时操作状态为 ACCEPTED、QUEUED、RUNNING、WAITING_FOR_BACKEND、CANCELLING、SUCCEEDED、FAILED、CANCELLED、TIMED_OUT。终态不可逆。重试创建新 operation_id，可携带 idempotency_key。

操作状态与 VM 状态分离：start operation 失败后，VM 最终状态仍需向 Backend 查询确认。进度按阶段报告；未知总量时不伪造百分比。

## 5. 统一事件封装

事件至少包含 event_schema_version、event_id、vm_id（Runtime 全局事件可为空）、runtime_generation、sequence、timestamp、operation_id（可为空）、source_layer、event_type、severity、payload、correlation_id/causation_id（可选）。

sequence 在单个 VM generation 内单调递增。进程重启后 generation 变化，客户端应重新获取完整 Snapshot 再订阅。

事件类型至少包括 VM_STATE_CHANGED、OPERATION_ACCEPTED/PROGRESS/COMPLETED、BACKEND_CAPABILITY_CHANGED、GUEST_BOOT_STAGE、GUEST_HEALTH_CHANGED、DEVICE_ATTACHED/DETACHED/DEGRADED、RESOURCE_PRESSURE、BRIDGE_CONNECTED/DISCONNECTED、IMAGE_IMPORT_PROGRESS、DIAGNOSTIC_AVAILABLE、RUNTIME_FATAL_ERROR。

事件是观测，不是唯一事实存储。重连时先获取 Snapshot，再从可用 sequence 续订；历史不可用时返回 GAP_DETECTED。

## 6. 统一错误结构

错误至少包括 domain、稳定 code、message、developer_message、retryability、operation_id、vm_id、stage、受限长度 cause_chain、脱敏 context、timestamp 和 recovery_hint。

错误域建议：VALIDATION、IMAGE、PROFILE、HOST、PERMISSION、BACKEND、BOOT、DEVICE、BRIDGE、STORAGE、SECURITY、RESOURCE、INTERNAL。

不得只返回字符串，也不得把原始异常堆栈直接显示给普通用户。

## 7. 错误码规则

错误码采用稳定域+名称，不复用已发布编号。示例：
- VALIDATION_INVALID_ARGUMENT；
- VALIDATION_SCHEMA_UNSUPPORTED；
- IMAGE_FORMAT_UNKNOWN；
- IMAGE_TRUNCATED；
- IMAGE_RESOURCE_LIMIT；
- PROFILE_INCOMPLETE；
- PROFILE_INTERFACE_MISMATCH；
- HOST_VIRTUALIZATION_UNAVAILABLE；
- HOST_SERVICE_UNAVAILABLE；
- PERMISSION_DENIED；
- BACKEND_UNSUPPORTED_CAPABILITY；
- BACKEND_CREATE_FAILED；
- BACKEND_OPERATION_TIMEOUT；
- BOOT_KERNEL_ENTRY_FAILED；
- BOOT_INIT_FAILED；
- BOOT_FRAMEWORK_TIMEOUT；
- DEVICE_NEGOTIATION_FAILED；
- BRIDGE_PERMISSION_REVOKED；
- STORAGE_NO_SPACE；
- STORAGE_INTEGRITY_FAILED；
- SECURITY_POLICY_VIOLATION；
- RESOURCE_BUDGET_EXCEEDED；
- RESOURCE_PRESSURE_ABORTED；
- INTERNAL_UNEXPECTED。

具体数字由代码中的单一 error registry 分配，不能由不同模块自行映射成不一致整数。

## 8. 重试、取消和恢复

- 仅 retryability=SAFE 的错误可自动重试。
- 权限拒绝、镜像损坏、接口不匹配和安全策略失败不能盲目重试。
- 重试前刷新 VM/Backend 状态，避免重复 create/start。
- 取消必须说明底层是否真正中断、是否产生部分输出。
- 镜像导入和分区转换采用临时产物、校验和原子提交。
- 启动失败保留诊断证据，不自动删除用户数据。
- 强制停止后标记 Guest 数据一致性未知，后续启动执行必要检查。

## 9. 日志关联与隐私

日志至少带 timestamp、level、source、vm_id、operation_id、sequence 和 event/error code。跨层调用传播 correlation_id。

禁止记录私钥、密钥材料、认证 token、Guest 用户数据内容、完整镜像内容、未脱敏敏感设备标识，以及 Camera/Microphone/Location 原始数据。诊断导出由用户确认，展示包含项并支持脱敏。

## 10. 测试与验收

测试状态转换全路径和非法转换、并发 start/stop/destroy、operation 终态不可逆、事件重复/乱序/丢失/重连/generation、错误码跨语言映射、retryability、Backend 成功但 Guest 未启动、强制停止和进程死亡恢复。

任何 VM 操作必须有 operation_id、最终状态和结构化错误；状态变化有 state_revision；事件断流后可通过 Snapshot 恢复；Backend RUNNING 与 Android UI_READY 明确区分；重试不会重复创建实例；日志可跨层关联且不泄露密钥或 Guest 数据。