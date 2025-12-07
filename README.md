# Gomoku-C

<div align="center">

![Gomoku-C Logo](docs/images/logo.png)

**터미널에서 즐기는 본격 오목 게임**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20WSL2-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/language-C-orange.svg)]()

[설치하기](#설치-방법) | [게임 시작하기](#게임-시작하기) | [조작법](#조작법) | [기능 소개](#주요-기능)

</div>

---

## 소개

**Gomoku-C**는 Linux/WSL2 환경에서 실행되는 터미널 기반 오목 게임입니다.

혼자서 AI와 대결하거나, 친구와 네트워크를 통해 대전할 수 있습니다. ncurses 라이브러리를 활용하여 터미널에서도 깔끔하고 직관적인 UI를 제공합니다.

![메인 메뉴 스크린샷](docs/images/main_menu.png)

### 왜 Gomoku-C인가요?

- **설치가 간단합니다** - 몇 줄의 명령어로 바로 플레이 가능
- **네트워크 대전 지원** - 친구와 LAN 또는 인터넷으로 대전
- **AI 대전 지원** - 혼자서도 쉬움/어려움 난이도의 AI와 대결
- **관전 모드** - 다른 사람의 게임을 실시간으로 관전
- **리플레이 기능** - 지난 게임을 다시 보며 복기
- **다양한 테마** - 취향에 맞는 색상 테마 선택
- **게임패드 지원** - 키보드 외에 게임패드로도 조작 가능

---

## 스크린샷

<div align="center">

|           싱글 플레이 with AI            |     멀티플레이 with Chat      |
| :--------------------------------------: | :---------------------------: |
| ![게임 플레이](docs/images/gameplay.png) | ![채팅](docs/images/chat.png) |

|            테마 선택            |              리플레이               |
| :-----------------------------: | :---------------------------------: |
| ![테마](docs/images/themes.png) | ![리플레이](docs/images/replay.png) |

</div>

---

## 설치 방법

### 요구 사항

- **운영체제**: Linux (Ubuntu/Debian 권장) 또는 WSL2
- **터미널 크기**: 최소 120 x 31 (자동으로 확인됨)

### Step 1: 필수 패키지 설치

터미널을 열고 아래 명령어를 실행하세요:

**Ubuntu / Debian:**

```bash
sudo apt-get update
sudo apt-get install -y build-essential libncursesw5-dev git
```

**Fedora / RHEL:**

```bash
sudo dnf install -y gcc ncurses-devel git
```

### Step 2: CMake 설치 (3.10 이상 필요)

#### 방법 1: apt로 간편 설치 (구버전, 빠름)

```bash
sudo apt-get install -y cmake
```

> **참고**: Ubuntu 18.04 이상에서는 CMake 3.10 이상이 설치됩니다.

#### 방법 2: 최신 버전 직접 설치 (권장)

apt로 설치한 CMake 버전이 낮거나, 최신 버전이 필요한 경우:

```bash
# 기존 cmake 제거 (설치되어 있는 경우)
sudo apt-get remove -y cmake

# CMake 3.28.1 다운로드 및 설치
wget https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.sh
chmod +x cmake-3.28.1-linux-x86_64.sh
sudo ./cmake-3.28.1-linux-x86_64.sh --skip-license --prefix=/usr/local

# 설치 확인
cmake --version
```

> **참고**: `/usr/local/bin`이 PATH에 포함되어 있어야 합니다. 포함되어 있지 않다면:
>
> ```bash
> echo 'export PATH=/usr/local/bin:$PATH' >> ~/.bashrc
> source ~/.bashrc
> ```

### Step 3: 소스 코드 다운로드

```bash
git clone https://github.com/whitedev7773/gomoku-c.git
cd gomoku-c
```

### Step 4: 빌드 및 실행

```bash
./build_and_run.sh
```

이게 전부입니다! 게임이 자동으로 빌드되고 실행됩니다.

> **참고**: 빌드에 문제가 있다면 클린 빌드를 시도해보세요:
>
> ```bash
> ./build_and_run.sh --clean
> ```

---

## 게임 시작하기

### 메뉴 모드 (권장)

가장 간단한 방법입니다. 메뉴에서 원하는 모드를 선택하세요:

```bash
./build/gomoku-c
```

![메뉴 선택 화면](docs/images/menu_select.png)

### 명령줄 옵션으로 바로 시작

메뉴를 거치지 않고 바로 게임을 시작할 수 있습니다:

#### 싱글플레이 (AI 대전)

```bash
# 쉬운 난이도
./build/gomoku-c --singleplay --easy

# 어려운 난이도
./build/gomoku-c --singleplay --hard
```

#### 멀티플레이 (친구와 대전)

**호스트 (게임 방 만들기):**

```bash
./build/gomoku-c --multiplay-host
```

화면에 표시되는 IP 주소를 친구에게 알려주세요.

**클라이언트 (게임 방 참가):**

```bash
./build/gomoku-c --multiplay-client -ip 192.168.0.10
```

포트를 지정하려면 (기본값: 7773):

```bash
./build/gomoku-c --multiplay-client -ip 192.168.0.10 -port 7773
```

#### 관전 모드

진행 중인 게임을 관전할 수 있습니다 (최대 3명):

```bash
./build/gomoku-c --spectator -ip 192.168.0.10
```

---

## 조작법

### 인게임 조작

|        키        | 동작                  |
| :--------------: | :-------------------- |
| `↑` `↓` `←` `→`  | 커서 이동             |
|     `Space`      | 돌 놓기               |
| `Enter` 또는 `T` | 채팅 모드 진입        |
|      `ESC`       | 채팅 모드 종료 / 메뉴 |

### 채팅 명령어

채팅창에서 특수 명령어를 입력할 수 있습니다:

|  명령어   | 설명                                          |
| :-------: | :-------------------------------------------- |
|  `/quit`  | 게임 퇴장                                     |
|  `/undo`  | 무르기 요청 (상대방 수락 필요, 10초 타임아웃) |
| `/giveup` | 기권 (즉시 패배)                              |

> **팁**: `/q`까지만 입력하면 나머지 글자가 자동완성으로 표시됩니다.

---

## 주요 기능

### 1. 싱글플레이 - AI 대전

![AI 대전](docs/images/singleplay.png)

혼자서 AI와 대결할 수 있습니다:

- **Easy 모드**: 오목을 처음 접하는 분들을 위한 쉬운 난이도
- **Hard 모드**: 미니맥스 알고리즘 기반의 강력한 AI

플레이어는 항상 흑돌(선공)로 시작합니다.

### 2. 멀티플레이 - 네트워크 대전

![멀티플레이](docs/images/multiplay.png)

|                     HOST 모드                      |                 CLIENT(JOIN) 모드                  |
| :------------------------------------------------: | :------------------------------------------------: |
| ![멀티플레이-host](docs/images/multiplay-host.png) | ![멀티플레이-join](docs/images/multiplay-join.png) |

TCP 소켓을 이용한 네트워크 대전을 지원합니다:

- **LAN 대전**: 같은 네트워크에 있는 친구와 대전
- **인터넷 대전**: 포트포워딩 설정 시 외부 네트워크에서도 대전 가능
- **도메인 지원**: IP 대신 도메인 주소로도 접속 가능

### 3. 관전 모드

![관전 모드](docs/images/spectator.png)

진행 중인 게임을 실시간으로 관전할 수 있습니다:

- 최대 3명의 관전자 지원
- 실시간 게임 상태 동기화
- 채팅 관람 가능

### 4. 리플레이

![리플레이](docs/images/replay.png)

모든 게임은 자동으로 기록됩니다:

- 파일 형식: `gomoku-YYYYMMDD-HH:MM.log`
- 메뉴에서 리플레이를 선택하여 지난 게임 재생
- `/quit` 명령어로 리플레이 종료

### 5. 실시간 채팅

![채팅](docs/images/chat_bubble.png)

게임 중 상대방과 채팅할 수 있습니다:

- 말풍선 스타일의 채팅 UI
- 최대 15자까지 입력 가능
- 시스템 메시지 표시 (입장, 착수 위치 등)

### 6. 테마 커스터마이징

![테마 선택](docs/images/theme_select.png)

다양한 색상 테마를 지원합니다:

|  테마   | 설명             |
| :-----: | :--------------- |
|  White  | 기본 흰색 테마   |
| Hacker  | 녹색 해커 스타일 |
|  Gold   | 황금빛 고급 테마 |
| Skyblue | 시원한 하늘색    |
|  Pink   | 핑크 테마        |
| Rainbow | 무지개 색상      |

### 7. 공정한 게임 규칙

Gomoku-C는 프로 오목 규칙을 적용합니다:

#### Renju Rule (렌주 룰)

흑돌의 선공 이점을 보정하기 위한 규칙:

- **삼삼 금지**: 동시에 두 개의 열린 3을 만들 수 없음
- **사사 금지**: 동시에 두 개의 4를 만들 수 없음
- **육목 금지**: 6개 이상 연속으로 놓을 수 없음

#### Swap Rule (스왑 룰)

공정한 시작을 위한 규칙:

- 첫 3수(흑1, 백1, 흑2) 후 백이 흑백 교체를 선택할 수 있음

---

## 문제 해결

### 터미널 크기가 너무 작다고 나와요

터미널 창의 크기를 **120 x 31** 이상으로 조절해주세요. 현재 크기는 화면에 표시됩니다.

### UTF-8 문자가 깨져 보여요

locale 설정을 확인하세요:

```bash
export LANG=ko_KR.UTF-8
export LC_ALL=ko_KR.UTF-8
```

### ncursesw를 찾을 수 없다고 해요

```bash
# Ubuntu/Debian
sudo apt-get install libncursesw5-dev

# Fedora/RHEL
sudo dnf install ncurses-devel
```

### 빌드가 실패해요

클린 빌드를 시도해보세요:

```bash
rm -rf build
./build_and_run.sh
```

### 네트워크 연결이 안 돼요

1. 호스트와 클라이언트가 같은 네트워크에 있는지 확인
2. 방화벽에서 포트 7773이 열려 있는지 확인
3. 외부 네트워크라면 포트포워딩 설정 필요

---

## 프로젝트 구조

```
gomoku-c/
├── src/
│   ├── main.c                 # 프로그램 진입점
│   ├── game/                  # 게임 로직
│   │   ├── core/              # 보드, 규칙, 턴 관리
│   │   ├── ai/                # AI 엔진
│   │   ├── mode/              # 싱글/멀티/관전 모드
│   │   └── feature/           # 로거 등 부가 기능
│   ├── ui/                    # 사용자 인터페이스
│   │   ├── core/              # UI 핵심, 테마, 입력 처리
│   │   ├── menu/              # 메뉴 화면
│   │   └── game/              # 게임 화면 (보드, 채팅, 정보창)
│   ├── network/               # 네트워크 통신
│   │   ├── core/              # TCP 소켓, 클라이언트/서버
│   │   └── messages/          # 프로토콜 메시지
│   └── utils/                 # 유틸리티 함수
├── docs/
│   └── images/                # README 이미지
├── CMakeLists.txt             # CMake 빌드 설정
├── build_and_run.sh           # 빌드 및 실행 스크립트
└── README.md                  # 이 문서
```

---

## 기여하기

버그 리포트, 기능 제안, Pull Request를 환영합니다!

1. 이 저장소를 Fork 합니다
2. 새 브랜치를 만듭니다 (`git checkout -b feature/amazing-feature`)
3. 변경사항을 커밋합니다 (`git commit -m 'feat: Add amazing feature'`)
4. 브랜치에 Push 합니다 (`git push origin feature/amazing-feature`)
5. Pull Request를 생성합니다

---

## 라이선스

이 프로젝트는 MIT 라이선스로 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

---

<div align="center">

**Gomoku-C** - 터미널에서 즐기는 오목의 재미

Made with C and ncurses

</div>
