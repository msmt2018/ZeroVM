# 16. 术语表

| 术语 | 含义 |
|---|---|
| Host | 运行 ZeroVM 的真实 Android 设备 |
| Guest | VM 内运行的独立操作系统 |
| Hypervisor | 提供硬件虚拟化和隔离的底层机制 |
| VMM | 管理 VM、vCPU、内存和虚拟设备的软件 |
| AVF | Android Virtualization Framework |
| pKVM | Protected KVM，增强型虚拟机隔离机制 |
| KVM | Linux Kernel-based Virtual Machine |
| crosvm | Rust 实现的虚拟机监控器 |
| Cuttlefish | AOSP 完整虚拟 Android 设备参考平台 |
| GSI | Generic System Image，通用 Android System 镜像 |
| GKI | Generic Kernel Image，通用内核镜像 |
| HAL | Hardware Abstraction Layer，硬件抽象层 |
| HIDL | Android 历史硬件接口描述/通信机制 |
| AIDL | Android 接口定义语言及其 IPC 接口机制 |
| VINTF | Vendor Interface，System/Vendor 接口兼容性描述与检查 |
| Treble | Android System 与 Vendor 解耦架构 |
| VirtIO | 虚拟设备通信标准 |
| Vsock | Host/Guest 虚拟 Socket 通信接口 |
| DTB | Device Tree Blob，设备树二进制 |
| Ramdisk | 启动早期使用的内存文件系统 |
| Bootconfig | Android/Linux 启动配置传递机制 |
| AVB | Android Verified Boot |
| VBMeta | AVB 元数据镜像 |
| Dynamic Partitions | Android 逻辑分区机制 |
| Super | 承载 Android Dynamic Partitions 的物理分区容器 |
| Gralloc | Android 图形 Buffer 分配接口 |
| Composer HAL | Android 硬件合成接口 |
| DRM | Linux Direct Rendering Manager |
| DMA-BUF | Linux 跨设备/进程共享 Buffer 机制 |
| Fence | 图形/设备异步操作同步原语 |
| Guest Profile | 描述 Guest Android 版本、Kernel、Vendor 和启动要求的配置 |
| Hardware ABI | Guest 与虚拟设备之间稳定的设备接口契约 |
| Bridge | 在 Host 与 Guest 之间提供受控设备能力的服务 |
| CoW | Copy-on-Write，写时复制存储层 |
| Snapshot | 某个时刻的磁盘或 VM 状态快照 |
| Guest Ready | Android Guest 达到定义的系统可用健康条件 |
