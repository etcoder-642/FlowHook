#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

# --helper functions--
assert_exit() {
  local expected="$1" actual="$2" desc="$3"
  if [[ "$expected" == "$actual" ]]; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: $desc (expected $expected, got $actual)"
  fi
}

assert_contains() {
  local haystack="$1" needle="$2" desc="$3"
  if [[ "$haystack" == *"$needle"* ]]; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: $desc (expected output to contain: $needle)"
    echo "  actual output: $haystack"
  fi
}

assert_not_contains() {
  local haystack="$1" needle="$2" desc="$3"
  if [[ "$haystack" != *"$needle"* ]]; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: $desc (output should NOT contain: $needle)"
    echo "  actual output: $haystack"
  fi
}

fresh_dir() {
  local d
  d=$(mktemp -d)
  export FLOWHOOK_CONFIG_DIR_TEST="$d/config"
  mkdir -p "$FLOWHOOK_CONFIG_DIR_TEST"
  cd "$d" || exit 1
}
