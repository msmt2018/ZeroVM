# 19. Android App、应用框架与 Runtime 设计

## 1. 职责边界

Android App 是管理和控制面，不是 VMM。它负责用户交互、权限申请、镜像导入、VM 配置、后台任务协调、状态展示、日志和诊断。Guest 执行由 Native Runtime/Backend 承担。

## 2. 推荐应用架构

采用按功能分层、按业务域拆分的模块化架构。初期不必为每个类建立 Gradle module，但边界必须先清晰；当模块具备独立依赖、构建收益或团队并行需求时再拆分。

建议模块：
- `app`：Application、导航、主题、入口和依赖组装。
- `core:model`：VM、镜像、Guest Profile、能力、错误等纯模型。
- `core:common`：Result、Dispatcher、时钟、日志接口、ID。
- `core:database`：Room 数据库、DAO、迁移。
- `core:datastore`：用户偏好和非关键设置。
- `core:runtime-api`：Kotlin Runtime API、事件和接口。
- `core:native-bridge`：JNI 封装、Native 加载和错误映射。
- `feature:dashboard`：VM 列表、状态、资源概览。
- `feature:vm-create`：Guest Profile、资源策略和 VM 创建。
- `feature:image-manager`：导入、扫描、校验和镜像仓库。
- `feature:console`：串口、启动日志、调试命令。
- `feature:display`：Guest 显示和输入界面。
- `feature:settings`：Backend、资源和诊断设置。

拆分依据是稳定业务边界，不是机械地“一屏一个模块”。

## 3. UI 技术选型

### Jetpack Compose
适用于全新 UI、状态驱动界面、复杂配置和响应式窗口布局。优点是状态和 UI 组合清晰；需要管理重组成本、稳定 key、生命周期收集和大日志列表性能。

### Android View
当需要接入 SurfaceView、TextureView、传统输入控件、特殊窗口/显示管线或已有稳定 View 组件时使用。Compose 可通过 AndroidView 承载，但显示 Surface 的创建/销毁必须和 VM 生命周期分离。

### 混合方案
推荐管理 UI 使用 Compose；Guest Display 根据后端输出选择 SurfaceView/TextureView/自定义 View。不能因为 Compose UI 已完成，就假设 Guest 图形缓冲区自动可显示。

## 4. 应用业务架构

使用单向数据流：
`UI Action → ViewModel → UseCase → Repository/Runtime API → Event/Result → UI State`

- ViewModel 仅管理界面状态和调用用例。
- UseCase 封装用户操作的业务规则。
- Repository 管理 VM 元数据、镜像索引和持久化。
- Runtime API 负责实际 VM 操作。
- Flow/StateFlow 传递状态和事件；不得用 UI 轮询替代 Runtime 事件通道。
- 长时间任务使用明确的 Operation ID、进度、取消和恢复策略。

## 5. VM 与镜像数据模型

VM 元数据至少包含：
- stable VM ID、名称、创建时间；
- Guest Profile ID/version；
- Backend ID 和最后一次能力快照；
- 镜像引用、哈希、格式和来源；
- 资源策略及最近一次实际分配结果；
- 当前持久化状态（仅为上次观察值）；
- 最近启动/退出原因；
- 独立 userdata、metadata、overlay 路径。

实时状态必须从 Runtime/Backend 查询并订阅。进程重启后不能仅凭数据库里 `RUNNING` 字段认定 VM 仍在运行。

## 6. Service 与进程生命周期

当 VM 需要在 Activity 离开后持续运行时，评估 Foreground Service、通知、Android 后台启动限制和设备厂商电源管理限制。服务是控制面，不保证底层 VM 在 Host 进程死亡后仍存在；这一点由 Backend 的真实生命周期决定。

需要实现：
- 前台服务启动/停止和通知；
- Activity 重连和状态恢复；
- Runtime 初始化幂等；
- 进程死亡后的后端状态重查；
- 用户主动停止、系统回收和异常退出的区分；
- 服务解绑、观察者释放和 Native Handle 生命周期。

## 7. 权限与平台能力协调器

`HostCapabilityRepository` 负责探测；`PermissionCoordinator` 负责解释和请求可由应用请求的权限；系统级授权缺失时必须说明不可通过普通运行时权限弹窗获得。

能力分层记录：
1. 硬件存在；
2. Kernel/驱动可用；
3. Android Framework 服务存在；
4. App 当前身份有权限；
5. Backend API 暴露能力；
6. Guest 驱动/HAL 实现能力；
7. 端到端测试通过。

## 8. 错误和事件模型

错误至少分域：HOST_PERMISSION、BACKEND_UNAVAILABLE、RESOURCE_UNAVAILABLE、IMAGE_INVALID、PROFILE_INCOMPATIBLE、BOOT_CONFIG_INVALID、GUEST_BOOT_FAILED、DEVICE_BRIDGE_FAILED、STORAGE_FAILURE、SECURITY_FAILURE、CANCELLED、INTERNAL.

事件应包含 VM ID、operation ID、monotonic sequence、timestamp、阶段、严重级别、错误码和可安全展示的上下文。不要把原始敏感路径、密钥或 Guest 私密数据写入普通日志。

## 9. 持久化选择

- Room：VM 配置、镜像元数据、Guest Profile 索引、操作历史。
- DataStore：界面偏好、用户默认选项。
- 文件系统：镜像、虚拟磁盘、overlay、快照、日志。
- 数据库只存文件引用、哈希、大小、状态和元数据；不把数 GB 镜像存入数据库。
- 数据迁移必须有版本号、事务和失败恢复。

## 10. 测试要求

- ViewModel/UseCase 单测：状态流、并发点击、取消和错误映射。
- Repository 测试：数据库迁移、镜像索引和损坏数据。
- Service 测试：重连、重复启动、进程重建。
- UI 测试：大屏/折叠/旋转、无权限、无镜像、启动失败。
- Runtime 集成测试：Fake Backend 和真实 Backend 分开标记。
