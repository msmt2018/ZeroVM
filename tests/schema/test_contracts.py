import copy
import json
import unittest
from pathlib import Path

from jsonschema import Draft202012Validator, FormatChecker

ROOT = Path(__file__).resolve().parents[2]
SCHEMA_PATH = ROOT / "schemas/v1/zerovm.schema.json"
FIXTURES = ROOT / "schemas/v1/fixtures"


def load_json(path):
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def validator_for(schema, definition):
    selected = {
        "$schema": schema["$schema"],
        "$defs": schema["$defs"],
        "$ref": f"#/$defs/{definition}",
    }
    return Draft202012Validator(selected, format_checker=FormatChecker())


def semantic_errors(document):
    """Checks invariants that are intentionally outside JSON Schema."""
    errors = []
    policy = document.get("resource_policy", {})
    for key in ("cpu", "memory"):
        value = policy.get(key, {})
        if value.get("mode") == "adaptive" and value.get("min", 0) > value.get("max", -1):
            errors.append(f"resource_policy.{key}: min exceeds max")
    graph = document if document.get("schema_id") == "zerovm.partition-graph" else None
    if graph:
        ids = {node.get("node_id") for node in graph.get("nodes", [])}
        for node in graph.get("nodes", []):
            parent = node.get("parent_id")
            if parent is not None and parent not in ids:
                errors.append(f"partition node {node.get('node_id')}: missing parent {parent}")
            offset = node.get("offset_bytes", 0)
            length = node.get("length_bytes", 0)
            if offset + length > 9223372036854775807:
                errors.append(f"partition node {node.get('node_id')}: extent overflows int64")
    return errors


class SchemaContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.schema = load_json(SCHEMA_PATH)

    def validate(self, definition, document):
        validator_for(self.schema, definition).validate(document)

    def test_valid_vm_spec_fixture(self):
        document = load_json(FIXTURES / "valid-vm-spec.json")
        self.validate("VmSpec", document)
        self.assertEqual([], semantic_errors(document))

    def test_invalid_vm_spec_resource_ranges(self):
        document = load_json(FIXTURES / "invalid-vm-spec-min-greater-than-max.json")
        self.validate("VmSpec", document)
        self.assertTrue(semantic_errors(document))

    def test_valid_rom_manifest_fixture(self):
        self.validate("RomManifest", load_json(FIXTURES / "valid-rom-manifest.json"))

    def test_partition_extent_overflow_fixture(self):
        document = load_json(FIXTURES / "invalid-partition-graph-overflow.json")
        self.validate("PartitionGraph", document)
        self.assertTrue(any("overflows int64" in item for item in semantic_errors(document)))

    def test_unknown_schema_version_is_rejected(self):
        document = load_json(FIXTURES / "valid-vm-spec.json")
        document["schema_version"] = 2
        with self.assertRaises(Exception):
            self.validate("VmSpec", document)

    def test_schema_contains_all_nine_contracts(self):
        expected = {
            "VmSpec", "RuntimeSnapshot", "RomManifest", "ImageSet",
            "GuestPlatformProfile", "VirtualHardwareManifest",
            "PartitionGraph", "HostCapabilityManifest", "ResourceBudget",
        }
        self.assertTrue(expected.issubset(self.schema["$defs"].keys()))


if __name__ == "__main__":
    unittest.main()
