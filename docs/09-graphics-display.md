# 09. Guest 图形、GPU 与显示系统

## 图形完整链路

Android 图形路径通常涉及应用渲染、BufferQueue、SurfaceFlinger、Hardware Composer、Gralloc Allocator/Mapper、EGL/GLES/Vulkan、Kernel DRM/GPU 驱动以及 VMM/Host GPU 后端。ZeroVM 必须同时解决 Guest 图形栈和 Host 显示传输。

## 组件职责

- Gralloc：分配和描述图形 Buffer。
- Mapper：导入、映射和查询 Buffer。
- Composer HAL：向 SurfaceFlinger 提供显示合成接口。
- EGL/GLES：OpenGL ES 上下文和渲染。
- Vulkan HAL：Vulkan 实现和能力暴露。
- DRM/KMS：Kernel 图形设备和显示资源管理。
- DMA-BUF：跨设备/进程共享 Buffer 的 Linux 机制。
- Sync Fence：GPU/显示 Buffer 同步。
- VirtIO GPU：Guest 与 VMM 间的虚拟 GPU 设备协议。
- gfxstream：可研究的图形命令转发/虚拟 GPU 技术。

## 端到端链路

Guest App → Android Graphics Framework → SurfaceFlinger → Composer/Gralloc → Guest EGL/Vulkan → Guest DRM/VirtIO GPU → VMM GPU Backend → Host GPU 或软件渲染 → Host Surface。

每段都定义 Buffer 所有权、同步 fence、格式、stride、modifier、生命周期和错误恢复。

## 加速模式

- 硬件加速虚拟 GPU：需要 Host GPU 后端、驱动和共享资源能力全部兼容。
- 软件渲染：作为兼容后备，性能和功耗通常较差。
- Headless：用于启动、ADB、系统服务和自动化测试，不提供交互桌面。

不能把 Guest 看到 VirtIO GPU 当作图形加速完成。必须验证 Gralloc、Composer、EGL/Vulkan、SurfaceFlinger 和实际帧输出。

## Host 显示容器

Host App 可使用 SurfaceView/TextureView 或其他 Surface 容器显示 Guest 帧，但 Surface 容器本身不会自动接收 VMM framebuffer。必须实现明确的帧传输、Buffer 导入或编码传输机制。

## 自适应显示

根据 Host 窗口尺寸、屏幕分辨率、刷新率、热状态和 GPU 能力，选择 Guest 分辨率、密度、帧率上限和渲染路径。显示尺寸变化应区分窗口变化与 Guest display configuration 变化，并正确处理旋转、Insets 和输入坐标变换。

## 验收

Guest SurfaceFlinger 稳定启动；Composer 和 Gralloc 实际工作；应用窗口可显示；Buffer 不持续泄漏；Fence 不死锁；旋转和尺寸变化正确；GPU reset 或 Host Surface 销毁后可恢复或报告明确错误。