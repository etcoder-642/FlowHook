#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")" || exit 1

PASS=0
FAIL=0

source ./test_helpers.sh
# test_run.sh

test_run_default_starts_and_exits_on_sigint() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo hi" >/dev/null 2>&1

  local outfile
  outfile=$(mktemp)

  flowhook run > "$outfile" 2>&1 &
  local pid=$!

  sleep 1
  kill -INT "$pid"
  wait "$pid" 2>/dev/null
  local code=$?

  local out
  out=$(cat "$outfile")
  assert_contains "$out" "Watching" "run prints watching message"
  assert_contains "$out" "Exiting safely" "run prints clean exit message on SIGINT"

  if [[ "$code" -ne 139 && "$code" -ne 134 ]]; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: run should not crash on SIGINT (got exit $code)"
  fi

  rm -f "$outfile"
}

test_run_all_starts_and_exits_on_sigint() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo hi" >/dev/null 2>&1

  local outfile
  outfile=$(mktemp)

  flowhook run --all > "$outfile" 2>&1 &
  local pid=$!

  sleep 1
  kill -INT "$pid"
  wait "$pid" 2>/dev/null

  local out
  out=$(cat "$outfile")
  assert_contains "$out" "Exiting safely" "run --all exits cleanly on SIGINT"
  rm -f "$outfile"
}

test_run_active_with_no_active_tasks_errors() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo hi" >/dev/null 2>&1
  # never set --active

  local out code
  out=$(flowhook run --active 2>&1)
  code=$?
  assert_contains "$out" "Error: no active tasks to start" "run --active with no active tasks errors"
}

test_run_active_with_active_task_starts_and_exits() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo hi" >/dev/null 2>&1
  flowhook set --active >/dev/null 2>&1

  local outfile
  outfile=$(mktemp)

  flowhook run --active > "$outfile" 2>&1 &
  local pid=$!

  sleep 1
  kill -INT "$pid"
  wait "$pid" 2>/dev/null

  local out
  out=$(cat "$outfile")
  assert_contains "$out" "Exiting safely" "run --active with an active task exits cleanly on SIGINT"
  rm -f "$outfile"
}

test_run_quiet_suppresses_command_output() {
  fresh_dir
  flowhook init >/dev/null 2>&1
  flowhook add --command "echo SHOULD_NOT_APPEAR" >/dev/null 2>&1

  local outfile
  outfile=$(mktemp)
  flowhook run --quiet > "$outfile" 2>&1 &
  local pid=$!
  sleep 1
  kill -INT "$pid"
  wait "$pid" 2>/dev/null

  local out
  out=$(cat "$outfile")
  assert_not_contains "$out" "SHOULD_NOT_APPEAR" "run --quiet suppresses build output"
  rm -f "$outfile"
}
