# 윈도우와 WSL2에서 포트 포워딩하는 방법

포트 포워딩은 WSL2 인스턴스에서 실행 중인 서비스를 윈도우 호스트 또는 네트워크의 다른 장치에서 액세스할 수 있도록 설정하는 과정입니다. 아래 단계를 따라 포트 포워딩을 설정하세요.

## 사전 준비

- WSL2가 설치된 Windows 10 이상 버전.
- Windows 관리자 권한.
- WSL2에서 실행 중인 서비스(예: 웹 서버).

## 단계

### 1. WSL2 IP 주소 확인

WSL2는 자체 가상 네트워크 어댑터와 IP 주소를 사용합니다. IP 주소를 확인하려면:

1. WSL2 터미널을 엽니다.
2. 다음 명령어를 실행합니다:
   ```bash
   ip addr | grep eth0
   ```
   `inet` 주소를 찾으세요(예: `172.20.240.1`). 이 주소가 WSL2의 IP 주소입니다.

### 2. `netsh`를 사용하여 포트 포워딩 설정

`netsh` 명령어를 사용하여 Windows 호스트에서 WSL2로 포트를 포워딩합니다.

1. Windows 명령 프롬프트 또는 PowerShell을 **관리자 권한으로 실행**합니다.
2. 다음 명령어를 실행하여 포트를 포워딩합니다:

   ```cmd
   netsh interface portproxy add v4tov4 listenaddress=0.0.0.0 listenport=<WindowsPort> connectaddress=<WSL2_IP> connectport=<WSL2Port>
   ```

   - `<WindowsPort>`: Windows 호스트에서 사용할 포트(예: `8080`).
   - `<WSL2_IP>`: WSL2 인스턴스의 IP 주소(예: `172.20.240.1`).
   - `<WSL2Port>`: WSL2에서 서비스가 실행 중인 포트(예: `8080`).

   예시:

   ```cmd
   netsh interface portproxy add v4tov4 listenaddress=0.0.0.0 listenport=8080 connectaddress=172.20.240.1 connectport=8080
   ```

3. 포트 포워딩 규칙 확인:
   ```cmd
   netsh interface portproxy show v4tov4
   ```

### 3. 방화벽 규칙 활성화

Windows 방화벽이 포워딩된 포트의 트래픽을 허용하도록 설정합니다:

1. Windows Defender 방화벽을 엽니다.
2. **고급 설정**으로 이동합니다.
3. 새 **인바운드 규칙**을 생성합니다:
   - 규칙 유형: 포트
   - 프로토콜: TCP
   - 포트: 포워딩된 포트(예: `8080`).
   - 작업: 연결 허용
   - 프로필: 적절한 프로필 선택(도메인, 개인, 공용).

### 4. 포트 포워딩 테스트

Windows 호스트 또는 네트워크의 다른 장치에서 Windows 호스트의 IP 주소와 포트를 사용하여 포워딩된 서비스에 액세스합니다:

```bash
http://<WindowsHostIP>:<WindowsPort>
```

### 5. 포트 포워딩 제거(선택 사항)

포트 포워딩 규칙을 제거하려면:

1. Windows 명령 프롬프트 또는 PowerShell을 **관리자 권한으로 실행**합니다.
2. 다음 명령어를 실행합니다:
   ```cmd
   netsh interface portproxy delete v4tov4 listenaddress=0.0.0.0 listenport=<WindowsPort>
   ```
   `<WindowsPort>`를 제거하려는 포트로 바꿉니다.

## 참고 사항

- 포트 포워딩 규칙은 재부팅 후에도 유지되지 않습니다. 재부팅 후 다시 설정해야 할 수 있습니다.
- 규칙을 지속적으로 유지하려면 시작 스크립트를 사용하는 것을 고려하세요.

---

이 단계를 따르면 WSL2에서 실행 중인 서비스를 Windows 호스트에서 성공적으로 포트 포워딩할 수 있습니다.
