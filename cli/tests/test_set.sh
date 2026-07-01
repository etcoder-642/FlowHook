#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")" || exit 1

PASS=0
FAIL=0

source ./test_helpers.sh
# test_set.sh

test_set_active() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --active >/dev/null 2>&1
  local out
  out=$(flowhook check --active 2>&1)
  assert_contains "$out" "is active" "set --active makes task active"
}

test_set_deactive() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --active >/dev/null 2>&1
  flowhook set --deactive >/dev/null 2>&1
  local out
  out=$(flowhook check --deactive 2>&1)
  assert_contains "$out" "is deactive" "set --deactive makes task deactive"
}

test_set_active_then_check_not_deactive() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --active >/dev/null 2>&1
  local out
  out=$(flowhook check --deactive 2>&1)
  assert_contains "$out" "not deactive" "active task is not deactive"
}

test_set_deactive_then_check_not_active() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --deactive >/dev/null 2>&1
  local out
  out=$(flowhook check --active 2>&1)
  assert_contains "$out" "not active" "deactive task is not active"
}

test_set_depth() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --depth 7 >/dev/null 2>&1
  local out
  out=$(flowhook check --depth 2>&1)
  assert_contains "$out" "7" "set --depth 7 reflected in check --depth"
}

test_set_depth_overwrite() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --depth 3 >/dev/null 2>&1
  flowhook set --depth 9 >/dev/null 2>&1
  local out
  out=$(flowhook check --depth 2>&1)
  assert_contains "$out" "9" "set --depth overwrites previous depth"
  assert_not_contains "$out" "3" "old depth not present after overwrite"
}

test_set_active_and_deactive_together_last_wins() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook set --active --deactive >/dev/null 2>&1
  local out
  out=$(flowhook check --deactive 2>&1)
  assert_contains "$out" "is deactive" "set --active --deactive together: deactive wins (last processed)"
}

test_set_no_flags_is_noop() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local code
  flowhook set >/dev/null 2>&1
  code=$?
  assert_exit 0 "$code" "set with no flags does not crash"
}
