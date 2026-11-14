#!/usr/bin/env python3
"""Validate that every upstream manifest project has an Android ARM64 audit record."""
from __future__ import annotations

import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "third_party/manifest/zerodroid.xml"
MATRIX = ROOT / "third_party/porting/source-portability.json"
REQUIRED_AUDIT = (
    "isa",
    "android_abi_bionic",
    "host_os_dependencies",
    "kernel_interfaces",
    "soc_hardware_assumptions",
    "android_security_permissions",
    "runtime_integration",
)
ALLOWED_STATUS = {
    "unreviewed",
    "source-reviewed",
    "android-buildable",
    "device-runnable",
    "integrated",
    "end-to-end-verified",
    "excluded",
}


def fail(message: str) -> None:
    raise SystemExit("Android ARM64 portability matrix: " + message)


def main() -> None:
    root = ET.parse(MANIFEST).getroot()
    expected = {}
    for node in root.findall("project"):
        name = node.get("name")
        path = node.get("path") or name
        if not name or name in expected:
            fail(f"missing or duplicate manifest project: {name!r}")
        expected[name] = path

    data = json.loads(MATRIX.read_text(encoding="utf-8"))
    if data.get("format") != "zerovm-android-arm64-portability-matrix":
        fail("unexpected matrix format")
    if data.get("target_host") != {
        "os": "Android",
        "abi": "arm64-v8a",
        "cpu_isa": "AArch64",
    }:
        fail("target host must be Android arm64-v8a/AArch64")

    actual = {}
    for item in data.get("projects", []):
        name = item.get("name")
        if not name or name in actual:
            fail(f"missing or duplicate matrix project: {name!r}")
        actual[name] = item
        if item.get("path") != expected.get(name):
            fail(f"path mismatch for {name}: {item.get('path')!r}")
        if item.get("portability_status") not in ALLOWED_STATUS:
            fail(f"invalid portability status for {name}")
        audit = item.get("audit", {})
        missing = [key for key in REQUIRED_AUDIT if audit.get(key) not in ALLOWED_STATUS]
        if missing:
            fail(f"{name} has incomplete audit fields: {', '.join(missing)}")

    missing = sorted(set(expected) - set(actual))
    extra = sorted(set(actual) - set(expected))
    if missing or extra:
        fail(f"manifest/matrix mismatch; missing={missing}, extra={extra}")
    if data.get("project_count") != len(expected):
        fail(f"declared project_count {data.get('project_count')} != {len(expected)}")

    print(
        f"Validated {len(expected)} upstream projects; "
        f"all {len(REQUIRED_AUDIT)} Android ARM64 audit dimensions are tracked. "
        "Unreviewed entries are intentionally not considered compatible."
    )


if __name__ == "__main__":
    main()
