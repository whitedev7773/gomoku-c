# Gomoku-C

Gomoku-C는 Linux/WSL2 환경에서 실행되는 터미널 기반의 오목 게임입니다. 이 프로젝트는 싱글플레이(플레이어 vs AI), 멀티플레이(온라인 대전), 관전자 모드, 리플레이 기능을 포함하며, ncurses를 활용한 직관적인 UI를 제공합니다.

---

## 1. 설치 및 실행하는 법

### 의존성 설치

Gomoku-C를 실행하기 위해서는 다음 의존성들이 필요합니다:

- **CMake 3.10+**
- **ncursesw** (UTF-8 지원)
- **pthread** (멀티스레딩 지원)

Ubuntu/Debian에서 의존성을 설치하려면 아래 명령어를 실행하세요:

```bash
sudo apt-get install build-essential cmake libncursesw5-dev
```

### 빌드 및 실행

1. 프로젝트를 클론합니다:

```bash
git clone https://github.com/whitedev7773/gomoku-c.git
cd gomoku-c
```

2. 빌드 스크립트를 실행합니다:

```bash
./build_and_run.sh
```

3. 클린 빌드가 필요할 경우:

```bash
./build_and_run.sh --clean
```

4. 수동으로 빌드하려면:

```bash
mkdir -p build && cd build
cmake ..
make
./gomoku-c
```

---

## 2. 사용법 및 주요 기능 설명

### 실행 모드

Gomoku-C는 다양한 실행 모드를 제공합니다. 아래는 주요 실행 명령어입니다:

#### 메뉴 모드 (기본)

```bash
./gomoku-c
```

#### 싱글플레이 모드

- 쉬움 난이도:

```bash
./gomoku-c --singleplay --easy
```

- 어려움 난이도:

```bash
./gomoku-c --singleplay --hard
```

#### 멀티플레이 모드

- 호스트:

```bash
./gomoku-c --multiplay-host
```

- 클라이언트:

```bash
./gomoku-c --multiplay-client -ip <호스트 IP>
```

- 포트 지정:

```bash
./gomoku-c --multiplay-client -ip <호스트 IP> -port <포트 번호>
```

#### 관전자 모드

- 최대 3명의 관전자가 지원됩니다:

```bash
./gomoku-c --spectator -ip <호스트 IP> -port <포트 번호>
```

### 주요 기능

1. **싱글플레이**: AI와 대결할 수 있으며, 쉬움/어려움 난이도를 선택할 수 있습니다.
2. **멀티플레이**: TCP 소켓을 이용한 온라인 대전이 가능합니다.
3. **관전자 모드**: 진행 중인 게임을 실시간으로 관전할 수 있습니다.
4. **리플레이**: 이전 게임의 기록을 재생할 수 있습니다.
5. **채팅**: 멀티플레이 중 실시간 채팅이 가능합니다.
6. **UI**: 100x31 고정 레이아웃으로 직관적인 인터페이스를 제공합니다.

---

## 3. 구현 시 어려웠던 점 설명

### 1. **UI 레이아웃 설계**

- 터미널 크기를 100x31로 고정하면서도 다양한 UI 요소(게임판, 채팅창, 정보 패널 등)를 효율적으로 배치하는 데 어려움이 있었습니다.
- `ncurses` 라이브러리를 활용하여 동적 레이아웃을 구현하고, UTF-8 문자를 지원하도록 설정하는 데 많은 시간이 소요되었습니다.

### 2. **멀티플레이 네트워크 프로토콜**

- TCP 소켓을 이용한 커스텀 바이너리 프로토콜을 설계하면서 메시지 직렬화/역직렬화, 연결 상태 관리, 동기화 문제를 해결해야 했습니다.
- 특히, 관전자 모드에서 실시간 상태 동기화를 구현하는 데 어려움이 있었습니다.

### 3. **AI 엔진 개발**

- 쉬움 난이도에서는 휴리스틱 기반 평가를 사용했지만, 어려움 난이도에서는 미니맥스 알고리즘과 알파-베타 가지치기를 구현해야 했습니다.
- 성능 최적화를 위해 탐색 깊이를 조정하고, 평가 함수의 효율성을 높이는 데 집중했습니다.

### 4. **리플레이 시스템**

- 게임 기록을 저장하고 이를 재생하는 기능을 구현하면서, 기록 포맷 설계와 UI 통합에 많은 노력이 필요했습니다.

---

Gomoku-C는 이러한 도전 과제를 극복하며 완성된 프로젝트로, 터미널 기반 게임 개발의 좋은 사례가 될 것입니다.

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
