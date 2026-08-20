#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

run_case() {
  local label="$1"
  local file="$2"
  local expected="$3"
  local actual
  actual=$(./bay run "$file")
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

run_case "variables milestone" "tests/variables.bay" $'hello, world\n15'

cat > /tmp/bay_var_x.bay <<'EOF'
variable x equals 10.
say the x end.
EOF
run_case "single variable lookup" "/tmp/bay_var_x.bay" $'10'

cat > /tmp/bay_var_message.bay <<'EOF'
variable message equals "hello".
say the message end.
EOF
run_case "string variable lookup" "/tmp/bay_var_message.bay" $'hello'

cat > /tmp/bay_hello_compile.bay <<'EOF'
say "hello".
EOF

./bay compile /tmp/bay_hello_compile.bay >/tmp/bay_compile.out 2>/tmp/bay_compile.err || true
if [[ ! -x /tmp/bay_hello_compile ]]; then
  echo "FAIL: host native compile output missing"
  exit 1
fi
actual=$(/tmp/bay_hello_compile)
if [[ "$actual" != "hello" ]]; then
  echo "FAIL: compiled hello output"
  echo "Expected: hello"
  echo "Actual: $actual"
  exit 1
fi

./bay compile /tmp/bay_hello_compile.bay --target linux-x64 >/tmp/bay_compile_target.out 2>/tmp/bay_compile_target.err || true
if [[ ! -x /tmp/bay_hello_compile ]]; then
  echo "FAIL: linux-x64 compile output missing"
  exit 1
fi

if ! ./bay compile --help 2>&1 | grep -q "windows-x64"; then
  echo "FAIL: help output missing windows-x64 target"
  exit 1
fi

if ./bay compile /tmp/bay_hello_compile.bay --target windows-foo >/tmp/bay_invalid.out 2>/tmp/bay_invalid.err; then
  echo "FAIL: invalid target should be rejected"
  exit 1
fi
