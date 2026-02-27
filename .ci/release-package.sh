#!/usr/bin/env bash
set -euo pipefail

version="${VERSION:-}"
build_dir="${BUILD_DIR:-build}"
dist_dir="${DIST_DIR:-dist}"

if [[ -z "$version" ]]; then
  echo "::error::VERSION is required."
  exit 1
fi

if [[ ! -d "$build_dir" ]]; then
  echo "::error::Build output directory not found: ${build_dir}"
  exit 1
fi

asset_name="puniark-winlator-starterpack-${version}.zip"
asset_path="${dist_dir}/${asset_name}"
repo_root="$(pwd)"

rm -rf "$dist_dir"
mkdir -p "$dist_dir"

(
  cd "$build_dir"
  zip -r "${repo_root}/${asset_path}" .
)

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  printf 'asset_path=%s\n' "$asset_path" >> "$GITHUB_OUTPUT"
fi

printf 'Packaged release asset: %s\n' "$asset_path"
