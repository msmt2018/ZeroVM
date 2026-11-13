# 26. ROM 家族兼容性与全品类 ROM 适配架构

## 1. 目标与边界

ZeroVM 的目标是尽可能扩大 Android ROM 的可运行范围，包括 AOSP GSI、AOSP 派生系统、Pixel 系统映像，以及 HyperOS/MIUI、One UI、ColorOS、OriginOS 等厂商定制系统。

“任意 ROM”不是单一通用 Vendor 或万能 Stub 可以保证的工程承诺。ROM 可能依赖特定 SoC、设备树、内核驱动、专有 HAL、TEE、基带、分区布局、AVB 信任链和厂商服务。缺少对应硬件或专有实现时，可能无法启动，或启动后关键功能不可用。

因此必须采用分级兼容性，而不是简单的支持/不支持。

## 2. 兼容性分级

| 级别 | 名称 | 验收含义 |
|---|---|---|
| R0 | 可识别 | 识别包格式、设备目标、Android 版本、分区和依赖 |
| R1 | 可解析 | 提取镜像、解析分区元数据、建立依赖图 |
| R2 | Kernel 启动 | 匹配的 Guest Kernel、固件和设备描述可启动 |
| R3 | Android 启动 | init、挂载、SELinux、servicemanager、Zygote 和 System Server 稳定 |
| R4 | 桌面可用 | Launcher、输入、显示、存储、网络和常用设置可用 |
| R5 | 厂商框架兼容 | 厂商私有服务、扩展 API、设置和专有组件通过验证 |
| R6 | 硬件功能兼容 | 对虚拟平台实际提供的摄像头、音频、生物识别、传感器等逐项验收 |

每份报告必须列出已验证、缺失、降级、崩溃风险和未测试功能。

## 3. ROM 兼容身份

兼容对象不能只有 ROM 名称。至少记录 ROM 家族/版本、Android release/API、CPU ABI、Kernel release/GKI/KMI、Boot Header、boot/vendor_boot/init_boot 组合、分区拓扑、动态分区组、VINTF、HAL 实例及版本、AVB 链、回滚索引、厂商扩展服务、属性命名空间、设备节点依赖和目标设备/SoC 信息。

同一厂商、同一营销 ROM 名称的不同设备版本也不能假设兼容。

## 4. Guest Platform Profile 匹配

每个 ROM 必须匹配明确的 Guest Platform Profile，而不是任意组合材料。Profile 绑定 Kernel、模块、DTB/DTBO、启动方式、分区布局、Vendor/ODM、VINTF、HAL、SELinux policy、init 配置、虚拟硬件 ABI、AVB 信任策略、设备桥接能力和测试结果。

匹配输出 exact、compatible-with-degradation、requires-adapter、incompatible 或 unknown，并提供证据与阻断原因。

## 5. 厂商 ROM 适配策略

优先保持 ROM 原有 System/Vendor/ODM 组合并验证其虚拟化适配性；若 System/Product 可与另一套 Vendor ABI 兼容，才使用经过验证的平台组合；对可隔离的扩展开发版本化 Adapter；对无法实现的物理硬件功能明确降级。

直接用 Cuttlefish Vendor 替换任意厂商 Vendor 不是通用方案。System 与 Vendor 的 VINTF、HAL、属性、SELinux、内核接口和私有服务必须匹配。

## 6. Adapter 插件模型

组件包括 RomDetector、PartitionGraphProvider、GuestProfileResolver、VendorExtensionAdapter、BootCompatibilityChecker、RuntimeCompatibilityMonitor 和 CompatibilityReporter。

每个 Adapter 声明适用 ROM、Android 版本、接口版本、依赖、权限、风险、测试设备和校验哈希。禁止宽泛匹配影响无关 ROM。

## 7. 失败分类

区分镜像格式错误、分区缺失、AVB 拒绝、Kernel panic、first-stage mount 失败、VINTF 不匹配、HAL 缺失、SELinux denial、属性不一致、System Server watchdog、图形初始化失败、TEE/KeyMint 不可用和应用 ABI 不兼容。

## 8. 完成标准

每个受支持 ROM 都有独立兼容档案；声明绑定镜像哈希与 Profile 版本；解析成功不等于能启动，能进桌面不等于全功能兼容；HyperOS、One UI、ColorOS 等必须逐设备/版本验证。未验证者标记实验性或未知。
