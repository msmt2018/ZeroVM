# ZeroDroid v1 machine-readable schemas

This directory is the first executable data-contract layer. The canonical JSON Schema uses JSON Schema Draft 2020-12 and exposes nine named definitions under `$defs`:

- `VmSpec`
- `RuntimeSnapshot`
- `RomManifest`
- `ImageSet`
- `GuestPlatformProfile`
- `VirtualHardwareManifest`
- `PartitionGraph`
- `HostCapabilityManifest`
- `ResourceBudget`

The schema is a contract baseline, not evidence that a runtime serializer or migration engine has been implemented. Consumers should validate the selected definition (for example `#/$defs/VmSpec`) and then apply cross-field semantic validation that JSON Schema cannot fully express, including resource min/max ordering, partition extent containment/non-overlap, parent references, time ordering, and negotiated capability consistency.

## Validate fixtures

With Python 3.10+ and `jsonschema` 4.x installed:

```sh
python -m pip install 'jsonschema>=4.21,<5'
python -m jsonschema --instance schemas/v1/fixtures/valid-vm-spec.json --schema schemas/v1/zerodroid.schema.json
```

The CLI command above validates against the document root, so production validation must select the named definition using a validator constructed from `schema["$defs"]["VmSpec"]`. The test harness added in the next implementation stage will validate every fixture against its matching definition.

## Versioning and migration

Version 1 schemas reject unknown schema versions through the `schema_version` constant. A future migration must be a separately implemented, deterministic transformation with:
1. source-version validation;
2. conversion into a temporary destination;
3. destination-version validation;
4. semantic invariant checks;
5. atomic replacement only after all checks pass;
6. preserved source digest and migration provenance.

No v1→v2 migration is claimed by this schema-only commit. A migration implementation and golden migration fixtures are deliberately deferred until a v2 target contract exists; inventing a no-op migration would create false confidence.
