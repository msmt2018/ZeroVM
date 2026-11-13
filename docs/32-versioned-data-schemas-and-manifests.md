# 32. VM、ROM、Guest Profile 与虚拟硬件数据结构规范

## 1. 基本规则

本文定义 ZeroVM 持久化数据和跨模块 DTO 的规范模型。正式实现可采用 JSON、CBOR、Protocol Buffers 或内部 Kotlin/C++ 类型，但必须保持字段语义、版本迁移和校验规则。

所有 schema 必须有独立 schema_id、schema_version 和可选 revision；使用稳定 ID；区分用户期望、解析事实、Backend 协商结果和运行时观测；记录来源、校验状态和时间；对未知字段执行显式策略；所有数值设范围、单位和溢出检查。迁移前备份原始数据，迁移失败不得覆盖源文件。

## 2. 标识与单位

- vm_id：创建时生成 UUID/ULID，生命周期内稳定，不复用。
- image_id：内容寻址 ID，关联 SHA-256 或更强摘要及格式版本。
- profile_id：Guest 平台 Profile 稳定逻辑 ID；profile_revision 标识具体修订。
- device_id：VM 内稳定设备 ID，不等同 Host 设备节点。
- operation_id：一次异步操作 ID，不作为 VM ID。
- 时间统一使用 UTC Unix epoch 毫秒或 RFC 3339 UTC；单一 schema 内不得混用。
- 容量统一字节，频率统一 Hz，时间字段明确 ns/ms 后缀。

## 3. VM Spec

VM Spec 表达用户期望配置，不是 Backend 最终运行状态。推荐结构：

    {
      "schema_id": "zerovm.vm-spec",
      "schema_version": 1,
      "vm_id": "01J...",
      "display_name": "Android Guest",
      "guest_profile": {"profile_id": "aosp-gsi-arm64-api35", "revision": "2026.1"},
      "image_set_id": "imageset:sha256:...",
      "resource_policy": {
        "cpu": {"mode": "adaptive", "min_vcpus": 2, "max_vcpus": 6},
        "memory": {"mode": "adaptive", "min_bytes": 2147483648, "max_bytes": 6442450944},
        "storage": {"quota_bytes": 34359738368}
      },
      "device_policy": {
        "display": "required", "network": "optional",
        "camera": "disabled", "microphone": "disabled", "usb": "disabled"
      },
      "security": {
        "vm_isolation": "required",
        "verified_boot_policy": "profile_defined"
      }
    }

校验规则：
- min 不得大于 max；预算不得超过 Host/Backend 硬上限。
- adaptive 范围只是策略边界，不代表运行时一定分配 max。
- 用户不能通过 VM Spec 指定 Host 任意路径、真实 block device 或任意设备节点。
- ROM 元数据不得自动降低安全策略。
- 必需设备不可协商时 preflight 失败；可选设备降级必须出具报告。

## 4. VM Runtime Snapshot

Runtime Snapshot 是当前事实快照，至少包括 vm_id、runtime_generation、state、state_revision、backend_id/version、requested_resources、negotiated_resources、observed_usage、negotiated_devices、current_operation_id、last_error、created/started/updated 时间、guest_boot_stage、guest_health 和持久化 overlay 引用。

Snapshot 不得作为下一次启动配置覆盖 VM Spec。观察值和用户期望值必须分开保存。

## 5. ROM Manifest 与 Image Set

ROM Manifest 描述导入材料来源和解析事实，不代表可启动。字段至少包括：
- source_type、source_digest、source_size_bytes、source_name；
- source_version、target_version（可确定时）；
- detected_format、container_format；
- partitions：名称、类型、大小、摘要、slot、逻辑/物理属性、文件系统和只读属性；
- boot artifacts：boot/vendor_boot/init_boot、kernel、ramdisk、dtb/dtbo；
- vbmeta/AVB descriptors 与验证结论；
- VINTF manifest/matrix 解析结果；
- provenance、parser_version、warnings、errors。

文件路径只是本机存储 locator，不是 ROM 身份。压缩容器摘要与解包分区摘要分开记录。Image Set 表示不可变分区材料集合，记录分区间版本一致性约束、来源和转换历史。禁止混合不同版本 system/vendor/boot 而不生成兼容性评估。

## 6. Guest Platform Profile

Profile 至少包含：
- Android release/API、ABI、VNDK/VINTF level；
- Kernel source/revision、config digest、KMI、modules；
- Boot image header version、bootconfig、ramdisk/fstab；
- DTB/DTBO、启动参数和 init 约束；
- 分区图、动态分区 group/extent、slot 语义；
- Vendor build identity、HAL instance/version、VINTF；
- SELinux policy identity、AVB chain/trust policy；
- 虚拟硬件 ABI 版本与必需/可选设备；
- Backend constraints、Host capability constraints；
- 资源建议范围和动态预算约束；
- 已验证 ROM 家族/设备/版本组合；
- 测试等级、结果、已知问题和证据引用。

Profile 状态建议为 DRAFT、PARSED、COMPATIBILITY_CHECKED、BOOT_TESTED、FRAMEWORK_TESTED、DEVICE_VALIDATED、DEPRECATED。解析成功不能标记为已验证。

## 7. Virtual Hardware Manifest

设备项至少包含 device_id、device_type、device_abi_major/minor、transport、topology/address/interrupt（后端可表达时）、requested_features、required_features、optional_features、queue/interrupt/buffer limits、guest_driver_constraints、host_backend_constraints、security_domain、permission_policy、hotplug/reset/migration/snapshot 支持声明和 validation state。

设备清单必须拆分 requested manifest 与 negotiated manifest。DTB/Guest 配置由协商结果生成或验证，避免配置声称存在实际没有的设备。

## 8. Partition Graph Schema

分区图表达 disk、GPT partition、super container、logical partition、filesystem、boot artifact 节点及父子关系、extent、slot、属性、容量、文件系统、挂载点、backing image、overlay、AVB descriptor、hashtree、rollback index、读写策略、稀疏格式、块大小和对齐。

所有 offset/length 做无符号范围校验；计算 offset+length 必须检查溢出。Extent 必须在容器边界内且不得非法重叠，并按具体格式区分允许的重叠语义。

## 9. Host Capability 与 Resource Budget

Host Capability Manifest 记录 OS/API/kernel/ABI、CPU topology/features、虚拟化扩展探测、AVF/KVM/vendor backend API、服务和权限、device node 可见性与访问验证、GPU backend 逐项探测、内存压力、可用 RAM、存储余量、温度/电源状态、probe source、timestamp、confidence 和 error。

Resource Budget 记录 requested range、host reserve、backend hard limit、profile minimum/recommendation、per-VM allocation、global remaining budget、pressure policy、降级动作、budget revision、有效期和计算原因。

一次探测结果不能永久缓存为设备能力，必须有失效策略和运行时重新探测。

## 10. Schema 演进与迁移

- 添加可选字段且默认语义明确可 minor 升级。
- 删除字段、改变单位/含义或安全默认值必须 major 升级。
- migration 先校验输入，再写临时文件，校验成功后原子替换。
- 未知 major 默认拒绝写入，可提供只读诊断。
- 保留原始来源和迁移链。
- migration 不得自动改变 AVB、SELinux、设备授权和安全等级。

## 11. 测试夹具与验收

仓库后续应提供最小合法 VM Spec、adaptive 资源边界、完整 Cuttlefish Profile、GSI-only 不完整 Image Set、动态分区 super 图、AVB 链损坏、设备 feature 缺失、未知字段/major、数值溢出、重复 ID、路径穿越和超大数组 fixture。

Kotlin、C++、Rust 对同一 fixture 的解析结果必须一致；迁移有逐版本测试；容量和 offset 统一字节且有范围检查；VM Spec、Runtime Snapshot、ROM Manifest 和 Profile 不得混淆；设备配置来自 negotiated manifest；未验证 Profile 不得显示为稳定可运行。