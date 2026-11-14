# 28. 厂商扩展、专有 HAL 与系统服务适配框架

## 1. 目标

为 HyperOS/MIUI、One UI、ColorOS、OriginOS 等 ROM 建立可版本化、可审计的厂商兼容层。该层处理私有 AIDL/HIDL 服务、设备节点、属性、权限和系统组件依赖，但不得把伪造服务当成真实硬件能力。

## 2. 依赖发现

ROM 分析收集 VINTF manifest、framework compatibility matrix、init rc 服务定义、AIDL/HIDL interface/instance/version、service_contexts、hwservice_contexts、SELinux policy、ueventd 规则、fstab、模块列表、property files/property contexts、sys/proc/dev 节点访问线索和系统进程依赖。

静态扫描只产生候选依赖，必须结合动态日志和实际调用验证，不能据此自动生成任意空服务。

## 3. Adapter 类型

- Native implementation：关键功能且可实现时，提供符合厂商 ABI、生命周期和错误语义的真实服务。
- Compatibility proxy：将 ROM 私有接口映射到 ZeroDroid 标准服务，明确字段、单位、状态机、超时和错误映射。
- Restricted stub：仅对非关键、可选或明确允许降级的功能使用；返回接口允许的 UNSUPPORTED/空能力状态，不能伪造认证或成功执行结果。
- Device node adapter：通过受控 Guest Kernel driver 或专用服务提供虚拟节点；不能假定普通 App 可随意挂载或修改宿主 sysfs。
- Property profile：按 ROM Profile 提供验证过的属性集合，满足命名空间、类型、只读时机和 property context 一致性；不能靠篡改 ro.* 冒充真实设备。

## 4. 不可盲目 Stub 的服务

KeyMint/Keymaster、Gatekeeper、Weaver、StrongBox、OEM unlock、TEE、DRM/受保护媒体、Radio/基带/SIM/IMS、紧急呼叫、生物识别、安全支付、Camera、GPU、音频路由、设备证明和 Verified Boot 均不得用空实现冒充可用。

可提供受限模拟实现，但必须向 Guest 报告真实安全级别和能力，不得伪造硬件背书。

## 5. AIDL/HIDL 契约

每个服务声明 interface package、版本、instance、transport、稳定性、VINTF manifest、init 用户/组/能力、SELinux domain、service context、Binder 并发、重启策略、death recipient、超时、错误码、Android release 和 ROM build 范围。

ServiceManager 注册同名服务不代表接口兼容。事务码、parcelable 布局、语义和生命周期必须一致。

## 6. SELinux 与节点安全

不通过关闭 SELinux 或宽泛 allow 规则解决问题。每个 Adapter 使用独立 domain、最小权限和明确文件类型；虚拟节点建立 Guest 内部隔离。需要内核策略补丁时纳入 Guest Platform 版本和测试。

## 7. Adapter Manifest

机器可读 Manifest 至少包含 id、vendor、rom_family、android_api_range、build_fingerprint_constraints、interfaces、properties、device_nodes、dependencies、security_level、degradation、conflicts、source_revision、artifact_hashes 和 tests。

加载遵循 allowlist、签名/哈希验证、版本匹配、冲突检测和回滚。不能从 ROM 输入执行未经信任脚本或 native binary。

## 8. 运行时监测与验收

监测服务注册/死亡、Binder 调用失败、init restart、SELinux denial、属性读取异常、节点访问失败、System Server watchdog 和关键进程崩溃，并关联 Adapter ID 与 ROM fingerprint。

每个 Adapter 绑定确切 ROM build/API 范围；Stub 明确标记降级；安全敏感接口不伪造成功；AIDL/HIDL、VINTF、init、SELinux 和依赖服务通过验证。
