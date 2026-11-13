#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "\${BASH_SOURCE[0]}")/../.." && pwd)"
THIRD_PARTY="$ROOT/third_party"
AOSP_DIR="$THIRD_PARTY/aosp"
MANIFEST="$THIRD_PARTY/manifest/zerovm.xml"
JOBS="\${ZEROVM_SYNC_JOBS:-4}"
DEPTH="\${ZEROVM_SYNC_DEPTH:-1}"

if ! command -v repo >/dev/null 2>&1; then
  echo "Install the official Android repo launcher: https://source.android.com/docs/setup/reference/repo" >&2
  exit 2
fi
command -v git >/dev/null 2>&1 || { echo "git is required" >&2; exit 2; }
command -v python3 >/dev/null 2>&1 || { echo "python3 is required" >&2; exit 2; }
test -f "$MANIFEST" || { echo "Missing manifest: $MANIFEST" >&2; exit 2; }

mkdir -p "$AOSP_DIR"
TMP_MANIFEST="$(mktemp -d)"
cleanup() { rm -rf "$TMP_MANIFEST"; }
trap cleanup EXIT
cp "$MANIFEST" "$TMP_MANIFEST/default.xml"
git -C "$TMP_MANIFEST" init -q -b main
git -C "$TMP_MANIFEST" config user.name "ZeroVM Source Sync"
git -C "$TMP_MANIFEST" config user.email "zerovm-source-sync@localhost"
git -C "$TMP_MANIFEST" add default.xml
git -C "$TMP_MANIFEST" commit -q -m "ZeroVM pinned upstream manifest"

INIT_ARGS=(init -u "file://$TMP_MANIFEST" -b main -m default.xml --repo-url=https://gerrit.googlesource.com/git-repo --no-clone-bundle)
if [[ "$DEPTH" != "0" ]]; then INIT_ARGS+=(--depth="$DEPTH"); fi
(
  cd "$AOSP_DIR"
  repo "\${INIT_ARGS[@]}"
  repo sync -c --no-tags -j"$JOBS"
  repo manifest -r -o "$THIRD_PARTY/SOURCE_LOCK.xml"
)

python3 - "$THIRD_PARTY/SOURCE_LOCK.xml" "$THIRD_PARTY/SOURCE_LOCK.json" <<'PY'
import json, pathlib, sys, xml.etree.ElementTree as ET
manifest_path, output_path = map(pathlib.Path, sys.argv[1:])
root = ET.parse(manifest_path).getroot()
remotes = {n.get("name"): n.get("fetch", "") for n in root.findall("remote")}
default = root.find("default")
projects = []
for n in root.findall("project"):
    remote = n.get("remote") or default.get("remote")
    projects.append({
        "name": n.get("name"),
        "path": n.get("path") or n.get("name"),
        "remote": remotes.get(remote, ""),
        "revision": n.get("revision"),
        "groups": n.get("groups", "")
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
echo "Source sync complete. Review and commit third_party/SOURCE_LOCK.xml and SOURCE_LOCK.json."
