# Gomoku-C

오목 게임 C 언어 구현 프로젝트

## 요구사항

### 시스템 요구사항
- Linux 환경 (Ubuntu/Debian 권장)
- GCC 컴파일러
- CMake 3.10.0 이상
- ncursesw 라이브러리 (UTF-8 지원)

### 필수 패키지 설치

Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libncursesw5-dev
```

Fedora/RHEL:
```bash
sudo dnf install gcc cmake ncurses-devel
```

## 빌드 방법

### 방법 1: 스크립트를 사용한 빌드 및 실행

가장 간단한 방법으로, 빌드와 실행을 한 번에 수행합니다:

```bash
./build_and_run.sh
```

클린 빌드가 필요한 경우:
```bash
./build_and_run.sh --clean
```

### 방법 2: 수동 빌드

직접 빌드 과정을 제어하고 싶은 경우:

```bash
# 빌드 디렉터리 생성
mkdir -p build
cd build

# CMake 설정
cmake ..

# 빌드
make

# 실행
./gomoku-c
```

### 방법 3: 빌드 후 실행 스크립트 사용

이미 빌드된 상태에서 실행만 하려는 경우:

```bash
./run.sh
```

## 테스트 빌드

프로젝트에는 여러 테스트 실행 파일이 포함되어 있습니다:

```bash
cd build

# 각종 테스트 실행
./test_phase2      # Phase 2 테스트
./test_phase3_ui   # Phase 3 UI 테스트
./test_menu        # 메뉴 테스트
./test_ai          # AI 엔진 테스트
./test_board       # 보드 단위 테스트
./test_rules       # 규칙 단위 테스트
```

또는 테스트 스크립트 사용:
```bash
./run_tests.sh
```

## 프로젝트 구조

```
gomoku-c/
├── src/
│   ├── main.c              # 메인 진입점
│   ├── game/               # 게임 로직
│   ├── ui/                 # 사용자 인터페이스
│   ├── network/            # 네트워크 기능
│   ├── utils/              # 유틸리티 함수
│   └── test/               # 테스트 파일
├── include/                # 헤더 파일
├── build/                  # 빌드 출력 (생성됨)
├── CMakeLists.txt          # CMake 설정
└── build_and_run.sh        # 빌드 및 실행 스크립트
```

## 문제 해결

### ncursesw를 찾을 수 없는 경우

빌드 시 ncursesw 관련 경고가 나타나면:

```bash
# Ubuntu/Debian
sudo apt-get install libncursesw5-dev

# Fedora/RHEL
sudo dnf install ncurses-devel
```

### UTF-8 문자가 제대로 표시되지 않는 경우

locale 설정 확인:

```bash
export LANG=ko_KR.UTF-8
export LC_ALL=ko_KR.UTF-8
```

### 빌드가 실패하는 경우

클린 빌드 시도:

```bash
rm -rf build
./build_and_run.sh
```

## 개발 환경

- 언어: C
- 빌드 시스템: CMake
- UI 라이브러리: ncurses (wide character support)
- 버전: 0.1.0

## 라이선스

이 프로젝트는 ELEC462 과제의 일부입니다.
