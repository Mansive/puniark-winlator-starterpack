#!/usr/bin/env bash
set -euo pipefail

version="${VERSION:-}"
asset_path="${ASSET_PATH:-}"
repository="${GITHUB_REPOSITORY:-}"

if [[ -z "$version" ]]; then
  echo "::error::VERSION is required."
  exit 1
fi

if [[ -z "$asset_path" ]]; then
  echo "::error::ASSET_PATH is required."
  exit 1
fi

if [[ ! -f "$asset_path" ]]; then
  echo "::error::Asset not found: ${asset_path}"
  exit 1
fi

if [[ -z "$repository" ]]; then
  echo "::error::GITHUB_REPOSITORY is required."
  exit 1
fi

if gh release view "$version" --repo "$repository" >/dev/null 2>&1; then
  echo "Release ${version} already exists. Uploading latest artifact."
  gh release upload "$version" "$asset_path" --clobber --repo "$repository"
else
  gh release create "$version" "$asset_path" \
    --title "$version" \
    --generate-notes \
    --repo "$repository"
fi
