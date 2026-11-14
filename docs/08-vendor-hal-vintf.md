# 08. Virtual Vendor Platform、HAL 与 VINTF

## Vendor 的职责

Guest System 通过稳定的 Treble 接口与 Vendor 实现交互。ZeroDroid 必须提供与虚拟硬件匹配的 Vendor Platform，不能只提供 system.img 就期待 Android Framework 自动发现所有虚拟设备。

## HAL 范围

根据 Android 版本和接口演进，HAL 可能使用 HIDL 或 AIDL。需要关注 Graphics Allocator/Mapper/Composer、Audio、Camera Provider、Sensors、Power、Thermal、Health、Vibrator、Lights、GNSS、Bluetooth、Wi-Fi、USB、KeyMint 和 Gatekeeper。

具体接口版本由 Guest Android release 和 VINTF 约束决定，不能将某个版本 HAL 二进制复制到其他 Android 版本就假定兼容。

## VINTF

每个 Guest Profile 管理 Vendor Manifest、Framework Compatibility Matrix、适用的 Device Compatibility Matrix、HAL instance 名称和版本、Kernel FCM/requirements、Manifest fragment 和 VINTF validation 结果。

VINTF 是 System 与 Vendor 的契约检查机制，不是可忽略的启动装饰文件。

## HAL 实现策略

优先顺序：
1. 使用 AOSP/Cuttlefish 已有且许可证允许的虚拟设备 HAL。
2. 复用上游接口和实现，适配 ZeroDroid 设备 ABI。
3. 为 ZeroDroid 专有桥接设备编写 Native HAL Service。
4. 对无法提供的硬件能力不注册虚假 HAL，或通过受支持接口报告 unavailable。

HAL 必须真实执行功能并处理错误、超时、服务重启和设备断连。

## 版本化平台目录

按接口族和 Guest API 版本组织源码，不为每个版本复制所有公共代码。公共协议、设备桥接和资源管理尽量复用；HIDL/AIDL 适配层和 ABI 特定代码独立。

## SELinux 与 init

每个服务需要对应 init rc、service identity、SELinux domain、file_contexts、hwservice_contexts/service_contexts 和权限规则。全局 permissive 不是修复方案。根据真实 denial 日志定位缺少的最小权限。

## 安全接口边界

软件模拟 KeyMint/Gatekeeper 不等价于 StrongBox 或真实 TEE。Guest 密钥不得未经明确设计直接复用 Host 私钥。无法提供的安全能力应报告能力等级和限制。