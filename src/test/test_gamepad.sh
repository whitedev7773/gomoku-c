#!/bin/bash

echo "==================================="
echo "  Xbox Controller Detection Test"
echo "==================================="
echo ""

# 빌드 디렉토리 확인
if [ ! -f "build/test_gamepad" ]; then
    echo "Error: test_gamepad executable not found"
    echo "Please run: cd build && cmake .. && cmake --build ."
    exit 1
fi

# /dev/input 디바이스 확인
echo "Checking /dev/input devices..."
echo ""

if [ -d "/dev/input" ]; then
    echo "Available joystick devices:"
    ls -la /dev/input/js* 2>/dev/null || echo "  (no js* devices found)"
    echo ""

    echo "Available event devices:"
    ls -la /dev/input/event* 2>/dev/null | head -5
    echo ""
else
    echo "Warning: /dev/input directory not found"
    echo ""
fi

# 권한 확인
if [ -e "/dev/input/js0" ]; then
    if [ ! -r "/dev/input/js0" ]; then
        echo "Warning: No read permission for /dev/input/js0"
        echo "You may need to run: sudo chmod a+r /dev/input/js*"
        echo ""
    fi
fi

# 테스트 실행
echo "Starting test..."
echo ""

cd build
./test_gamepad

echo ""
echo "Test completed."
