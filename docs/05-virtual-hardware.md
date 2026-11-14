# 05. 虚拟硬件平台与 ABI

## 目标

ZeroDroid Virtual Hardware Platform 定义 Guest 可见的虚拟设备、设备标识、总线拓扑、设备能力和版本协商方式。虚拟硬件 ABI 尽量稳定，具体设备能力根据 Host 和 Backend 能力协商。

## 设备类别

| 类别 | 设备/接口 | 主要职责 |
|---|---|---|
| CPU | ARM64 vCPU、定时器 | Guest 指令执行和中断 |
| 内存 | Guest RAM、共享内存 | Guest 物理内存 |
| 中断 | 虚拟中断控制器 | IRQ 和定时器 |
| 固件 | DTB、Bootconfig、Ramdisk | 描述和启动设备 |
| 存储 | VirtIO Block | 系统盘、数据盘 |
| 网络 | VirtIO Net | Guest 网络接口 |
| 通信 | VirtIO Console、Vsock | 控制台和 Host/Guest 通信 |
| 输入 | VirtIO Input | 触摸、键盘、鼠标 |
| 图形 | VirtIO GPU / DRM | GPU 命令和显示 Buffer |
| 音频 | VirtIO Sound 或受控 Bridge | 音频输入输出 |
| 电源 | 虚拟电源/电池服务 | 电池与充电状态 |
| 传感器 | 虚拟 Sensor HAL | 传感器事件 |
| 相机 | Camera Bridge | 相机帧数据 |
| USB | 虚拟 USB 控制器（可选） | USB 设备转发 |

第一版不必实现所有设备。未实现设备应不暴露，或通过受支持接口报告能力缺失。

## VirtIO

VirtIO 是虚拟设备与 Guest 驱动之间的标准化接口。典型组成包括 virtqueue、descriptor、available/used ring、feature negotiation 和中断通知。

设备模型明确设备 ID 和 feature bits、Queue 数量和大小、DMA/共享内存约束、中断行为、Reset 和错误恢复、设备版本兼容规则以及 Host Backend 权限边界。

优先使用上游 Linux VirtIO 驱动和成熟 VMM 设备实现，不自行创造不兼容私有协议。

## Guest Device Tree

ARM64 Guest 需要启动平台提供与实际虚拟设备拓扑一致的 DTB。DTB 描述 CPU、内存区域、中断控制器、定时器、VirtIO 设备节点、控制台和其他设备。DTB 必须与 VMM 实际暴露的设备地址、中断号、队列和设备类型一致。生成器应有 schema、校验和 golden tests。

## 虚拟硬件 ABI 版本

每个 VM 配置记录 hardware_abi_major、hardware_abi_minor、device_manifest_version，以及每个设备的 ID、版本、feature set、Guest 驱动最低要求和后端实现版本。

Major 用于不兼容变化，Minor 用于向后兼容扩展。Guest Profile 声明支持的 ABI 范围。设备缺失和 feature 缺失必须显式记录。

## 硬件身份

Guest 看到的产品名称、设备型号、序列号和硬件属性来自 VM 独立配置，不得直接泄漏 Host 真实设备标识。身份字段稳定、可重置并与每台 VM 绑定。

## 直通与桥接边界

设备直通需要 IOMMU、设备隔离、驱动解绑、权限和宿主稳定性条件。普通 Android 应用通常无法安全执行通用平台设备直通。第一阶段优先 VirtIO 和受控数据 Bridge，不承诺任意物理设备直通。