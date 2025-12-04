#!/bin/bash

# Gomoku-C Memory Leak Check Script
# Uses valgrind to check for memory leaks

echo "======================================"
echo "  Gomoku-C Memory Leak Check"
echo "======================================"
echo ""

# valgrind 설치 확인
if ! command -v valgrind &> /dev/null; then
    echo "⚠  valgrind is not installed."
    echo "   Install with: sudo apt-get install valgrind"
    echo ""
    echo "   Skipping memory leak check..."
    exit 0
fi

cd build

echo "▶ Checking Board Tests for Memory Leaks..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
         --error-exitcode=1 --quiet \
         ./test_board > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ No memory leaks detected in Board tests"
else
    echo "✗ Memory leaks detected in Board tests"
    echo "   Run: valgrind --leak-check=full ./build/test_board"
fi
echo ""

echo "▶ Checking Rules Tests for Memory Leaks..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
         --error-exitcode=1 --quiet \
         ./test_rules > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ No memory leaks detected in Rules tests"
else
    echo "✗ Memory leaks detected in Rules tests"
    echo "   Run: valgrind --leak-check=full ./build/test_rules"
fi
echo ""

echo "======================================"
echo "  Memory Check Complete"
echo "======================================"
echo ""
echo "Note: Full game (gomoku-c) uses ncurses which may"
echo "      report reachable blocks. This is normal."
echo "======================================"
