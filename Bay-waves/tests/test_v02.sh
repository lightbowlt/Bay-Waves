#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

run_case() {
  local label="$1"
  local expected="$2"
  shift 2
  local actual
  actual=$("$@")
  if [[ "$actual" != "$expected" ]]; then
    echo "FAIL: $label"
    echo "Expected:"
    printf '%s\n' "$expected"
    echo "Actual:"
    printf '%s\n' "$actual"
    exit 1
  fi
  echo "PASS: $label"
}

run_case "input" $'Hello, Alex' bash -lc "printf 'Alex\\n' | ./bay run tests/input.bay"
run_case "booleans" $'true\nfalse' bash -lc "./bay run tests/booleans.bay"
run_case "comparisons" $'true' bash -lc "./bay run tests/comparisons.bay"
run_case "logical and" $'teen' bash -lc "./bay run tests/logical_and.bay"
run_case "logical or" $'outside range' bash -lc "./bay run tests/logical_or.bay"
run_case "logical not" $'still working' bash -lc "./bay run tests/logical_not.bay"
run_case "if/otherwise" $'adult' bash -lc "./bay run tests/if_else.bay"
run_case "repeat" $'hello\nhello\nhello\nhello\nhello' bash -lc "./bay run tests/repeat.bay"
run_case "while" $'not yet\nnot yet\nnot yet' bash -lc "./bay run tests/while.bay"
run_case "functions" $'Hello, Alex' bash -lc "./bay run tests/functions.bay"
run_case "parameters" $'15' bash -lc "./bay run tests/parameters.bay"
run_case "return values" $'25' bash -lc "./bay run tests/returns.bay"
run_case "function scope" $'local\nglobal' bash -lc "./bay run tests/scope.bay"
