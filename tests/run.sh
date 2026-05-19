#!/usr/bin/env bash
# tests/run.sh — run each tests/cases/*.cyp and diff against its .expected
#
# Usage:
#   ./tests/run.sh              run all tests, exit 1 on first failure
#   UPDATE=1 ./tests/run.sh     regenerate all .expected files (USE WITH CARE)
#   ./tests/run.sh foo bar      run only tests matching "foo" or "bar"

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CYPLANG="${CYPLANG:-$REPO_ROOT/build/bin/cyplang}"
CASES_DIR="$SCRIPT_DIR/cases"

if [ ! -x "$CYPLANG" ]; then
    echo "Error: $CYPLANG not found or not executable." >&2
    echo "Hint: run 'make' from the repo root first." >&2
    exit 2
fi

if [ ! -d "$CASES_DIR" ]; then
    echo "Error: $CASES_DIR not found." >&2
    exit 2
fi

shopt -s nullglob
cases=("$CASES_DIR"/*.cyp)
if [ ${#cases[@]} -eq 0 ]; then
    echo "No test cases found in $CASES_DIR." >&2
    exit 2
fi

pass=0
fail=0
skip=0
failed_names=()

for cyp in "${cases[@]}"; do
    name=$(basename "$cyp" .cyp)

    # If args provided, only run matching cases
    if [ $# -gt 0 ]; then
        match=0
        for pat in "$@"; do
            [[ "$name" == *"$pat"* ]] && match=1 && break
        done
        [ $match -eq 0 ] && continue
    fi

    expected="${cyp%.cyp}.expected"

    # Run from cases dir so the source path printed is just the basename (portable).
    bn=$(basename "$cyp")

    if [ "${UPDATE:-0}" = "1" ]; then
        (cd "$CASES_DIR" && "$CYPLANG" "$bn") > "$expected" 2>&1 || true
        echo "  UPDATED  $name"
        continue
    fi

    if [ ! -f "$expected" ]; then
        echo "  SKIP     $name  (no .expected — run with UPDATE=1 to create)"
        skip=$((skip + 1))
        continue
    fi

    actual=$(cd "$CASES_DIR" && "$CYPLANG" "$bn" 2>&1 || true)
    expected_content=$(cat "$expected")

    if [ "$actual" = "$expected_content" ]; then
        echo "  PASS     $name"
        pass=$((pass + 1))
    else
        echo "  FAIL     $name"
        diff -u "$expected" <(printf '%s\n' "$actual") | sed 's/^/           /' | head -40
        fail=$((fail + 1))
        failed_names+=("$name")
    fi
done

echo ""
echo "Results: $pass passed, $fail failed, $skip skipped"

if [ $fail -gt 0 ]; then
    echo "Failed: ${failed_names[*]}"
    exit 1
fi
exit 0
