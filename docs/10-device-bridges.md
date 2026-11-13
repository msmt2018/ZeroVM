# 10. Host/Guest 设备桥接

## 原则

Guest 只能通过明确的虚拟设备协议和授权 Bridge 使用 Host 能力。Guest 不直接获得 Host 任意设备节点、文件路径、Binder 服务或系统权限。

每个 Bridge 必须有独立权限、数据格式、生命周期、超时、速率限制和审计日志。

## 网络

优先由 VirtIO Net 提供 Guest 网卡，再由 Host 网络层提供 NAT/路由。处理 DNS、VPN、网络切换、IPv4/IPv6、MTU、连接追踪和 Host 网络权限。Guest 网络隔离策略明确是否允许局域网访问和入站连接。

## 音频

Guest Audio HAL/AudioFlinger 与 Host Audio Bridge 之间传输 PCM 或受控音频流。协商采样率、声道、格式、buffer size、延迟、音量、焦点、静音和设备断连。麦克风桥接遵守 Host 麦克风权限和隐私指示器要求。

## 摄像头

Guest Camera Provider 调用 Camera Bridge；Host 侧通过受支持 Android Camera API 获取帧。处理相机权限、前后台限制、镜头切换、分辨率、帧率、方向、时间戳、颜色格式和隐私状态。Guest 不得绕过 Host 相机权限。

## 传感器与定位

将 Host Sensor/Location 数据转换成 Guest HAL 所需事件。处理坐标系、单位、时间戳、采样率、精度和权限。未获权限或设备不支持时报告 unavailable，不合成虚假物理数据冒充真实传感器。

## 输入

触摸、鼠标、键盘和手写笔事件转换为 Guest Input 设备事件。定义坐标映射、屏幕旋转、压力、工具类型、多点触控、按键码和输入焦点。Guest 输入能力由虚拟硬件 Profile 声明。

## 蓝牙与 USB

蓝牙可通过 Host Bluetooth API 做受限代理，但 Guest 不应直接控制 Host Bluetooth Controller。USB 直通需要设备权限、驱动和隔离能力，属于后续高级能力，不作为基础可用性前提。

## 电池、热和振动

可将 Host 电池和充电状态映射为 Guest 虚拟 Health/Power 状态。振动通过受控 Haptic Bridge 请求 Host 震动，并受权限、用户设置和场景限制。Guest Thermal 状态表达虚拟设备策略，不直接泄漏宿主敏感传感器细节。

## Bridge 协议

建议使用经过认证的 Vsock 通道或后端支持的安全通信通道。协议具备版本协商、请求 ID、消息长度限制、背压、超时、取消、重连和错误码。禁止把任意 Host 文件系统路径暴露给 Guest。