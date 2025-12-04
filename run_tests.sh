#!/bin/bash

# Gomoku-C Test Runner Script
# Phase 13: 테스트 및 최적화

echo "======================================"
echo "  Gomoku-C Test Suite"
echo "======================================"
echo ""

# 빌드 디렉토리로 이동
cd build

# 테스트 결과 추적
total_passed=0
total_failed=0

# Board 단위 테스트
echo "▶ Running Board Logic Tests..."
./test_board
if [ $? -eq 0 ]; then
    echo "✓ Board tests PASSED"
    ((total_passed++))
else
    echo "✗ Board tests FAILED"
    ((total_failed++))
fi
echo ""

# Rules 단위 테스트
echo "▶ Running Game Rules Tests..."
./test_rules
if [ $? -eq 0 ]; then
    echo "✓ Rules tests PASSED"
    ((total_passed++))
else
    echo "✗ Rules tests FAILED"
    ((total_failed++))
fi
echo ""

# AI 테스트
echo "▶ Running AI Engine Tests..."
./test_ai > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ AI tests PASSED"
    ((total_passed++))
else
    echo "✗ AI tests FAILED"
    ((total_failed++))
fi
echo ""

# 최종 결과
echo "======================================"
echo "  Test Summary"
echo "======================================"
echo "Test Suites Passed: $total_passed"
echo "Test Suites Failed: $total_failed"
echo "======================================"

if [ $total_failed -eq 0 ]; then
    echo "🎉 ALL TEST SUITES PASSED!"
    exit 0
else
    echo "❌ SOME TEST SUITES FAILED"
    exit 1
fi
