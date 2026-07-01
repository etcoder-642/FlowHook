#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")" || exit 1

PASS=0
FAIL=0

# test_remove.sh

# --- remove --path ---

test_remove_path_removes_from_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local outside_dir
  outside_dir=$(mktemp -d)
  flowhook add --path "$outside_dir" >/dev/null 2>&1
  flowhook remove --path "$outside_dir" >/dev/null 2>&1
  local out
  out=$(flowhook list --paths 2>&1)
  assert_not_contains "$out" "$outside_dir" "remove --path removes externally-added dir from list --paths"
  rm -rf "$outside_dir"
}
test_remove_nonexistent_path_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook remove --path "nonexistent path" 2>&1)
  assert_contains "$out" "Error:" "removing a path never added errors"
}

# --- remove --command ---

test_remove_command_removes_from_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo bye" >/dev/null 2>&1
  flowhook remove --command "echo bye" >/dev/null 2>&1
  local out
  out=$(flowhook list --commands 2>&1)
  assert_not_contains "$out" "echo bye" "remove --command removes it from list --commands"
}

test_remove_nonexistent_command_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook remove --command "nonexistent command" 2>&1)
  assert_contains "$out" "Error:" "removing a command never added errors"
}

# --- remove --on-success ---

test_remove_on_success_removes_from_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --on-success "echo success" >/dev/null 2>&1
  flowhook remove --on-success "echo success" >/dev/null 2>&1
  local out
  out=$(flowhook list --on-success 2>&1)
  assert_not_contains "$out" "echo success" "remove --on-success removes it from list --on-success"
}

test_remove_nonexistent_on_success_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook remove --on-success "nonexistent" 2>&1)
  assert_contains "$out" "Error:" "removing on-success never added errors"
}

# --- remove --on-failure ---

test_remove_on_failure_removes_from_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --on-failure "echo failure" >/dev/null 2>&1
  flowhook remove --on-failure "echo failure" >/dev/null 2>&1
  local out
  out=$(flowhook list --on-failure 2>&1)
  assert_not_contains "$out" "echo failure" "remove --on-failure removes it from list --on-failure"
}

test_remove_nonexistent_on_failure_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook remove --on-failure "nonexistent" 2>&1)
  assert_contains "$out" "Error:" "removing on-failure never added errors"
}

# --- remove --ignored-path ---

test_remove_ignored_path_removes_from_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --ignored-path "myignoredpath" >/dev/null 2>&1
  flowhook remove --ignored-path "myignoredpath" >/dev/null 2>&1
  local out
  out=$(flowhook list --ignored 2>&1)
  assert_not_contains "$out" "myignoredpath" "remove --ignored-path removes it from list --ignored"
}

test_remove_nonexistent_ignored_path_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook remove --ignored-path "nonexistent" 2>&1)
  assert_contains "$out" "Error:" "removing ignored-path never added errors"
}

# --- remove --ignored-pattern ---

test_remove_ignored_pattern_removes_from_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --ignored-pattern "*.jpeg" >/dev/null 2>&1
  flowhook remove --ignored-pattern "*.jpeg" >/dev/null 2>&1
  local out
  out=$(flowhook list --ignored 2>&1)
  assert_not_contains "$out" "*.jpeg" "remove --ignored-pattern removes it from list --ignored"
}

test_remove_nonexistent_ignored_pattern_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook remove --ignored-pattern "*.nonexistent" 2>&1)
  assert_contains "$out" "Error:" "removing ignored-pattern never added errors"
}

test_remove_whole_task_with_force_flag() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook remove -f >/dev/null 2>&1
  local out
  out=$(flowhook list --tasks 2>&1)
  local task_dir
  task_dir=$(pwd)
  assert_not_contains "$out" "$task_dir" "remove -f deletes the whole task"
}

test_remove_whole_task_confirm_yes_deletes() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local task_dir
  task_dir=$(pwd)
  echo "yes" | flowhook remove >/dev/null 2>&1
  local out
  out=$(flowhook list --tasks 2>&1)
  assert_not_contains "$out" "$task_dir" "remove with 'yes' confirmation deletes the task"
}

test_remove_whole_task_decline_keeps_task() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local task_dir
  task_dir=$(pwd)
  echo "no" | flowhook remove >/dev/null 2>&1
  local out
  out=$(flowhook list --tasks 2>&1)
  assert_contains "$out" "$task_dir" "remove declining confirmation keeps the task"
}
