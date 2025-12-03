# UTF-8 특수문자 표시 문제 해결

## 문제
터미널에서 게임 실행 시 한글, 박스 문자 등 특수문자가 깨지는 현상

## 해결 방법

### ✅ 자동 해결 (권장)

실행 스크립트 사용:
```bash
# 메인 게임 실행
./run.sh

# 테스트 실행
./run_test.sh menu      # 메뉴 테스트
./run_test.sh phase3    # Phase 3 UI 테스트
./run_test.sh phase2    # Phase 2 게임 로직 테스트
```

### 🔧 수동 해결

#### 1. ncursesw 설치 확인
```bash
# Debian/Ubuntu
sudo apt-get install libncursesw5-dev

# 이미 설치되어 있으면 스킵
```

#### 2. 환경 변수 설정 후 실행
```bash
cd build
LANG=ko_KR.UTF-8 LC_ALL=ko_KR.UTF-8 ./gomoku-c
```

또는 쉘 설정에 추가 (~/.bashrc):
```bash
export LANG=ko_KR.UTF-8
export LC_ALL=ko_KR.UTF-8
```

### 📋 적용된 변경사항

1. **CMakeLists.txt**: ncurses 대신 ncursesw 사용
2. **run.sh**: UTF-8 로케일 자동 설정 후 실행
3. **run_test.sh**: 테스트 실행 시 UTF-8 로케일 자동 설정

### 🧪 확인 방법

```bash
# 현재 로케일 확인
locale

# ncursesw 링크 확인
ldd build/gomoku-c | grep ncurses

# 실행 테스트
./run.sh
```

정상적으로 표시되어야 할 문자들:
- 박스 문자: ┏ ┓ ┗ ┛ ━ ┃ ┣ ┫ ┳ ┻
- 화살표: ▶ ↑ ↓ ↵
- 오목돌: ● ○
- 기타: █ ░ ╹
