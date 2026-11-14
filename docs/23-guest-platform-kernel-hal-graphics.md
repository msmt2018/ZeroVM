# 23. Guest Platform：Kernel、Vendor HAL、Android Framework 与图形架构

## 1. Guest Platform 的定义

Guest Platform 是让 Android System 能在 ZeroDroid 虚拟硬件上运行的完整底座，不等于一份 Vendor 镜像。它包含 Kernel、模块、DTB、启动材料、分区布局、Vendor/HAL、VINTF、SELinux policy、init 配置和虚拟设备 ABI。

## 2. 版本化平台策略

每个平台绑定：
- Android release/API；
- Android Common Kernel/GKI 分支和 KMI；
- boot header 版本；
- Vendor build 与 HAL 接口；
- VINTF compatibility matrix；
- 虚拟硬件 ABI；
- AVB 信任链；
- 图形和设备桥接实现版本。

禁止将不同 Android 版本的 HAL、Kernel 模块和 Ramdisk 随意混搭。

## 3. Kernel 工程

从对应 Android Common Kernel/Cuttlefish 参考分支建立可复现构建。维护 defconfig、fragment、必要 patch、模块清单、DTS/DTB 生成、符号/KMI 检查和产物哈希。

内核功能按启动依赖排序：CPU/内存/中断、VirtIO transport、Block/Console、文件系统、Device Mapper、Binder、网络、图形和其他设备。早期启动依赖必须内建或在 Ramdisk 中可加载。

## 4. Vendor HAL 工程

先从目标 Android 版本的 Cuttlefish/AOSP 虚拟设备 HAL 评估可复用实现。每个 HAL 明确：
- AIDL/HIDL 接口版本；
- instance 名称；
- init rc；
- SELinux domain 和 service context；
- 依赖设备节点/协议；
- Host Bridge 依赖；
- VINTF manifest 声明；
- 功能等级和测试。

未实现的设备不要注册伪 HAL。HAL 服务崩溃、重启和设备消失必须有明确处理。

## 5. VINTF 与系统接口

在每个 Guest Profile 中保存 Vendor Manifest 和 Framework Compatibility Matrix。构建时和导入时执行 VINTF 检查，区分缺失 HAL、版本不兼容、instance 不匹配和 Kernel FCM 不满足。不能以忽略 VINTF 错误作为兼容策略。

## 6. SELinux 与 init

为每个 Native/HAL service 定义专用 domain、文件标签、服务标签和最小访问规则。通过实际 denial 日志逐项修复。不要全局 permissive。

Init 配置应区分 early-init、init、late-init、各阶段 mount 和服务启动依赖。服务必须声明 class、用户/组、权限、重启策略和依赖条件。

## 7. Android Framework 启动

Framework 启动成功至少需要：关键分区挂载、属性服务、servicemanager/hwservicemanager（按版本）、HAL、Zygote、System Server 和必要系统服务。定义 health probes 和启动超时，但超时只用于诊断，不能把未完成阶段标成 RUNNING。

## 8. 图形平台

图形能力是跨层组合：
- Guest Kernel DRM/GPU driver；
- Gralloc Allocator/Mapper；
- Composer HAL；
- EGL/GLES/Vulkan；
- BufferQueue/DMA-BUF/Fence；
- VMM GPU backend；
- Host rendering/display output。

任何一层缺失都可能降级或无法启动 SurfaceFlinger。定义 software rendering、framebuffer transport 和 accelerated graphics 三种不同能力等级，不混为一谈。

## 9. Guest Ready 等级

- G0：Backend 已创建；
- G1：Kernel 已进入并可获取 Console；
- G2：Android first-stage/second-stage init 工作；
- G3：关键分区挂载、HAL/VINTF 通过；
- G4：Zygote/System Server 正常；
- G5：SurfaceFlinger/Launcher 可用；
- G6：输入、网络、存储等声明功能通过端到端测试。

对外显示实际等级和失败阶段，不只显示一个“启动成功”。
