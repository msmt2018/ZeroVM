# 31. 跨层 API、JNI、C ABI 与 Backend 接口契约

## 1. 目的与规范级别

本文把 Kotlin 应用层、Service、JNI、C ABI、C++ Runtime、Rust 辅助模块、虚拟化 Backend 和设备后端之间的调用边界固定下来。本文是后续源码实现的接口规范，不代表这些接口已经落地。MUST 表示必须遵守，SHOULD 表示强烈建议。

## 2. 分层依赖方向

唯一允许的主调用方向：

UI/Compose → Kotlin UseCase/Repository → VM Controller/Foreground Service → JNI Facade → ZeroDroid C ABI → C++ Runtime → Backend Adapter → AVF 或获得系统授权的 ARM64 KVM/VMM/厂商 Hypervisor 后端。禁止接入 QEMU TCG 等软件 CPU 模拟路径。

镜像解析、Guest Profile、资源规划和设备清单属于领域服务。Backend 不得依赖 Activity、Fragment、Compose、ViewModel 或 UI 类型。Guest 驱动与 VMM 设备模型之间使用独立设备协议，不能把 JNI API 当作 Guest VirtIO 协议。

## 3. Kotlin 公共 API 契约

建议公共 API 提供 initialize、createVm、getVm、startVm、requestShutdown、stopVm、destroyVm、observeEvents 等异步操作。耗时操作使用 suspend/Flow 或明确的异步回调，不得阻塞 Android 主线程。

- Kotlin 层不得接触 C++ 对象、STL 类型或裸指针。
- VM Handle 是不透明 token，不是 Native 地址。
- UI 生命周期只控制事件订阅，不决定 VM 生命周期；VM 由 Runtime/Service 的明确所有者持有。
- create、start、shutdown、stop、destroy 必须经过统一状态机和操作序列化。
- destroy 必须幂等；重复释放返回确定错误，不得误释放其他 VM。
- handle 必须带 generation 或等价防重用机制，避免旧 handle 指向新对象。
- 所有 API 明确线程安全、取消语义、超时和错误类型。

## 4. JNI 与 C ABI

JNI 仅负责类型转换、线程衔接和错误映射；业务状态机不得放在 JNI glue 层。

C ABI 是跨语言稳定边界：
- 导出符号统一使用 zdroid_ 前缀；
- 公共结构体包含 abi_version 和 struct_size；
- 不暴露 C++ class、STL、异常、Rust trait 或编译器特定布局；
- 字符串明确 UTF-8、长度和 NUL 规则；
- 数组显式传元素数量；
- 所有输入长度、嵌套深度和分配规模有上限；
- 所有函数返回稳定 status code，详细诊断通过 error_info 获取；
- 每个输出对象明确由谁释放，禁止隐式混合所有权。

建议 ABI 轮廓：

    typedef uint64_t zdroid_handle;
    typedef struct {
        uint32_t abi_version;
        uint32_t struct_size;
        uint64_t flags;
    } zdroid_runtime_options;

    typedef struct {
        uint32_t code;
        uint32_t domain;
        uint64_t operation_id;
        char message[256];
    } zdroid_error_info;

    zdroid_status zdroid_runtime_create(
        const zdroid_runtime_options* options,
        zdroid_handle* out_runtime,
        zdroid_error_info* out_error);

    zdroid_status zdroid_vm_create(
        zdroid_handle runtime,
        const uint8_t* spec,
        size_t spec_size,
        zdroid_handle* out_vm,
        zdroid_error_info* out_error);

    zdroid_status zdroid_handle_release(
        zdroid_handle runtime,
        zdroid_handle object,
        zdroid_error_info* out_error);

该示例规定接口形态，不替代正式头文件。正式 ABI 必须定义字段布局、对齐、最大长度、所有权、线程安全和版本兼容规则。

### 4.1 Handle 规则

- Handle 是 token 或 slot+generation 编码，不是裸地址。
- Runtime、VM、Operation、Subscription 使用不同 handle 类型域。
- 每个 handle 绑定创建它的 Runtime；跨 Runtime 使用必须拒绝。
- 释放后立即失效；generation 不匹配返回 STALE_HANDLE。
- handle 不可持久化，也不可作为 VM 身份 ID。
- Native 对象由注册表管理，并通过引用计数、锁或消息队列保护并发访问。

### 4.2 JNI 规则

- JNI_OnLoad 只做轻量注册，不启动 VM 或扫描大镜像。
- 每个入口检查 null、数组长度、字符串编码和 handle。
- C++ 异常必须在 JNI 边界捕获并转换，不得跨 JNI frame 传播。
- Native callback 不得长期持有 Activity/Fragment GlobalRef。
- callback 注册返回 subscription handle；取消订阅后不得再发起新回调，并定义已排队事件的处理语义。
- Native 线程按需 AttachCurrentThread，退出前 DetachCurrentThread。
- DirectByteBuffer 必须明确共享/拷贝语义和 buffer 生命周期。

## 5. Runtime 与 Backend 契约

Backend 抽象至少覆盖 queryCapabilities、validateConfiguration、create、start、requestShutdown、forceStop、queryState、subscribeEvents、destroy。每个操作必须定义前置状态、成功后状态、失败后的状态、副作用、超时和取消能力。

Backend 不支持的功能返回 UNSUPPORTED_CAPABILITY，不得静默 no-op。创建结果必须返回实际协商配置，包括实际 vCPU、Guest RAM、设备集合和 console/vsock/display 能力，不得只回显请求值。

能力分层为 advertised、probed、authorized、negotiated、validated。上层只能根据 negotiated/validated 状态开放功能；advertised 不等于可用。

### 5.1 Operation 模型

每个耗时操作具有唯一 operation_id，状态为 ACCEPTED、QUEUED、RUNNING、CANCELLING、SUCCEEDED、FAILED、CANCELLED 或 TIMED_OUT。终态不可逆。取消是请求，不保证底层操作可中断；必须报告实际取消结果。重复提交同一幂等键不得创建重复 VM 或重复写入；非幂等操作不得自动重试。

## 6. Rust/C++ 互操作

Rust 仅通过 C ABI 或明确限定的静态库边界与 C++ 通信：
- repr(C) 结构必须固定布局；
- Rust panic 不得越过 FFI，使用 catch_unwind 并映射错误；
- C++ 异常不得进入 Rust；
- 跨边界资源由同一侧释放，或使用显式配对 alloc/free；
- 不跨 FFI 传递 Rust String、Vec、Box、trait object 或 C++ STL；
- 回调定义上下文指针生命周期和并发保证；
- 不可信输入解析模块优先采用纯数据输入/输出。

## 7. 线程、锁与销毁顺序

- 单 VM 控制操作串行化；不同 VM 可并行但受全局资源预算约束。
- 不持有 Runtime 全局锁调用 Backend 或等待 I/O。
- 事件回调不得在 Backend 内部锁持有期间调用用户代码。
- Runtime shutdown 先停止接收新操作，再取消/等待可取消操作，再停止 VM，再关闭设备通道、订阅和线程，最后释放 Backend/Runtime。
- 对不能安全中断的后端调用定义等待策略和诊断结果。
- 固定锁顺序，防止回调重入和锁反转。

## 8. 版本兼容策略

分别版本化 Kotlin API、C ABI、DTO schema、Backend capability schema、VirtIO device ABI 和 bridge protocol，不使用一个总版本号代替所有边界。ABI major 变化表示不兼容；minor 只允许兼容扩展。新增字段必须有缺省语义。C ABI 使用 struct_size 支持尾部扩展。每个构建产物记录 ABI version、Git commit、toolchain 和依赖版本。

## 9. 必须覆盖的契约测试

Handle 重复释放、跨 Runtime、过期 generation、伪造 token；JNI null/超长字符串/非法编码/超大数组；C++ 异常、Rust panic、线程退出和 GC；并发 start/stop/destroy；Backend 缺失权限；不支持 pause/snapshot 时返回错误；callback 取消和 Runtime 关闭竞态；新旧 struct_size 互操作；Fake Backend 与真实 Backend 共用状态/错误契约测试。

## 10. 完成门禁

进入 IMPLEMENTED 前必须提交头文件、Kotlin API、绑定代码、错误码定义、状态机测试和 ABI 检查。进入 DEVICE_VALIDATED 前必须记录具体 Android 设备、系统版本、权限条件和 Backend。本文细化 docs/20 的调用边界，不代表源码已经实现。