# 21. 镜像仓库、分区引擎与 Guest Profile 框架

## 1. 子系统职责

镜像系统负责导入、识别、解析、校验、存储、映射、差分、快照和回收。它不负责启动 VMM；Boot Orchestrator 使用它提供的结构化结果。

## 2. 镜像处理流水线

```text
Import Request
 → Source Validation
 → Safe Staging
 → Hash & Size
 → Format Detection
 → Header/Metadata Parsing
 → Partition Graph
 → AVB Analysis
 → Guest Profile Matching
 → Compatibility Report
 → Immutable Image Store
 → Per-VM Writable Overlay
 → Backend Block Mapping
```

每阶段必须可取消、可恢复、可观测，并对磁盘空间和 CPU/内存开销设上限。

## 3. 格式适配器

按格式独立实现：
- Android Sparse；
- Raw block image；
- EXT4/EROFS/F2FS 文件系统镜像；
- GPT；
- Android Dynamic Partitions/liblp；
- Boot Image Header；
- Vendor Boot；
- Init Boot；
- VBMeta/AVB；
- OTA payload（后续阶段）。

格式识别依据 magic/header/metadata，不依据扩展名。所有解析器都必须进行整数溢出、偏移范围、压缩展开大小、嵌套深度和资源耗尽保护。

## 4. 第三方组件选择

优先评估 AOSP libsparse、liblp、libavb、libfs_mgr、libbase、liblog、mkbootimg/unpack_bootimg 和适用文件系统工具。选型前确认：
- 源码版本和许可证；
- Android/NDK 可构建性；
- 上游测试覆盖；
- 依赖链和补丁维护；
- 是否能以库方式集成；
- 是否存在命令行工具依赖或宿主环境假设。

不能仅因源码位于 AOSP 就认为它是稳定公开 NDK API。

## 5. Image Catalog 与存储模型

区分：
- 原始导入文件；
- 规范化/转换后的只读基础镜像；
- 逻辑分区视图；
- VM 专属可写 overlay；
- VM userdata/metadata；
- 快照和备份。

基础镜像按内容哈希去重，导入内容不可变；每台 VM 的写入必须进入自己的 overlay 或虚拟数据盘。删除镜像前检查引用计数、运行状态、快照依赖和正在进行的操作。

## 6. Copy-on-Write 选择条件

只有在后端块设备能力、写时复制语义、崩溃恢复和空间管理已验证时采用 CoW。需要明确：
- backing image 不可变；
- overlay 格式和版本；
- block size/alignment；
- 写入顺序和 flush；
- discard/TRIM 语义；
- overlay 链深度；
- 合并时机；
- 空间预留和上限；
- 崩溃恢复与一致性校验。

若 CoW 后端不可靠，先使用独立完整虚拟磁盘，不能为了节省空间牺牲数据一致性。

## 7. Guest Profile 模型

Guest Profile 描述一组可验证的 Guest Platform：
- Profile ID、schema version、revision；
- Android release/API/ABI；
- Kernel commit/config/hash/module ABI；
- Boot Header version 和 boot artifact；
- DTB/DTBO；
- Ramdisk/fstab/bootconfig；
- 分区布局与文件系统；
- Vendor build、HAL 清单、VINTF；
- AVB root/chain 和验证策略；
- 虚拟硬件 ABI 及设备 feature；
- 推荐资源范围和动态预算规则；
- 已验证 Backend、Host 设备范围；
- 启动阶段验证结果和已知限制。

Profile 必须有 schema migration 和向后兼容策略。未知字段要按 schema policy 处理，不能静默误读。

## 8. 匹配算法

输入为镜像元数据、Host 能力、Backend 能力和 Profile。输出不是单一兼容 boolean，而是：
- 完全匹配；
- 可匹配但有限制；
- 需要补充镜像；
- 接口/版本冲突；
- 后端不支持；
- 无法判断。

匹配过程解释每条约束和来源。禁止仅按 Android 版本字符串匹配 Vendor。

## 9. 分区图

Partition Graph 应表达物理容器、逻辑分区、slot、extent、文件系统、只读/可写、挂载点、AVB descriptor 和依赖。Backend block mapping 由图生成，不允许固定把某个 /dev/vdX 当成 system。

## 10. 安全

镜像导入属于处理不可信输入。使用最小权限、沙箱化解析（可行时）、文件大小限制、哈希、签名/来源信息、路径规范化和防止目录穿越。镜像不能覆盖 Host 真实分区或任意应用私有文件。

## 11. 验收

测试稀疏镜像、截断文件、错误 Header、恶意 LP metadata、整数溢出、损坏 vbmeta、超大压缩输入、空间不足、并发导入、取消、进程死亡、overlay 崩溃和快照恢复。
