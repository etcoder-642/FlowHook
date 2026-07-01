#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source test_helpers.sh

# --- add --path ---

test_add_path_appears_in_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  mkdir -p subdir
  flowhook add --path ./subdir >/dev/null 2>&1
  local out
  out=$(flowhook list --paths 2>&1)
  assert_contains "$out" "subdir" "add --path shows up in list --paths"
}

test_add_path_nonexistent_fails() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook add --path ./does_not_exist 2>&1)
  assert_contains "$out" "Error:" "add --path on nonexistent path errors"
}

test_add_duplicate_path_no_duplicate() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  mkdir -p dup
  flowhook add --path ./dup >/dev/null 2>&1
  flowhook add --path ./dup >/dev/null 2>&1
  local out count
  out=$(flowhook list --paths 2>&1)
  count=$(grep -o "dup" <<< "$out" | wc -l)
  assert_exit 2 "$count" "adding the same path twice does not duplicate"
}
# --- add --command ---

test_add_command_appears_in_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo hi" >/dev/null 2>&1
  local out
  out=$(flowhook list --commands 2>&1)
  assert_contains "$out" "echo hi" "add --command shows up in list --commands"
}

test_add_duplicate_command_fails() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo hi" >/dev/null 2>&1
  local out
  out=$(flowhook add --command "echo hi" 2>&1)
  assert_contains "$out" "Error:" "adding the same command twice errors"
}

# --- add --ignored-path ---

test_add_ignored_path_appears_in_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --ignored-path .git >/dev/null 2>&1
  local out
  out=$(flowhook list --ignored 2>&1)
  assert_contains "$out" ".git" "add --ignored-path shows up in list --ignored"
}

test_add_duplicate_ignored_path_fails() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --ignored-path .git >/dev/null 2>&1
  local out
  out=$(flowhook add --ignored-path .git 2>&1)
  assert_contains "$out" "Error:" "adding the same ignored-path twice errors"
}

# --- add --ignored-pattern ---

test_add_ignored_pattern_appears_in_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --ignored-pattern "*.o" >/dev/null 2>&1
  local out
  out=$(flowhook list --ignored 2>&1)
  assert_contains "$out" "*.o" "add --ignored-pattern shows up in list --ignored"
}

test_add_duplicate_ignored_pattern_fails() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --ignored-pattern "*.o" >/dev/null 2>&1
  local out
  out=$(flowhook add --ignored-pattern "*.o" 2>&1)
  assert_contains "$out" "Error:" "adding the same ignored-pattern twice errors"
}

# --- ignore-before-add interaction ---
# A directory ignored by name should not appear in list --paths once added,
# since add_path should respect already-registered ignores.

test_ignored_path_prevents_add_from_listing() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  mkdir -p ignoreme
  flowhook add --ignored-path ignoreme >/dev/null 2>&1
  flowhook add --path ./ignoreme >/dev/null 2>&1
  local out
  out=$(flowhook list --paths 2>&1)
  assert_not_contains "$out" "ignoreme" "ignored path should not appear in list --paths"
}

# --- add --on-success ---

test_add_on_success_appears_in_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --on-success "echo success" >/dev/null 2>&1
  local out
  out=$(flowhook list --on-success 2>&1)
  assert_contains "$out" "echo success" "add --on-success shows up in list --on-success"
}

test_add_duplicate_on_success_no_duplicate() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --on-success "echo success" >/dev/null 2>&1
  flowhook add --on-success "echo success" >/dev/null 2>&1
  local out count
  out=$(flowhook list --on-success 2>&1)
  count=$(grep -o "echo success" <<< "$out" | wc -l)
  assert_exit 1 "$count" "adding the same on-success command twice does not duplicate"
}

# --- add --on-failure ---

test_add_on_failure_appears_in_list() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --on-failure "echo failure" >/dev/null 2>&1
  local out
  out=$(flowhook list --on-failure 2>&1)
  assert_contains "$out" "echo failure" "add --on-failure shows up in list --on-failure"
}

test_add_duplicate_on_failure_no_duplicate() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --on-failure "echo failure" >/dev/null 2>&1
  flowhook add --on-failure "echo failure" >/dev/null 2>&1
  local out count
  out=$(flowhook list --on-failure 2>&1)
  count=$(grep -o "echo failure" <<< "$out" | wc -l)
  assert_exit 1 "$count" "adding the same on-failure command twice does not duplicate"
}

# add with no flags at all
test_add_with_no_flags() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook add 2>&1)
  assert_contains "$out" "Error:" "add with no flags errors"
}

# add --command with an empty
test_add_empty_command() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  local out
  out=$(flowhook add --command "" 2>&1)
  assert_contains "$out" "Error:" "add --command empty string errors"
}

# multiple flags in one add call — path + command + ignored-pattern together
test_add_multiple_flags_at_once() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  mkdir -p combo
  flowhook add --path ./combo --command "echo combo" --ignored-pattern "*.tmp" >/dev/null 2>&1
  local out
  out=$(flowhook list --paths 2>&1)
  assert_contains "$out" "combo" "multi-flag add: path registered"
  out=$(flowhook list --commands 2>&1)
  assert_contains "$out" "echo combo" "multi-flag add: command registered"
  out=$(flowhook list --ignored 2>&1)
  assert_contains "$out" "*.tmp" "multi-flag add: ignored-pattern registered"
}
