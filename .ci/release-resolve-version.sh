#!/usr/bin/env bash
set -euo pipefail

is_semver_tag() {
  local tag="$1"
  [[ "$tag" =~ ^v[0-9]+\.[0-9]+\.[0-9]+(-r[0-9]+)?$ ]]
}

trim_whitespace() {
  local value="$1"
  value="${value#"${value%%[![:space:]]*}"}"
  value="${value%"${value##*[![:space:]]}"}"
  printf '%s' "$value"
}

is_base_greater() {
  local left="${1#v}"
  local right="${2#v}"
  local left_major left_minor left_patch
  local right_major right_minor right_patch

  IFS='.' read -r left_major left_minor left_patch <<< "$left"
  IFS='.' read -r right_major right_minor right_patch <<< "$right"

  if (( 10#${left_major} != 10#${right_major} )); then
    (( 10#${left_major} > 10#${right_major} ))
    return
  fi
  if (( 10#${left_minor} != 10#${right_minor} )); then
    (( 10#${left_minor} > 10#${right_minor} ))
    return
  fi
  (( 10#${left_patch} > 10#${right_patch} ))
}

raw_input="$(trim_whitespace "${INPUT_VERSION:-}")"
if [[ -n "$raw_input" && "$raw_input" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-r[0-9]+)?$ ]]; then
  raw_input="v${raw_input}"
fi

mapfile -t all_tags < <(git tag --list)
semver_tags=()
declare -A all_bases=()
for tag in "${all_tags[@]}"; do
  if is_semver_tag "$tag"; then
    semver_tags+=("$tag")
    base="${tag%%-r*}"
    all_bases["$base"]=1
  fi
done

if [[ -n "$raw_input" ]]; then
  if ! is_semver_tag "$raw_input"; then
    echo "::error::Invalid version input. Use vMAJOR.MINOR.PATCH or vMAJOR.MINOR.PATCH-rN."
    exit 1
  fi
  resolved="$raw_input"
  reason="manual"
else
  mapfile -t head_tags < <(git tag --points-at HEAD)
  declare -A head_bases=()
  for tag in "${head_tags[@]}"; do
    if is_semver_tag "$tag"; then
      base="${tag%%-r*}"
      head_bases["$base"]=1
    fi
  done

  head_base_list=()
  for base in "${!head_bases[@]}"; do
    head_base_list+=("$base")
  done

  if (( ${#head_base_list[@]} > 1 )); then
    mapfile -t sorted_head_bases < <(printf '%s\n' "${head_base_list[@]}" | sort -V)
    joined_bases="$(printf '%s, ' "${sorted_head_bases[@]}")"
    joined_bases="${joined_bases%, }"
    echo "::error::HEAD has multiple semver version lineages tagged (${joined_bases}). Specify the version input explicitly."
    exit 1
  fi

  if (( ${#head_base_list[@]} == 1 )); then
    base="${head_base_list[0]}"
    max_revision=0
    for tag in "${semver_tags[@]}"; do
      if [[ "${tag%%-r*}" == "$base" && "$tag" =~ -r([0-9]+)$ ]]; then
        revision="${BASH_REMATCH[1]}"
        if (( 10#${revision} > max_revision )); then
          max_revision=$((10#${revision}))
        fi
      fi
    done
    resolved="${base}-r$((max_revision + 1))"
    reason="head-tag-revision"
  elif (( ${#all_bases[@]} > 0 )); then
    latest_base=""
    for base in "${!all_bases[@]}"; do
      if [[ -z "$latest_base" ]] || is_base_greater "$base" "$latest_base"; then
        latest_base="$base"
      fi
    done

    latest_trimmed="${latest_base#v}"
    IFS='.' read -r major minor patch <<< "$latest_trimmed"
    resolved="v$((10#${major})).$((10#${minor})).$((10#${patch} + 1))"
    reason="patch-bump"
  else
    resolved="v0.0.1"
    reason="bootstrap"
  fi
fi

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    printf 'version=%s\n' "$resolved"
    printf 'reason=%s\n' "$reason"
  } >> "$GITHUB_OUTPUT"
fi

printf 'Resolved release version: %s (%s)\n' "$resolved" "$reason"
