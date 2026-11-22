#!/bin/bash

COMPILER="./build/cminorcc"
PASSED=0
FAILED=0

echo "================================"
echo " C Minor Compiler Test Suite"
echo "================================"

run_positive() {
    FILE="$1"
    echo -n "Positive: $FILE ... "

    if $COMPILER --check "$FILE" > /dev/null 2>&1; then
        echo "PASS"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL"
        FAILED=$((FAILED + 1))
    fi
}

run_negative() {
    FILE="$1"
    echo -n "Negative: $FILE ... "

    if $COMPILER --check "$FILE" > /dev/null 2>&1; then
        echo "FAIL"
        FAILED=$((FAILED + 1))
    else
        echo "PASS"
        PASSED=$((PASSED + 1))
    fi
}

run_positive tests/positive/basic.cm

run_negative tests/negative/undeclared.cm
run_negative tests/negative/duplicate.cm
run_negative tests/negative/break.cm

echo "================================"
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "================================"

if [ "$FAILED" -eq 0 ]; then
    exit 0
else
    exit 1
fi
