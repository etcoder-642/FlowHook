#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source ./test_helpers.sh

# test_list.sh

test_list_no_flags_is_noop() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out code
  out=$(flowhook list 2>&1)
  code=$?
  assert_exit 0 "$code" "list with no flags exits 0"
  if [[ -z "$out" ]]; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: list with no flags should print nothing"
    echo "  actual output: $out"
  fi
}

test_list_tasks_works_without_init_in_cwd() {
  fresh_dir
  # init in one dir, then list --tasks from a different, uninitialized dir
  flowhook init >/dev/null 2>&1
  local task_dir
  task_dir=$(pwd)
  mkdir -p ../other_dir
  cd ../other_dir || exit 1
  local out
  out=$(flowhook list --tasks 2>&1)
  assert_contains "$out" "$task_dir" "list --tasks shows tasks regardless of cwd"
}

test_list_paths_fails_without_init_in_cwd() {
  fresh_dir
  # no init at all in this fresh dir
  local out
  out=$(flowhook list --paths --commands 2>&1)
  assert_contains "$out" "Error: No flowhook task found" "list --paths/--commands errors when cwd has no task"
}

test_list_ignored_shows_defaults() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook list --ignored 2>&1)
  assert_contains "$out" ".git" "list --ignored shows default ignores even with no user additions"
}

test_list_combined_flags_when_initialized() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  mkdir -p combo
  flowhook add --path ./combo --command "echo combo" >/dev/null 2>&1
  local out
  out=$(flowhook list --paths --commands 2>&1)
  assert_contains "$out" "combo" "combined list flags: path shown"
  assert_contains "$out" "echo combo" "combined list flags: command shown"
}
