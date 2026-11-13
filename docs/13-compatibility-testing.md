# 13. GSI 兼容性矩阵与测试策略

## 兼容性不是单一布尔值

兼容性由 Host 设备和虚拟化后端、Guest CPU 架构、Android release/API、Guest Kernel/驱动、Boot Image 格式、Vendor HAL/VINTF、分区和文件系统、AVB/SELinux、图形栈和可选 Bridge 共同决定。

结果分级：不支持、未检测、仅镜像可解析、Kernel 可启动、Android init 可启动、Framework 可启动、桌面可交互、完整功能通过。

## Guest Profile 矩阵

每个 Profile 记录 Android release/API、ABI/架构、Kernel branch/version/commit、Boot header version、Vendor/HAL 接口版本、VINTF matrix、VirtIO ABI、分区布局、AVB 策略、SELinux policy、已验证 Host/Backend 范围、已知限制和测试日期。

不应仅根据 Android 版本号推断兼容。

## 测试层级

### 单元测试
镜像头解析、Sparse 解码、GPT/LP metadata、Boot Header、AVB descriptor、配置 schema、状态机、预算算法和协议编解码。

### Native 集成测试
JNI Handle 生命周期、Backend Adapter、VirtIO 设备创建、磁盘读写、Vsock、Bridge 超时和资源释放。

### Guest 启动测试
Kernel、first-stage init、分区挂载、SELinux、VINTF、HAL、Zygote、System Server、SurfaceFlinger 和 Launcher。

### 真机测试
不同 SoC、内存容量、Android 版本、热状态、屏幕尺寸、GPU 驱动、权限配置和 AVF 支持情况。

### 稳定性测试
反复启动/关机、长时间运行、低内存、磁盘满、Host 进程重启、VM 崩溃、Bridge 断连和设备温度升高。

## 启动验收等级

- L0：镜像识别和静态校验。
- L1：VMM 创建成功。
- L2：Guest Kernel 启动并有 Console。
- L3：Android init 和必要分区挂载。
- L4：Zygote/System Server 正常。
- L5：Launcher 和交互显示正常。
- L6：核心设备能力（网络、输入、图形）通过。
- L7：声明范围内的设备桥接和稳定性通过。

公开兼容声明说明通过等级、Host 型号/系统、Backend 版本、Guest Profile 和已知限制。

## 自动化与回归

每次测试保存去敏 Host capability snapshot、VM 配置/Profile 版本、镜像哈希、Backend 版本、Kernel/Boot 日志、Guest logcat、VINTF 结果、SELinux denial 摘要、资源/温度事件和测试结果。

更新 Kernel、VMM、HAL、VirtIO ABI、镜像解析器或 Boot Profile 时运行对应回归集合。