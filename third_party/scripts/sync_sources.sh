#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
THIRD_PARTY="$ROOT/third_party"
AOSP_DIR="$THIRD_PARTY/aosp"
MANIFEST_REPO="$THIRD_PARTY/.manifest-repo"
MANIFEST="$THIRD_PARTY/manifest/zerovm.xml"
JOBS="${ZEROVM_SYNC_JOBS:-4}"
DEPTH="${ZEROVM_SYNC_DEPTH:-1}"

if ! command -v repo >/dev/null 2>&1; then
  echo "Install the official Android repo launcher: https://source.android.com/docs/setup/reference/repo" >&2
  exit 2
fi
command -v git >/dev/null 2>&1 || { echo "git is required" >&2; exit 2; }
command -v python3 >/dev/null 2>&1 || { echo "python3 is required" >&2; exit 2; }
test -f "$MANIFEST" || { echo "Missing manifest: $MANIFEST" >&2; exit 2; }

# Keep the local manifest Git repository persistent: repo sync may refresh the
# manifest project after initialization, so its file:// remote must not vanish.
mkdir -p "$MANIFEST_REPO" "$AOSP_DIR"
if [[ ! -d "$MANIFEST_REPO/.git" ]]; then
  git -C "$MANIFEST_REPO" init -q -b main
  git -C "$MANIFEST_REPO" config user.name "ZeroVM Source Sync"
  git -C "$MANIFEST_REPO" config user.email "zerovm-source-sync@localhost"
fi
cp "$MANIFEST" "$MANIFEST_REPO/default.xml"
git -C "$MANIFEST_REPO" add default.xml
if ! git -C "$MANIFEST_REPO" diff --cached --quiet; then
  git -C "$MANIFEST_REPO" config user.name "ZeroVM Source Sync"
  git -C "$MANIFEST_REPO" config user.email "zerovm-source-sync@localhost"
  git -C "$MANIFEST_REPO" commit -q -m "Update ZeroVM upstream manifest"
fi

INIT_ARGS=(init -u "file://$MANIFEST_REPO" -b main -m default.xml --repo-url=https://gerrit.googlesource.com/git-repo --no-clone-bundle)
if [[ "$DEPTH" != "0" ]]; then INIT_ARGS+=(--depth="$DEPTH"); fi
(
  cd "$AOSP_DIR"
  repo "${INIT_ARGS[@]}"
  repo sync -c --no-tags -j"$JOBS"
  repo manifest -r -o "$THIRD_PARTY/SOURCE_LOCK.xml"
)

python3 - "$THIRD_PARTY/SOURCE_LOCK.xml" "$THIRD_PARTY/SOURCE_LOCK.json" <<'PY'
import json
import pathlib
import sys
import xml.etree.ElementTree as ET

manifest_path, output_path = map(pathlib.Path, sys.argv[1:])
root = ET.parse(manifest_path).getroot()
remotes = {node.get("name"): node.get("fetch", "") for node in root.findall("remote")}
default = root.find("default")
projects = []
for node in root.findall("project"):
    remote = node.get("remote") or default.get("remote")
    projects.append({
        "name": node.get("name"),
        "path": node.get("path") or node.get("name"),
        "remote": remotes.get(remote, ""),
        "revision": node.get("revision"),
        "groups": node.get("groups", "")
    })
output_path.write_text(json.dumps({
    "format": "zerovm-source-lock",
    "format_version": 1,
    "manifest": "third_party/manifest/zerovm.xml",
    "resolved_manifest": "third_party/SOURCE_LOCK.xml",
    "projects": projects
}, indent=2) + "\n", encoding="utf-8")
print("Wrote %s with %d resolved revisions" % (output_path, len(projects)))
PY

echo "Source sync complete. Review and commit SOURCE_LOCK.xml and SOURCE_LOCK.json."
