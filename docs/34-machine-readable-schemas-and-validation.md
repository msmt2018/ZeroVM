# 34. 机器可读 Schema 与契约验证

## 当前仓库状态

本轮首次引入 `schemas/v1`，使用 JSON Schema Draft 2020-12 定义 32 号文档中的九类数据契约，并添加 Python unittest 验证入口。它们是可执行的数据约束，不代表 Kotlin/C++/Rust 序列化层或 Runtime 已实现。

## Schema 清单

| Definition | schema_id |
|---|---|
| VmSpec | `zerodroid.vm-spec` |
| RuntimeSnapshot | `zerodroid.runtime-snapshot` |
| RomManifest | `zerodroid.rom-manifest` |
| ImageSet | `zerodroid.image-set` |
| GuestPlatformProfile | `zerodroid.guest-platform-profile` |
| VirtualHardwareManifest | `zerodroid.virtual-hardware-manifest` |
| PartitionGraph | `zerodroid.partition-graph` |
| HostCapabilityManifest | `zerodroid.host-capability` |
| ResourceBudget | `zerodroid.resource-budget` |

规范文件：`schemas/v1/zerodroid.schema.json`。每个定义位于 `$defs`，由测试 Harness 通过根 schema 引用对应 definition，确保内部引用在同一资源中解析。

## 本地测试

Python 3.10+：

```sh
python -m pip install -r requirements-dev.txt
python -m unittest discover -s tests/schema -v
```

测试覆盖：
- 有效 VM Spec 与 ROM Manifest；
- 无效资源区间（Schema 结构有效，但语义校验必须拒绝）；
- 分区 extent 的 int64 溢出；
- 未知 schema_version；
- 九类契约定义是否齐全。

## Schema 与语义校验的边界

JSON Schema 可检查字段、类型、枚举、必填项和局部范围；跨字段/图结构规则仍需领域校验器。例如 min/max 顺序、partition extent 容器边界和重叠、父子图引用、时间先后、Host 实时资源预算、Guest 必需设备与 Backend negotiated capabilities 的一致性。不能仅凭 JSON Schema validation 就宣称镜像可启动或硬件能力可用。

## 迁移策略与当前限制

v1 只接受 `schema_version: 1`。本轮测试未知版本拒绝，防止新格式被旧 Runtime 静默读写。当前没有 v2 目标定义，也没有真实迁移器，因此不伪造 v1→v2 migration test 或 no-op migration。待 v2 字段语义稳定后，必须增加实际迁移实现、源/目标 golden fixtures、语义不变量测试、原子替换测试和失败回滚测试。

## 状态标签

- Schema 定义：已提交。
- Fixture 与测试代码：已提交。
- 本地/CI 测试执行：需在具备 Python 依赖的仓库工作区运行；本次 GitHub 文件 API 不提供工作区执行环境，因此不报告测试通过。
- Kotlin/C++/Rust 绑定：未实现。
- 真机验证：未执行。
