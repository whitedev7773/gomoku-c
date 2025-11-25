#!/bin/bash

set -e  # 하나라도 실패하면 즉시 종료

# 기본값
CLEAN_BUILD=false

# 옵션 파싱
if [ "$1" == "--clean" ]; then
    CLEAN_BUILD=true
fi

# 클린 빌드 여부
if [ "$CLEAN_BUILD" = true ]; then
    echo "[CLEAN] 기존 build 디렉터리를 삭제합니다."
    rm -rf build
fi

# 빌드 디렉터리가 없으면 생성
if [ ! -d build ]; then
    mkdir build
fi

cd build

# CMake & Make
cmake ..
make

# 실행 파일 확인
if [ ! -f "./gomoku-c" ]; then
    echo "Error: gomoku-c 실행 파일을 찾을 수 없습니다."
    exit 1
fi

echo ""
echo "========================="
echo "   Gomoku.c 실행 시작"
echo "========================="
echo ""

./gomoku-c
