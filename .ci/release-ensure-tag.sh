#!/usr/bin/env bash
set -euo pipefail

version="${VERSION:-}"
current_commit="${GITHUB_SHA:-$(git rev-parse HEAD)}"

if [[ -z "$version" ]]; then
  echo "::error::VERSION is required."
  exit 1
fi

if git rev-parse --quiet --verify "refs/tags/${version}" >/dev/null; then
  tag_commit="$(git rev-list -n 1 "${version}")"
  if [[ "$tag_commit" != "$current_commit" ]]; then
    echo "::error::Tag ${version} already exists on ${tag_commit}, not ${current_commit}."
    exit 1
  fi
  echo "Tag ${version} already exists on this commit."
else
  git tag -a "${version}" -m "Release ${version}"
  git push origin "refs/tags/${version}"
fi
