# 🎮 WSL2에서 Xbox 컨트롤러(게임패드) 연결 가이드

WSL(Windows Subsystem for Linux)은 기본적으로 윈도우에 연결된 USB 장치를 바로 인식하지 못합니다. 마치 윈도우와 리눅스 사이에 벽이 있는 것과 같습니다.

이 가이드는 그 벽을 뚫고 **윈도우에 꽂힌 컨트롤러를 WSL 리눅스로 넘겨주는 방법**(`usbipd` 사용)을 설명합니다.

---

## 1. 준비물 (윈도우에서 할 일)

가장 먼저 윈도우와 WSL 리눅스 사이에서 USB 신호를 중계해 줄 프로그램인 **`usbipd-win`**을 설치해야 합니다.

1.  **PowerShell(관리자 권한)**을 실행합니다. (시작 메뉴 → PowerShell 검색 → 우클릭 '관리자 권한으로 실행')
2.  아래 명령어를 입력하여 설치합니다.
    ```powershell
    winget install --interactive --exact dorssel.usbipd-win
    ```
3.  설치가 완료되면 **윈도우를 재부팅**하거나, 현재 열려 있는 터미널 창을 모두 닫았다가 다시 엽니다.

---

## 2. 리눅스 설정 (WSL에서 할 일)

이제 WSL(Kali 또는 Ubuntu)을 켜고, USB 장치를 인식할 수 있는 도구를 설치해야 합니다.

1.  WSL 터미널을 엽니다.
2.  아래 명령어를 입력해 USB 관련 도구를 설치합니다.
    ```bash
    sudo apt update
    sudo apt install linux-tools-generic hwdata
    sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*/usbip 20
    ```

---

## 3. 컨트롤러 연결하기 (핵심 단계)

이제 실제로 컨트롤러를 윈도우에서 리눅스로 "던져주는" 작업을 합니다. **이 과정은 매번 WSL을 새로 켤 때마다(또는 USB를 뽑았다 낄 때마다) 필요할 수 있습니다.**

### 1단계: 버스 ID(BUSID) 확인

1.  컨트롤러를 PC에 USB로 연결하세요.
2.  **윈도우 PowerShell(관리자 권한)**을 엽니다.
3.  아래 명령어로 연결된 USB 목록을 봅니다.
    ```powershell
    usbipd list
    ```
4.  목록에서 `Xbox Controller` (또는 비슷한 이름)를 찾고, 그 앞의 **BUSID**를 기억하세요. (예: `1-2` 또는 `2-4`)

### 2단계: 리눅스로 연결 (Attach)

1.  PowerShell에서 아래 명령어를 입력합니다. (`<BUSID>` 자리에 아까 본 숫자를 넣으세요)
    ```powershell
    usbipd bind --busid <BUSID>
    usbipd attach --wsl --busid <BUSID>
    ```
    - _(참고: `bind`는 처음 한 번만 하면 되고, 이후엔 `attach`만 해도 됩니다.)_

---

## 4. 연결 확인하기

다시 **WSL 리눅스 터미널**로 돌아와서 컨트롤러가 잘 넘어왔는지 확인합니다.

```bash
lsusb
```

결과 화면에 `Microsoft Corp. Controller` 등이 보인다면 하드웨어 연결은 성공한 것입니다\! 🎉

---

## ⚠️ 중요: `xboxdrv` 오류 해결법 (uinput 문제)

위 과정을 다 거쳤는데도 `xboxdrv` 실행 시 아래와 같은 에러가 뜬다면:

> `Error: No suitable uinput device found...`

**원인:**
WSL2의 기본 커널(리눅스의 심장)에는 `uinput`이라는 기능이 빠져 있습니다. `xboxdrv`는 이 기능이 꼭 필요합니다.

**해결 방법 (초보자용 추천):**

1.  **`xboxdrv`를 쓰지 않고 직접 읽기:**
    대부분의 최신 리눅스는 `xboxdrv` 없이도 `/dev/input/js0` 경로로 컨트롤러 입력을 받을 수 있습니다. 프로젝트(`gomoku-c`) 코드에서 `xboxdrv`에 의존하지 않고 리눅스 표준 조이스틱 입력을 받도록 하는 것이 훨씬 쉽습니다.

    연결 테스트 명령어:

    ```bash
    sudo apt install joystick
    jstest /dev/input/js0
    ```

    (이걸로 버튼을 눌렀을 때 숫자가 바뀌면, 굳이 `xboxdrv`를 안 써도 됩니다.)

2.  **커스텀 커널 빌드 (고급):**
    반드시 `xboxdrv`를 써야 한다면, WSL2 커널을 직접 빌드해서 `uinput` 모듈을 켜야 합니다. 이 과정은 매우 복잡하므로 초보자에게는 권장하지 않습니다.

---

### 요약

1.  윈도우에 `usbipd` 설치.
2.  PowerShell(관리자)에서 `usbipd attach`로 리눅스에 USB 넘겨주기.
3.  리눅스에서 `lsusb`로 확인.
4.  `xboxdrv` 대신 `jstest`로 입력 확인 추천.
