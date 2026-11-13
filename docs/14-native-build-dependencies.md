# 14. Native 构建环境与第三方依赖

## 语言分工

- Kotlin：Android App、服务、设置和 UI。
- C++：JNI、Android Native 接口、镜像解析、设备 Bridge 和部分 HAL。
- Rust：资源安全、协议解析和 VMM 适配；crosvm 本身为 Rust 项目。
- C：Linux Kernel、AOSP 部分底层库。
- Python/Shell：构建编排、镜像验证和测试工具。

不要求所有模块使用同一种语言。跨语言边界必须有稳定 ABI、错误模型和版本策略。

## 构建系统

可能涉及 Gradle（Android App）、Android NDK/CMake（JNI/Native）、Soong/Blueprint（AOSP Native/HAL）、Cargo（Rust）、Bazel/Ninja（上游项目要求）和对应 Android Kernel 构建系统。

不要在没有评估的情况下将 AOSP Soong 项目整体强行改写为 CMake。

## 关键上游项目

- AOSP：Android Framework、Native、HAL、镜像工具。
- Android Common Kernel：Guest Kernel 基础。
- AVF：Android 虚拟化框架。
- crosvm：VMM 和 VirtIO 设备。
- Cuttlefish：完整虚拟 Android 设备参考实现。
- minigbm：图形 Buffer 管理参考。
- gfxstream：虚拟 GPU/图形命令传输参考。
- Linux VirtIO：Guest 驱动和设备协议。
- libavb/liblp/libsparse/libfs_mgr：镜像和分区基础。

## 第三方依赖治理

每个依赖记录上游 URL、固定 commit/tag、许可证、本地补丁、构建说明、ABI/API 使用范围、安全公告监控方式、升级责任模块和分发限制。

不得仅凭文件名或来源不明二进制引入关键系统组件。优先使用可审计源码和可复现构建。

## 源码目录建议

app/、core/、guest-platform/、kernel/、rust/、third_party/、tools/、tests/ 可逐步建立。目录结构是规划，不代表当前均已创建或可构建。

## 构建产物

Kernel、HAL、VMM 和 Guest Profile 应输出 manifest，记录版本、commit、构建参数、工具链、哈希和许可证。构建产物与源码版本必须一一对应。