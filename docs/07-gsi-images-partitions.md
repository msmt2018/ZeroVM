# 07. GSI、镜像格式与分区系统

## GSI 不是完整虚拟设备

GSI 是通用 Android System 镜像，目标是符合 Treble 要求的设备。它通常不能独立提供虚拟设备所需 Kernel、Vendor HAL、设备树、启动 Ramdisk、完整分区布局和虚拟硬件驱动。

ZeroVM 提供两种导入路径：

### GSI System Import
用户导入 system.img。ZeroVM 选择经过验证的 Guest Platform Base，匹配 Kernel、Vendor、Boot、VINTF 和虚拟设备 ABI，再执行兼容性检查。

### Full Guest Image Import
用户导入完整 Guest 设备镜像或一组分区镜像。ZeroVM 解析 Boot、Vendor、Super、VBMeta、Data 等材料并验证布局。

两种路径必须在 UI、数据模型和校验逻辑中明确区分。

## 镜像格式

需要识别和处理 Raw block image、Android Sparse Image、EXT4、EROFS、F2FS（主要用于数据盘场景）、GPT、Android Dynamic Partitions/super.img、Android Boot Image、Vendor Boot Image、Init Boot Image、VBMeta/AVB metadata 和后续 OTA Payload。

文件扩展名不是可靠格式判据，必须解析 magic、header、metadata 和边界。

## AOSP 可复用组件

优先研究和评估 libsparse、liblp、libavb、libfs_mgr、libbase、liblog、mkbootimg/unpack_bootimg、EROFS 工具和内核支持、e2fsprogs/ext4 工具。

这些源码不一定都是稳定 NDK SDK。引入时必须记录版本、依赖、构建方式、补丁、许可证和安全更新来源。

## Dynamic Partitions

super.img 可能包含多个逻辑分区及其 metadata。必须解析 LP metadata，识别 group、partition、extent、slot 和大小约束。不能把 GSI system.img 直接当作 super.img，也不能固定假设某个 virtio block 设备永远对应 system/vendor/data。

## 分区布局引擎

Partition Layout Engine 输入镜像检测结果、Guest Profile、Boot/Fstab 信息、LP/GPT metadata、分区容量约束和 AVB descriptor/chain 信息。

输出逻辑分区图、虚拟磁盘映射、挂载关系、可写性和快照策略、校验报告。

所有偏移、长度和乘法运算必须进行溢出检查，解析器应防止越界、恶意尺寸和压缩炸弹。

## AVB 与验证策略

AVB 检查应解析 vbmeta、descriptor、rollback index 和验证链。对用户导入镜像，应区分已验证、签名不受信任、缺少 metadata、验证关闭和验证失败。不能默认通过关闭 verity 来掩盖问题。

## 虚拟磁盘与数据隔离

每台 VM 有独立可写 userdata、metadata 和需要独立修改的分区。可选采用只读基础镜像加 Copy-on-Write 差分层，以减少重复存储。差分层必须有一致性、合并、快照、崩溃恢复和磁盘空间保护机制。

## 刷写语义

离线刷写只能修改 VM 虚拟磁盘。在线刷写通过 Guest Recovery、fastbootd 或 Guest OTA 流程。禁止将宿主物理分区映射为默认刷写目标。刷写前检查目标分区、镜像大小、slot、AVB 和 VM 关机状态。