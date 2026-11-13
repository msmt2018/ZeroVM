# 18. 工程架构与技术选型总规范

## 1. 文档目的

本文将 ZeroVM 从概念设计推进到可执行的工程架构决策。每项技术选型必须回答：解决什么问题、适用条件、必要前提、替代方案、限制、集成边界、验证方式和退出/替换成本。

本文是技术决策入口，不替代各子系统的详细接口规范。任何组件不得仅因“流行”而引入框架，也不得把设计候选误写为已实现能力。

## 2. 架构决策原则

1. **真实能力优先**：先探测 Host OS、SoC、虚拟化后端、驱动、权限和资源，再选择运行路径。
2. **分层且可替换**：UI、应用服务、Runtime API、VM Core、Backend、Guest Platform 之间使用明确契约。
3. **复用成熟实现**：优先复用 AVF、KVM、crosvm、VirtIO、AOSP、Android Common Kernel、Cuttlefish。
4. **不自研 Hypervisor**：除非未来有明确研究目标，不开发 CPU 指令仿真器或 Hypervisor。
5. **接口稳定、实现可换**：VM API、设备 ABI、镜像模型和 Guest Profile 不绑定单一 Backend。
6. **失败可解释**：权限缺失、后端不可用、镜像不兼容、资源不足和 Guest 启动失败必须区分。
7. **安全默认开启**：AVB、SELinux、隔离和密钥边界不得为方便启动而默认关闭。
8. **资源自适应**：CPU、RAM、GPU、存储、并发数均依据设备实际状态和策略计算，不设跨设备固定档位。
9. **阶段性验证**：解析成功、Kernel 启动、init 启动、Framework 启动、桌面交互和真机稳定性是不同完成等级。
10. **可追溯**：第三方源码、二进制、镜像、补丁、构建产物和兼容性结论都要记录来源与版本。

## 3. 总体分层

```text
Android UI / Compose
  └─ Presentation State + Navigation
      └─ Application Use Cases
          ├─ VM Catalog / Image Import / Settings
          ├─ Foreground Runtime Service
          └─ Diagnostics / Permission Coordinator
              └─ ZeroVM Kotlin API
                  └─ JNI Boundary
                      └─ Native Runtime Facade
                          ├─ VM Core / Lifecycle State Machine
                          ├─ Guest Profile & Boot Orchestrator
                          ├─ Image & Partition Engine
                          ├─ Virtual Hardware Manager
                          ├─ Device Bridge Manager
                          ├─ Resource & Security Manager
                          └─ Backend Interface
                              ├─ AVF Backend
                              ├─ Native crosvm Backend (conditional)
                              └─ Test/Fake Backend
                                  └─ Guest Platform
                                      ├─ Kernel / DTB / Ramdisk
                                      ├─ Vendor / HAL / VINTF
                                      └─ Android Guest
```

## 4. 技术栈选型矩阵

| 层 | 首选候选 | 选择条件 | 不应承担的职责 |
|---|---|---|---|
| Android UI | Kotlin + Jetpack Compose + Material 3 | 新 UI、复杂状态、窗口自适应 | 直接调用 VMM 或操作设备节点 |
| Android 应用架构 | 单向数据流、ViewModel、Repository、UseCase | UI 状态与运行任务需分离 | 持有 Native 资源所有权 |
| 持久化 | Room/SQLite + DataStore | VM 元数据、设置、索引；小型偏好配置 | 把实时 VM 状态当作数据库真相 |
| 后台运行 | Foreground Service + 生命周期协调 | 用户明确启动且需要持续运行的 VM | 绕过 Android 后台执行限制 |
| Kotlin/Native 通信 | JNI + 明确 C ABI | 需要调用 C/C++ Runtime | 暴露 STL、C++ 异常或裸指针给 Kotlin |
| Native 核心 | C++20 | VM 编排、镜像解析、设备模型、状态机 | 自行实现 Hypervisor |
| 安全敏感/解析模块 | Rust（可选、按收益引入） | 需要强内存安全边界的复杂解析器/协议 | 无必要地跨语言拆分所有模块 |
| VMM | AVF 提供的受支持路径；条件允许时 crosvm/KVM | 设备实际支持且权限、接口、部署方式可用 | 假设普通 APK 一定可访问 |
| Guest Kernel | Android Common Kernel/GKI/Cuttlefish 对应版本 | Guest Android 版本与虚拟设备 ABI 已确定 | 直接复用任意手机 vendor kernel |
| 虚拟设备 | VirtIO 及后端明确支持的设备模型 | Guest Kernel 有对应驱动且 VMM 提供设备 | 仅在 UI 里模拟设备外观 |
| Guest 基础平台 | Cuttlefish/AOSP 可验证产物 | 作为参考 Guest Platform 起点 | 将 Cuttlefish Vendor 视为所有 GSI 通用 |
| 构建 | Gradle + Android NDK + CMake；Cargo 按需 | Android 与 Native 混合工程 | 用单一构建系统强行管理所有上游源码 |
| 测试 | Kotlin/JUnit、Native 单测、集成测试、设备测试 | 按模块和风险分层 | 只用一次启动截图作为验收 |

具体版本必须在仓库版本目录中锁定，并按 Android Gradle Plugin、Gradle、Kotlin、NDK、CMake、Rust 工具链兼容矩阵验证。

## 5. 何时选择何种 Backend

### AVF Backend
适用：目标 Android 设备提供 AVF/VirtualizationService，应用身份和权限满足系统要求，目标 VM 类型与镜像格式受支持。

必须先验证：服务存在、API 版本、授权、VM 配置约束、镜像输入格式、设备支持矩阵、VM 生命周期能力、显示/控制通道。

不适用：普通应用没有所需系统权限，设备未提供服务，或目标 Guest 配置超出后端支持范围。

### Native crosvm/KVM Backend
适用：平台允许合法部署并运行 VMM，存在可用 KVM 接口/权限，所需设备模型和安全配置可实现，且具备与 Host Android 版本相匹配的 Native 依赖。

必须先验证：`/dev/kvm` 或平台提供的等价接口是否可用、SELinux/权限、VMM 可执行环境、链接依赖、内核 KVM 能力、设备模型和启动协议。

限制：Linux 工作站能运行 crosvm 不代表 Android 手机上的普通 APK 能运行同样的 crosvm。

### Fake/Test Backend
适用：开发 UI、状态机、持久化和错误处理，不依赖真实 VM 的阶段。

必须明确标注模拟结果，不得将其报告为 Guest Kernel 或 Android 启动成功。

## 6. 模块依赖规则

- Presentation 只能依赖 Application API 和状态模型。
- Application 层依赖 Runtime 抽象，不依赖具体 AVF/crosvm 类。
- VM Core 依赖 Backend 接口，不依赖 Activity、Fragment 或 Compose。
- Image Engine 负责解析与校验，不直接启动 VM。
- Boot Orchestrator 组合 Kernel、DTB、Ramdisk、分区和设备清单。
- Backend 负责创建/运行 VMM，不负责解释 Android GSI 的语义。
- Guest Platform 通过版本化 Profile 描述，不硬编码到 VM Core。
- Device Bridge 的权限由 Host Coordinator 管理，Guest 不能越权访问 Host 设备。

## 7. 技术选型记录 ADR

每项重大选型必须建立 ADR，包含：状态（proposed/accepted/deprecated）、背景、候选方案、决策、理由、前置条件、风险、性能/安全影响、迁移路径、验证证据和复审触发条件。

建议首先建立 ADR：
- ADR-001 Backend Strategy
- ADR-002 Guest Architecture and ABI
- ADR-003 Native Language Boundary
- ADR-004 Image Storage and Copy-on-Write
- ADR-005 Guest Display Transport
- ADR-006 Host/Guest Device Bridge Security
- ADR-007 Guest Profile Versioning

## 8. 工程完成标准

每个模块必须具备接口定义、实现、单元测试、集成测试、失败路径、日志/诊断、资源释放、文档和兼容性声明。未具备真实后端验证的功能标记为 planned 或 experimental。
