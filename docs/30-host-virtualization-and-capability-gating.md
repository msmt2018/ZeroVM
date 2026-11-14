# 30. Host 虚拟化能力门控、权限与后端选择

## 1. 目标

ZeroDroid 必须依据真实 Host 设备和系统环境选择后端。SoC 理论上支持 ARM 虚拟化，不代表 Android 内核、EL2 所有权、设备节点、系统服务和应用权限允许第三方 App 创建 VM。

## 2. 分层能力探测

每项能力分别记录 silicon、firmware、kernel、device node、service、app permission、backend API 和 end-to-end 状态。CPU feature 检测结果不能直接当作 KVM/AVF 可用结论。

## 3. Backend 选择

AVF：检测设备实际提供的 API、VirtualizationService、权限和支持的 VM 类型。AVF API/权限受 Android 版本、系统镜像、设备实现、应用预装/特权授权和具体 API 约束，不能简单规定 Android 13+ 所有手机可用。

直接 KVM/VMM：需要 Host Kernel 暴露 KVM 接口、权限、EL2/虚拟化控制权和兼容的 VMM/设备后端。/dev/kvm 是否存在只是检查项，不代表 App 可访问或创建所需 VM。crosvm 仅是 VMM 候选，不是 Hypervisor；可在 Android ARM64 KVM 上运行也必须通过系统权限和设备闭环验证。

厂商 Hypervisor：Gunyah、GenieZone 等接口受 SoC、厂商、内核和系统集成约束，不能当作 Android 通用标准，需独立适配、授权和真机验证。

软件 CPU 仿真（包括 QEMU TCG）不属于 ZeroDroid 运行后端，也不得作为完整 Guest 的自动 fallback。没有可用硬件虚拟化时，应明确报告设备不支持完整 VM。

## 4. Root、Shizuku 与系统权限

Shizuku、su、KernelSU、Magisk 不会自动提供 Hypervisor、EL2 或 AVF 管理权限。Root 不能凭空启用缺失硬件能力、内核配置或安全固件接口。刷入修改内核是独立实验路径，带来启动、安全和 OTA 风险，不能作为普通用户前提。

## 5. 资源预算

CPU、RAM、GPU、存储和并发 VM 数量均按 Host 实际探测与实时压力计算。考虑 Android 前后台限制、内存压力、温度、电池、存储余量、GPU backend 和用户负载。不能规定所有设备必须 8GB RAM，也不能固定 Guest 分配 4–6GB。

## 6. GPU 能力门控

Vulkan 版本/扩展存在并不足以证明 gfxstream/Venus、AHardwareBuffer、DMA-BUF、同步 Fence 和零拷贝路径可用。必须逐项探测并执行 buffer import/export、render、present 和同步测试。不可用时选择可验证的软件渲染或禁用对应能力，并明确性能等级。

## 7. Host Capability Manifest

记录 host_fingerprint、android_api、kernel_release、abi、cpu_features、virtualization_extensions、avf_api、service_status、device_nodes、permission_status、backend_candidates、gpu_extensions、memory_budget、storage_budget、thermal_state、verified_features 和 probe_timestamp。敏感标识仅用于本地诊断，导出前脱敏。

## 8. Android ARM64 Host 移植门控

SoC 是 ARM64 并不代表上游 VMM 可直接在 Android 上运行。必须检查 Bionic/NDK、Linux 用户态服务依赖、SELinux、linker namespace、cgroup/seccomp、设备节点和厂商 Hypervisor ABI；PC x86/PCI/ACPI/APIC/BIOS 路径必须移除或隔离。VMM 需分为 Portable Core、Android Host Adapter 和 Hypervisor Adapter。具体审计和测试标准见 [Android ARM64 宿主移植规范](37-android-arm64-host-porting-and-adaptation.md)。

## 9. 验收

探测结果区分理论能力与端到端验证；每个 Backend 有 capability contract 和明确错误码；无权限或后端不可用时不崩溃、不假报成功；资源配置动态且有硬上限；每种后端记录具体真机型号、系统版本和构建指纹测试结果。
