# 25. 构建、CI、测试、诊断与可观测性框架

## 1. 构建目标

构建系统必须支持 Android Kotlin、JNI/C++、按需 Rust、Guest Kernel/平台产物和测试工具，同时保证每个产物可追溯。不同上游项目可以保留其原生构建系统，通过受控脚本和产物接口集成。

## 2. 构建分层

- Android：Gradle Wrapper、AGP、Kotlin、Android SDK。
- Native：NDK、Clang、CMake、Ninja。
- Rust（按需）：固定 toolchain、Cargo.lock、Android target。
- AOSP/Kernel/crosvm：遵循各自 Soong/Bazel/Make/Ninja/Cargo 构建要求。
- Packaging：统一产物目录、manifest、哈希和许可证清单。

CI 必须固定工具链版本和依赖来源，禁止依赖开发机未声明的环境变量或全局工具。

## 3. 代码质量与安全扫描

- Kotlin lint/静态分析；
- C/C++ clang-tidy、编译告警和 sanitizer（可运行环境）；
- Rust clippy/audit（采用 Rust 时）；
- dependency/license inventory；
- secret scanning；
- SBOM；
- Native ABI/API compatibility；
- 第三方 patch 与上游版本检查。

## 4. 测试金字塔

1. 纯函数/模型单测；
2. Kotlin UseCase/Repository/StateMachine 测试；
3. Native parser/resource/state 单测；
4. Fuzz 与属性测试；
5. JNI/Native 集成测试；
6. Fake Backend Contract Test；
7. Linux/KVM/Cuttlefish 系统集成测试；
8. Android 真机 Backend/权限/Kernel 启动测试；
9. 完整 Android Guest E2E；
10. 长稳、温度、内存压力、多 VM 和恢复测试。

Fake Backend 只能验证控制面逻辑，不能替代真实虚拟化测试。

## 5. 启动阶段自动化验证

为每次启动记录阶段时间和结果：backend create、kernel entry、kernel init、first-stage init、partition mount、second-stage init、VINTF/HAL、Zygote、System Server、SurfaceFlinger、Launcher、输入/网络验证。

每阶段有日志采集、错误码、超时、重试策略和诊断附件。超时阈值按平台/Profile 校准，不使用统一固定值作为所有设备的性能标准。

## 6. 日志与指标

日志按 Host App、Runtime、Backend、Kernel Console、Guest init、HAL、Framework、Bridge 分域。每条日志带 VM ID、operation ID、sequence、时间戳、阶段和 severity。

指标包括资源预算/实际使用、启动耗时、崩溃原因、I/O 错误、Bridge 延迟/断连、温度状态、内存压力和快照操作结果。敏感内容脱敏。

## 7. 崩溃与诊断包

诊断包可包括：
- Host/Backend 能力快照；
- Guest Profile 和镜像元数据（不包含镜像内容）；
- 启动配置脱敏副本；
- VM 状态事件；
- Kernel/Guest 日志；
- Backend 错误和资源统计；
- 版本、构建 commit、依赖清单。

导出前提供隐私说明和用户确认。不得自动上传用户镜像、userdata、密钥或完整内存 dump。

## 8. CI 分层

- PR：格式、静态检查、Kotlin/Native 单测、解析器 fuzz smoke、构建。
- Main：完整单测、ABI 检查、依赖/许可证检查、产物归档。
- Nightly：长时间 fuzz、性能回归、Cuttlefish/KVM 集成。
- Device Lab：按设备型号、Android 版本、后端能力和权限配置执行真机测试。

设备矩阵要记录具体硬件和系统版本；不能将一台设备通过视为所有 Android 手机兼容。

## 9. 发布门禁

发布前确认：构建可复现、关键测试通过、无未处理高危安全问题、镜像解析边界测试通过、数据迁移验证、权限说明准确、后端支持范围真实、已知限制公开、崩溃恢复与数据保护测试通过。

## 10. 完成状态定义

- DOCUMENTED：设计文档完成；
- IMPLEMENTED：代码已实现；
- BUILT：目标构建通过；
- UNIT_TESTED：单测通过；
- INTEGRATION_TESTED：集成测试通过；
- EMULATOR_VALIDATED：模拟环境验证；
- DEVICE_VALIDATED：指定真机验证；
- STABLE：持续稳定性门禁通过。

状态必须按功能和设备记录，不能以项目整体单一状态掩盖缺口。
