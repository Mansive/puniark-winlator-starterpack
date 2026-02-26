#!/usr/bin/env bash
set -euo pipefail

version="${VERSION:-}"
current_commit="${GITHUB_SHA:-$(git rev-parse HEAD)}"
tagger_name="github-actions[bot]"
tagger_email="41898282+github-actions[bot]@users.noreply.github.com"

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
  GIT_COMMITTER_NAME="$tagger_name" \
  GIT_COMMITTER_EMAIL="$tagger_email" \
  GIT_AUTHOR_NAME="$tagger_name" \
  GIT_AUTHOR_EMAIL="$tagger_email" \
  git tag -a "${version}" -m "Release ${version}"

  if git push origin "refs/tags/${version}"; then
    echo "Created and pushed tag ${version}."
  else
    echo "Tag push failed, re-checking remote tag state."
    git tag -d "${version}" >/dev/null 2>&1 || true
    git fetch --force --tags origin

    if git rev-parse --quiet --verify "refs/tags/${version}" >/dev/null; then
      tag_commit="$(git rev-list -n 1 "${version}")"
      if [[ "$tag_commit" != "$current_commit" ]]; then
        echo "::error::Tag ${version} exists on ${tag_commit}, not ${current_commit}."
        exit 1
      fi
      echo "Tag ${version} was created by another run on this commit."
    else
      echo "::error::Failed to push tag ${version}, and tag was not found on remote afterward."
      exit 1
    fi
  fi
fi
