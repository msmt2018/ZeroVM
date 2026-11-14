# 29. ROM 导入、OTA 解析与分区转换流水线

## 1. 目标

从用户提供的 ROM 包安全提取系统材料，生成可审计的 Guest Image Set。导入器负责解析、转换和报告，不得默认破坏原始镜像或自动绕过安全校验。

## 2. 输入格式

按模块化 Parser 支持 raw partition images、Android Sparse、super.img/liblp、boot/vendor_boot/init_boot/dtbo/vbmeta、OTA payload.bin（全量与增量）、*.new.dat、*.new.dat.br、transfer list、ZIP/TAR 容器，以及有文档或验证依据的厂商 OTA 容器。

格式支持需登记 parser 版本和测试样本。未知格式进入隔离区，不执行包内脚本。

## 3. 安全导入流水线

Input → Quarantine → Archive Inspection → Path Validation → Size/Ratio Limits → Hash → Format Detection → Metadata Parse → Partition Extraction → Image Validation → AVB Analysis → Dependency Graph → Profile Match → Transformation Plan → Output Verification。

防止路径穿越、符号链接逃逸、压缩炸弹、超大分配、整数溢出、恶意 protobuf、畸形 sparse block 和解析器崩溃。限制输入长度、解压比例、分区尺寸、嵌套深度和总磁盘占用。

## 4. OTA 语义

payload.bin 可能是增量 OTA，不能保证仅凭包内数据重建完整分区。增量 payload 需要匹配源分区与版本，并验证 source/target fingerprint、操作列表、源哈希和目标哈希。缺少基线时明确报告无法应用。

*.new.dat.br 需结合 transfer list 和对应分区版本解析；仅 Brotli 解压不代表得到完整可启动分区。

## 5. Sparse、LP 与分区映射

Sparse 转换保留逻辑块数量、块大小和填充语义；liblp 解析识别 metadata slot、group、partition attributes、extent、block device 和动态分区约束；重组前检查分区总量、group 上限、super 容量、slot 与 metadata 一致性。

不将 super.img 当作单一文件系统，不假设所有厂商分区都在 super 中；未知厂商分区名称应保留元数据，不得擅自删除。

## 6. 转换策略

先生成 Transformation Plan，记录输入分区/哈希、操作类型、依赖与兼容依据、输出分区/尺寸/哈希、对 AVB/VINTF/fstab/SELinux/HAL/OTA 的影响、可逆性和恢复方式。

优先保持原始分区不变。替换 Vendor、Kernel、DTB 或 vbmeta 属于高风险变换，只有匹配 Guest Platform Profile 且有验证证据时允许。禁止默认用同一份通用虚拟设备 Vendor 替换所有 ROM。

## 7. AVB 与 dm-verity

解析 vbmeta、descriptors、hashtree、chain partition、rollback index 和 public key 元数据，报告签名与校验关系。

不得把自动剥离 AVB、关闭 dm-verity 或清除验证标志设为通用导入步骤。开发测试若需修改镜像，必须用户明确选择、保留原始输入、标记测试态、说明安全后果并验证修改后的启动链。生产安全模型使用受控信任根和一致签名策略。

## 8. 输出与手机端资源

每次导入生成只读原包归档、分区哈希/大小/格式/状态、Partition Graph、Boot Material Set、AVB Report、Profile 匹配、Transformation Plan、执行日志、阻断项和最终 Guest Image Set。

大镜像处理使用流式 I/O、分块、临时文件配额和可恢复任务。内存预算按 Host 可用内存与压力动态计算并设硬上限，不能把数十 GB 镜像整体读入 RAM。

## 9. 验收

每个 Parser 有合法/畸形/边界样本、模糊测试和哈希回归；每次转换可复现并能解释输入输出差异。
