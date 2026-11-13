# 22. 虚拟硬件与设备模型框架设计

## 1. 设计原则

虚拟硬件不是 UI 中的一组配置项，而是 Guest Kernel、VMM、DTB、HAL 和 Host Bridge 共同遵守的设备 ABI。每个设备必须有：设备模型、传输协议、地址/中断、feature negotiation、Guest 驱动、错误行为、生命周期和测试。

## 2. 设备模型层次

1. Device Capability Discovery：探测后端实际支持的设备类型和 feature。
2. Virtual Hardware Manifest：描述本 VM 设备拓扑和 ABI 版本。
3. Backend Device Adapter：把 Manifest 转换为 AVF/crosvm 的设备配置。
4. Guest Device Description：生成/验证 DTB、ACPI（若架构需要）或后端设备描述。
5. Guest Driver Contract：匹配 Kernel 驱动和模块。
6. Guest HAL/Framework：将设备能力暴露给 Android。

同一份 Manifest 是各层配置的单一事实来源，但必须记录后端协商后的实际设备集合。

## 3. 设备类别与技术选型

| 设备 | 候选机制 | 前置条件 |
|---|---|---|
| CPU/中断/Timer | Backend vCPU、GIC、虚拟 Timer | ARM64 虚拟化后端支持 |
| RAM | Guest memory mapping、共享内存、可选 balloon | 后端内存模型和回收语义 |
| Block | VirtIO Block 或后端原生虚拟磁盘 | Guest early driver、镜像映射 |
| Console | VirtIO Console/串口 | VMM 日志/控制通道 |
| Network | VirtIO Net + NAT/路由/DNS | Guest net driver、Host 网络权限 |
| Vsock | VirtIO Vsock | 后端和 Guest 两端支持 |
| Input | VirtIO Input 或专用输入通道 | Guest input driver、显示映射 |
| GPU | VirtIO GPU/gfxstream/软件渲染 | Guest DRM/HAL 与 Host GPU backend |
| Audio | 虚拟音频设备 + Host Audio Bridge | Guest Audio HAL、Host 音频权限 |
| Camera | Camera HAL + Host Camera Bridge | Camera2/Provider、权限与生命周期 |
| Sensors | 虚拟 Sensor HAL + 事件桥 | Guest Sensor HAL、Host Sensor 权限 |
| Wi-Fi/BT/GNSS | Host Bridge 或独立虚拟设备 | 明确协议、权限和功能降级 |
| USB | 虚拟 USB controller / 受控 passthrough | 后端控制器模型、授权和热插拔 |
| Battery/Power/Thermal | Guest HAL + 受限状态桥接 | Host 数据可用性和隐私策略 |
| KeyMint/Gatekeeper | Guest 独立实现或受限安全服务 | 明确安全等级，不冒充 Host TEE |

## 4. 资源自适应

CPU vCPU 数量、Guest RAM、GPU buffer、磁盘 I/O 和桥接带宽均由 Host Capability Snapshot、当前负载、温度、电量、并发 VM、后端限制和用户策略共同决定。资源调节必须有上下限、硬上限、滞回和压力降级，避免频繁抖动。

vCPU 数量不是简单复制 Host 核数；Guest RAM 不是按物理内存固定比例切分。设备存在不代表后端允许透传，驱动存在不代表 Guest ABI 已匹配。

## 5. GPU 与显示路径

候选路径：
- Backend 提供的虚拟 GPU + Guest DRM/Gralloc/Composer；
- gfxstream 等完整图形虚拟化路径；
- 软件渲染作为性能较低的兼容/调试模式。

选择前验证 Guest Kernel DRM、DMA-BUF、sync fence、Gralloc、Composer HAL、EGL/GLES/Vulkan 和 Host backend 的版本与能力。只实现 framebuffer 传输时应称为显示输出通道，不宣称完整 GPU 加速。

显示链路要定义 buffer 所有权、格式、stride、modifier、fence、同步、旋转/分辨率、resize、Surface 销毁重建和断连恢复。

## 6. 输入映射

定义坐标空间转换：Host View → Display viewport → Guest display pixels → Guest input device coordinates。处理旋转、缩放、Insets、多指触控、按键、鼠标、悬浮指针和输入焦点。输入事件应有时间戳、设备 ID 和取消语义。

## 7. 网络与设备桥接

设备桥接需定义协议版本、消息 framing、认证、授权、超时、限流、断线重连、背压和审计。网络采用明确的 NAT/route/DNS 方案；Guest 不直接获得 Host 网络管理权限。

## 8. ABI 版本策略

设备类型、feature bit、配置字段和消息协议均版本化。新增 feature 必须允许旧 Guest 忽略或协商失败；删除/变更语义必须提升 ABI major 或提供兼容层。

## 9. 测试

每种设备分别测试：枚举、feature negotiation、读写、异常输入、热插拔、超时、断连、Host 进程死亡、Guest 重启、资源压力和后端差异。设备“可枚举”不等于功能可用。
