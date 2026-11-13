# 11. 安全模型与隔离

## 安全目标

- Guest 内存与 Host 及其他 VM 隔离。
- Guest Kernel 与 Android 用户空间独立。
- VM 之间的可写磁盘、密钥和设备身份隔离。
- Bridge 最小权限。
- 镜像导入和解析过程抵抗恶意输入。
- VM 生命周期操作可审计。
- 发生崩溃时安全释放资源。

## 隔离层次

1. Hypervisor/Stage-2 地址转换提供 Guest 内存边界。
2. VMM 对 vCPU、设备和内存进行资源管理。
3. Guest 自己运行 SELinux、Binder、UID 和权限体系。
4. VM 数据目录、密钥和虚拟磁盘相互隔离。
5. Host/Guest Bridge 使用明确协议和最小授权。
6. App 与 Native Core 的接口校验所有输入和句柄。

## AVB

AVB 用于验证 Guest 分区完整性。导入镜像时解析 vbmeta 和验证链，运行时按 Guest Profile 执行验证策略。区分验证成功、签名未知、验证失败、验证关闭和缺少 metadata。不得默认关闭验证以绕过问题。

## SELinux

Guest SELinux 以 enforcing 为目标。为 VirtIO、HAL、Bridge 和系统服务编写最小权限策略。根据真实 denial 日志修复规则，不以全局 permissive 作为兼容策略。

## 密钥与可信执行环境

每台 VM 使用独立虚拟密钥空间。软件 KeyMint 的安全等级必须如实标记，不能宣称软件密钥具备 StrongBox/TEE 安全保证。Host TEE 资源只能通过平台明确支持且安全评审通过的接口使用。

## 镜像解析安全

解析 Sparse、GPT、LP、Boot、EROFS、EXT4 和 AVB 时，校验所有长度、偏移、数量和整数运算；防止整数溢出和越界读取；限制解压大小、压缩比、嵌套深度和文件数量；使用只读输入和临时工作区；校验哈希和来源元数据；失败时原子回滚。

## Bridge 安全

不暴露任意文件访问；每个设备能力单独授权；限制消息大小、速率和并发；保护超时、断连和恶意请求；相机、麦克风、位置等敏感能力提供明确授权和状态提示。

## VM 数据生命周期

删除 VM 时明确处理磁盘、快照、缓存、密钥和日志。快照可能包含凭据和应用数据，应按敏感数据保护。Factory Reset 需要销毁或重新生成 VM 数据密钥，并验证旧数据不可继续访问。