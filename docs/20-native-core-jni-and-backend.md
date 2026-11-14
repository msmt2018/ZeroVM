# 20. Native Core、JNI 与虚拟化 Backend 架构

## 1. 设计目标

Native Core 提供稳定的 VM 管理、镜像处理、启动编排和设备抽象。ARM64 Guest 指令必须由硬件虚拟化扩展执行；Native Core/VMM 负责 KVM/AVF 接入、vCPU/内存/中断/设备运行管理，不得以 QEMU TCG 等软件 CPU 模拟替代。Hypervisor 能力来自 Host Kernel/固件/AVF，不是普通 App 可自行创建的功能。

## 2. Native 模块边界

建议按职责组织：
- `runtime/`：Runtime 初始化、全局依赖和关闭。
- `vm/`：VM 对象、状态机、操作协调和句柄。
- `backend/api/`：抽象接口和能力模型。
- `backend/avf/`：AVF 适配。
- `backend/kvm/`：经系统授权的 ARM64 KVM Backend 与 ZeroDroid VMM 设备运行时。\n- `backend/crosvm/`：可选的 VMM 实现适配/代码复用层，只有通过 Android ARM64 KVM、权限、设备模型和启动闭环验证后才启用；它不是 Hypervisor。
- `backend/fake/`：测试后端。
- `boot/`：Guest Boot Profile、启动参数和 Boot Orchestrator。
- `image/`：镜像探测、解析、验证和分区。
- `hardware/`：虚拟设备 manifest、拓扑和 ABI。
- `bridge/`：Host/Guest 数据通道。
- `resource/`：预算、监控和压力策略。
- `security/`：策略、密钥引用和安全审计。
- `diagnostics/`：事件、日志、dump 和错误分类。

## 3. C++ 与 Rust 选型

C++20 适合与 Android NDK、JNI、AOSP Native 库和现有 C/C++ VMM 接口集成。Rust 适合复杂、不可信输入解析和内存安全要求高的独立模块，但引入前必须评估 Android target、Cargo 依赖、JNI/FFI、构建缓存、调试和 ABI 维护成本。

不要为了“安全”把所有代码拆成大量 C++/Rust 小库，导致跨语言所有权和错误处理复杂化。以明确安全边界为拆分依据。

## 4. Backend 接口

Backend 接口应表达能力，而不是假定每种后端都支持所有功能。概念接口：

```cpp
struct BackendCapabilities {
    bool supportsProtectedVm;
    bool supportsConsole;
    bool supportsVsock;
    bool supportsDisplay;
    bool supportsPause;
    bool supportsSnapshot;
    // plus versioned device/boot constraints
};

class IVirtualMachineBackend {
public:
    virtual ~IVirtualMachineBackend() = default;
    virtual BackendCapabilities queryCapabilities() = 0;
    virtual Result create(const BackendVmConfig&) = 0;
    virtual Result start() = 0;
    virtual Result requestShutdown() = 0;
    virtual Result stop() = 0;
    virtual Result destroy() = 0;
    virtual Result queryState(BackendState&) = 0;
};
```

这只是概念契约；实际接口需要异步操作、取消令牌、事件订阅、稳定 VM handle、错误域和版本协商。不要把不支持的 pause/resume/snapshot 伪装成成功。

## 5. JNI ABI 规范

Kotlin 不直接接触 C++ 对象、STL 类型或裸指针。JNI 边界采用：
- 稳定整数/不透明 Handle；
- UTF-8 字符串和明确编码规则；
- 有上限的 ByteArray/DirectByteBuffer；
- 结构化 DTO 或 JSON/Protobuf（按性能与演进需求选择）；
- 统一错误码和详细错误对象；
- callback/event listener 生命周期；
- 显式 close/release；
- 所有 JNI 入口捕获 C++ 异常，不让异常越过 JNI 边界。

每个 Handle 都要定义线程安全、并发调用、重复释放、进程重启和 stale handle 行为。

## 6. 线程和异步

- 不在 Android 主线程执行镜像解析、磁盘 I/O、启动等待和日志读取。
- VM 每实例使用串行操作队列或状态机操作锁，避免 start/stop/reboot 竞态。
- 后端回调不能直接持有 Activity/Compose 对象。
- Native 线程退出前停止回调、释放 GlobalRef、Join 线程。
- JNI attach/detach 规则必须明确。
- 对阻塞后端调用设置超时和取消语义；若底层 API 不支持取消，必须报告实际行为。

## 7. 资源所有权

为 fd、mmap、共享内存、虚拟磁盘、锁文件、socket、线程、JNI reference 和 backend handle 定义唯一所有者。采用 RAII 管理 Native 资源；跨层传递使用 move/共享引用等明确规则。销毁顺序必须先停止 Guest，再关闭设备通道，再释放后端对象和磁盘锁。

## 8. Backend 适配策略

AVF 与直接 KVM/VMM 的配置模型不同。上层使用统一 Guest Profile 和 Virtual Hardware Manifest；Backend Adapter 将其转换为 AVF 或 KVM/VMM 配置，并返回实际协商后的能力与资源。QEMU/TCG、Android Emulator、Goldfish、AEMU 和 Cuttlefish 镜像启动明确不在候选后端中。

启动前进行两次检查：
- Static preflight：镜像、Profile、接口和权限。
- Runtime preflight：当前资源、服务状态、设备能力和后端限制。

## 9. Native 构建与依赖

- Android App 使用 Gradle/AGP/Kotlin。
- JNI/C++ 使用 Android NDK + CMake。
- Rust 仅在选定模块需要时通过 Cargo/NDK target 集成。
- 上游 crosvm/AOSP/Kernel 的构建体系可能需要 Soong、Bazel、Ninja 等，不能假设可直接作为普通 CMake 子目录。
- 记录 NDK、Clang、CMake、Rust、ABI、API level、依赖 commit 和 patch。
- 依赖升级必须执行 Native 单测、ABI 检查和目标设备验证。

## 10. 测试

- Native 单元测试：状态机、错误映射、资源预算、解析器。
- Fuzz：Boot Image、Sparse、LP metadata、GPT、VBMeta 等不可信输入。
- JNI 测试：异常、重复释放、线程退出、GC、Activity 重建。
- Backend Contract Test：所有后端共享一组契约测试。
- 真机测试：真实权限、服务、启动和停止，不以 Fake Backend 结果代替。
