# 🎮 WSL2에서 Xbox 컨트롤러(게임패드) 연결 가이드

이 문서는 윈도우(Host)에 연결된 Xbox 컨트롤러를 WSL2(Linux)로 연결하고, `xboxdrv`를 이용해 인식시키는 전체 절차를 설명합니다.

---

## 1단계: 윈도우에서 USB 연결 넘겨주기 (PowerShell)

**준비물:** `usbipd-win`이 설치되어 있어야 합니다. (설치: `winget install dorssel.usbipd-win`)

1. **관리자 권한**으로 PowerShell을 실행합니다.
2. 현재 연결된 USB 목록을 확인하여 컨트롤러의 **BUSID**를 찾습니다.

   ```powershell
   usbipd list
   ```

   _(예: `1-2` 또는 `3-4` 같은 숫자를 찾으세요)_

3. 찾은 BUSID를 이용해 WSL로 장치를 연결합니다. **(핵심 명령어 1)**
   ```powershell
   usbipd attach --wsl --busid <BUSID>
   ```
   - `<BUSID>` 자리에 위에서 찾은 숫자를 넣으세요. (예: `usbipd attach --wsl --busid 1-2`)

---

## 2단계: WSL 리눅스 설정 (WSL Terminal)

이제 WSL 터미널을 열고 리눅스 내부 설정을 진행합니다.

1.  **uinput 모듈 로드하기 (핵심 명령어 2)**
    `xboxdrv`가 가상 조이스틱을 생성할 수 있도록 커널 모듈을 불러옵니다.

    ```bash
    sudo modprobe uinput
    ```

    _(참고: 에러가 발생하면 WSL 커널이 uinput을 지원하지 않는 상태입니다. 커스텀 커널 설정이 필요할 수 있습니다.)_

2.  **장치 ID 확인하기**
    연결된 컨트롤러의 USB ID(Vendor:Product)를 확인합니다.

    ```bash
    lsusb
    ```

    _(보통 Xbox 360 컨트롤러는 `045e:028e`입니다.)_

---

## 3단계: 드라이버 실행 (WSL Terminal)

마지막으로 드라이버를 실행하여 입력을 활성화합니다.

1.  **xboxdrv 실행 (핵심 명령어 3)**
    아래 명령어를 입력하여 드라이버를 가동합니다.

    ```bash
    sudo xboxdrv --device-by-id 045e:028e --type xbox360 --mimic-xpad --silent --detach-kernel-driver
    ```

    **명령어 설명:**

    - `--device-by-id`: 특정 USB 장치(045e:028e)를 지정합니다. 본인의 `lsusb` 결과에 따라 ID를 변경하세요.
    - `--type xbox360`: Xbox 360 컨트롤러 레이아웃을 사용합니다.
    - `--mimic-xpad`: 표준 리눅스 패드(xpad)인 것처럼 동작하게 하여 호환성을 높입니다.
    - `--detach-kernel-driver`: 충돌 방지를 위해 기존 커널 드라이버가 잡고 있다면 강제로 떼어냅니다.
    - `--silent`: 불필요한 로그 출력을 줄입니다.

---

### ✅ 연결 테스트

새 터미널 창을 열어서 아래 명령어로 버튼 입력이 잘 되는지 확인하세요.

```bash
jstest /dev/input/js0
```

---

## ⚠️ 중요: `xboxdrv` 오류 해결법 (uinput 문제)

```bash
# 대부분의 최신 리눅스는 `xboxdrv` 없이도 `/dev/input/js0` 경로로 컨트롤러 입력을 받을 수 있습니다.
# 프로젝트(`gomoku-c`) 코드에서 `xboxdrv`에 의존하지 않고 리눅스 표준 조이스틱 입력을 받도록 하는 것이 훨씬 쉽습니다.

# 연결 테스트 명령어:
sudo apt install joystick
jstest /dev/input/js0
```

```

```
