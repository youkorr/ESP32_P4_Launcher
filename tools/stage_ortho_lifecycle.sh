#!/usr/bin/env bash
# Stage the exact experimental engine commit as an external component.
# No registry upload and no vendored copy in the launcher repository.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
git -C "$tmp" init -q
git -C "$tmp" remote add origin https://github.com/youkorr/ppa-alternative-.git
git -C "$tmp" fetch --depth=1 origin 5defd2243d8b4ec7e5391fb2084d61b157c09e8c
git -C "$tmp" checkout --detach FETCH_HEAD
dst="$root/launcher/experimental_components/lv_draw_ortho"
test ! -e "$dst"
python3 "$tmp/tools/pack_lv_draw_ortho.py" "$dst"
# Keep public registry dependency as the normal build default. This script
# opts a checkout into the pinned external source, not the public package.
python3 - "$root/launcher/main/idf_component.yml" <<'PY'
import pathlib,sys
p=pathlib.Path(sys.argv[1])
s=p.read_text()
old='  youkorr/lv_draw_ortho: "0.2.0"'
assert s.count(old)==1
p.write_text(s.replace(old,'  youkorr/lv_draw_ortho:\n    version: "0.2.0"\n    override_path: ../experimental_components/lv_draw_ortho'))
PY
