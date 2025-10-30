# Software Requirements Specification (SRS)

## for

**Gomoku-C: Terminal-based Omok Game using curses**

**Prepared by**
Team “Gomoku.C”
Giwon Jang et al.

**2025.10.30**

---

### Revision History

| Name       | Date       | Reason for Change | Version |
| ---------- | ---------- | ----------------- | ------- |
| Giwon Jang | 2025-10-30 | Initial Draft     | 1.0     |

---

## 1. Introduction

### 1.1 Purpose & System Scope

**Gomoku-C**는 **C 언어와 ncurses 라이브러리**를 사용하여 제작된 **터미널 기반 오목 게임(TUI)** 시스템이다.
본 프로젝트의 목적은 사용자가 GUI 없이도 직관적이고 미려한 텍스트 인터페이스를 통해
오목을 플레이할 수 있는 환경을 제공하고,
**싱글플레이(AI)** 및 **LAN 멀티플레이**를 지원함으로써
사용자 간 실시간 대전을 가능하게 하는 것이다.

현재 CLI 기반 보드게임들은 단순한 출력 위주로 설계되어 인터랙션과 사용자 경험이 제한적이다.
Gomoku-C는 이러한 한계를 극복하여 다음과 같은 가치를 제공한다.

1. **직관적 텍스트 인터페이스**: curses 기반의 실시간 렌더링과 UI 구성
2. **다양한 플레이 모드**: AI 난이도 선택, LAN 대전
3. **실시간 상호작용**: 채팅, 무르기, 기권 등 커뮤니케이션 지원
4. **턴 기반 제약시간**: 플레이 템포 유지 및 긴장감 조성

---

### 1.2 System Scope

#### 1.2.1 포함 범위 (In Scope)

**[핵심 기능]**

* 싱글플레이: Easy/Hard AI 대전 모드
* LAN 멀티플레이: IP-PORT 기반 실시간 1:1 연결
* 턴 제한시간 표시 및 진행바(Progress Bar)
* 무르기 요청/수락/거절
* 기권 요청
* 방향키 이동 및 착수 (Enter)
* 상대방의 마지막 수 표시
* 채팅 송수신 (실시간)
* 닉네임 설정 및 표시

**[기술 스택]**

* **언어:** C (C11)
* **UI:** ncurses
* **Network:** TCP/IP (socket)
* **빌드:** GNU Make
* **환경:** Linux/macOS 터미널

#### 1.2.2 제외 범위 (Out of Scope)

* GUI 그래픽 렌더링 (OpenGL, SDL 등)
* 온라인 서버 매칭 시스템
* AI 학습 알고리즘 확장 (딥러닝 등)
* 리플레이 저장/재생 기능 (향후 확장 고려)

#### 1.2.3 기대 효과

* **터미널 환경에서도 완전한 오목 경험 제공**
* **네트워크 프로그래밍 및 TUI 설계 학습 사례 확보**
* **AI 알고리즘의 적용 및 난이도 조절 예제 제공**
* **클래식 보드게임의 CLI UX 개선 모델 제시**

---

### 1.3 System Overview

Gomoku-C는 curses 기반 **텍스트 사용자 인터페이스(TUI)**를 제공하며,
**AI 모듈**, **네트워크 통신 모듈**, **UI 모듈**, **게임 로직 모듈**로 구성된다.

#### 1.3.1 사용자 인터페이스 계층 (Presentation Layer)

* curses 기반 UI 렌더링
* 메뉴, 보드, 채팅창, 모달(Undo/Resign 확인창)
* 키보드 중심 조작(화살표, Enter, 단축키)

#### 1.3.2 애플리케이션 계층 (Application Layer)

* 게임 진행 및 턴 관리 로직
* AI 판단 알고리즘 (Easy, Hard)
* 채팅 메시지 관리
* 요청/응답 이벤트 처리 (Undo, Resign 등)

#### 1.3.3 네트워크 계층 (Network Layer)

* 서버/클라이언트 소켓 통신
* 데이터 동기화 및 Ping 계산
* 메시지 직렬화 및 패킷 전송

#### 1.3.4 데이터 계층 (Data Layer)

* 보드 상태 데이터 구조
* 플레이어 정보 (닉네임, 점수, 상태)
* 게임 로그 및 세션 정보

---

## 2. Overall Description

### 2.1 Product Perspective

#### 2.1.1 시스템 환경 및 맥락

* **로컬/네트워크 겸용 CLI 프로그램**
* **독립 실행형 터미널 앱**으로 작동하며, curses UI로 모든 시각적 요소를 표시
* Linux/macOS의 표준 콘솔 환경에서 동작
* LAN 모드는 동일 네트워크 내에서 연결 가능

#### 2.1.2 기존 시스템과의 차별점

| 항목    | 기존 CLI 오목 게임 | Gomoku-C             |
| ----- | ------------ | -------------------- |
| UI 표현 | 단순 ASCII 출력  | curses 기반 실시간 렌더링    |
| 멀티플레이 | 미지원          | LAN 1:1 실시간          |
| AI    | 랜덤           | 난이도 선택 (Easy/Hard)   |
| 인터랙션  | 없음           | 무르기, 기권, 채팅, 닉네임 설정  |
| 시간 관리 | 없음           | 턴 타이머 및 Progress Bar |

#### 2.1.3 새로운 시스템이 필요한 이유

* 단조로운 CLI 게임에서 벗어나 직관적이고 반응형 인터페이스를 제공
* 학생 개발자들이 **ncurses, socket, AI 로직**을 통합적으로 학습할 수 있는 예시
* 멀티플레이 및 네트워크 기반 동기화 기능을 학습 목적에 맞게 설계

#### 2.1.4 향후 확장 가능성

* 리플레이 저장 및 재생 기능
* 온라인 서버 매칭
* AI 강화 학습 (Medium, Expert 레벨 추가)
* 다양한 보드 규칙(33, 44 금수 등) 적용

---

### 2.2 Product Features

**[Core Features]**

1. **Single Player Mode (AI)**

   * 난이도 선택 (Easy/Hard)
   * AI 판단 로직 기반 수 계산
   * 승패 자동 판정

2. **LAN Multiplayer Mode**

   * IP 입력 후 연결
   * Ping 표시 및 동기화
   * 시스템 메시지 기반 실시간 통신

3. **Turn Timer**

   * 각 턴당 10초 제한
   * Progress Bar 및 숫자 표시
   * 시간 초과 시 자동 턴 종료

4. **Undo / Resign System**

   * 무르기 요청 및 수락/거절
   * 기권 요청 및 자동 판정

5. **Cursor Control & Board Rendering**

   * 방향키 이동, Enter 착수
   * 이전 턴 하이라이트 표시

6. **Chat System & Nickname Display**

   * 실시간 송수신
   * “YOU / P2” 구분 및 타임스탬프 표시
   * 닉네임 입력 및 상단 출력

---

### 2.3 User Characteristics

| 사용자 그룹 | 역할                    | 기술 수준 | 주요 목표         | 이용 빈도 |
| ------ | --------------------- | ----- | ------------- | ----- |
| 일반 사용자 | Player 1 / Player 2   | 초급~중급 | 오목 대전, 채팅     | 수시    |
| 개발자    | Tester / Maintainer   | 중급~고급 | 기능 검증, AI 튜닝  | 주기적   |
| 학습자    | System Design Learner | 초급    | TUI/Socket 학습 | 교육용   |

---

## 3. System Features

---

### 3.1 System Feature 1: Single Player (AI Mode)

#### 3.1.1 Functional Requirements

* F1-001: 사용자는 Easy/Hard 난이도 선택 가능
* F1-002: AI는 난이도에 따라 다른 알고리즘으로 수를 선택
* F1-003: 플레이어와 AI 턴이 교대로 진행
* F1-004: 승패 자동 판정 및 메시지 표시
* F1-005: 게임 종료 후 다시 시작/메뉴 복귀 가능

#### Use Case Description

| 항목                   | 내용                                       |
| -------------------- | ---------------------------------------- |
| Use Case Name        | Start Single Player Game                 |
| Related Requirements | F1-001 ~ F1-005                          |
| Primary Actors       | Player                                   |
| Secondary Actors     | AI Engine                                |
| Pre-conditions       | 메인 메뉴에서 1P 모드 선택                         |
| Successful End       | 승패 결과 화면 표시                              |
| Failed End           | AI 초기화 실패 시 메뉴 복귀                        |
| Trigger              | “1 Player Game (COM)” 선택                 |
| Main Flow            | 1) 난이도 선택 → 2) 보드 표시 → 3) 플레이 → 4) 승패 판정 |

---

### 3.2 System Feature 2: LAN Multiplayer

#### Functional Requirements

* F2-001: 사용자는 서버 또는 클라이언트 역할 선택
* F2-002: IP와 Port 입력 후 연결 시도
* F2-003: 연결 성공 시 실시간 데이터 동기화
* F2-004: Ping 표시 (1초 단위)
* F2-005: 연결 끊김 시 자동 재시도(최대 3회)

#### Use Case Description

| 항목             | 내용                                 |
| -------------- | ---------------------------------- |
| Use Case Name  | Connect to LAN Game                |
| Primary Actors | Player 1 (Host), Player 2 (Client) |
| Pre-conditions | 동일 네트워크 상 IP 접근 가능                 |
| Successful End | “Connected!” 메시지 표시                |
| Failed End     | 연결 실패 시 재시도                        |
| Trigger        | “2 Player Game (LAN)” 선택           |
| Main Flow      | IP 입력 → 연결 → 게임 시작 → 데이터 동기화       |

---

### 3.3 System Feature 3: Turn Timer

* F3-001: 각 턴 제한시간 10초
* F3-002: 진행 바와 숫자 형태로 표시
* F3-003: 시간 초과 시 자동 턴 종료
* F3-004: LAN 모드에서는 서버 시간을 기준으로 동기화

---

### 3.4 System Feature 4: Undo Request

* F4-001: 한 턴 무르기 요청 가능
* F4-002: 상대방이 수락/거절 선택
* F4-003: 수락 시 마지막 수 삭제, 턴 복원
* F4-004: 거절 시 메시지 출력
* F4-005: 요청 중에는 다른 입력 차단

**Use Case:**

1. P1이 Undo 요청 → P2 수락 여부 선택
2. 수락 시 상태 복원
3. 거절 시 “Request Declined” 표시

---

### 3.5 System Feature 5: Resign

* F5-001: 플레이어는 “RESIGN”으로 기권 가능
* F5-002: 상대방 자동 승리 처리
* F5-003: “[SYSTEM] Player resigned.” 메시지 표시

---

### 3.6 System Feature 6: Cursor Control

* F6-001: 방향키 이동, Enter로 착수
* F6-002: 돌이 존재하는 위치엔 착수 불가
* F6-003: 화면 경계 바깥 이동 불가

---

### 3.7 System Feature 7: Last Move Marker

* F7-001: 상대의 마지막 착수 위치를 강조 표시
* F7-002: curses 색상 속성으로 시각적 구분
* F7-003: 다음 턴 시작 시 이전 표시 제거

---

### 3.8 System Feature 8: Chat System

* F8-001: “T” 또는 Enter로 채팅창 활성화
* F8-002: 메시지 송수신 시 타임스탬프 표시
* F8-003: 송신/수신 구분 (“YOU / P2”)
* F8-004: 서버를 통해 실시간 전송

---

### 3.9 System Feature 9: Nickname Setting

* F9-001: 플레이 시작 전 닉네임 입력
* F9-002: 상단 헤더에 표시 (“P1: name”)
* F9-003: LAN 연결 시 상대방에게 전송

---

## Appendix A: Functional Requirements Summary

| ID     | Requirement Description | Actor  | Priority |
| ------ | ----------------------- | ------ | -------- |
| F1-001 | AI 난이도 선택               | Player | 5        |
| F2-002 | LAN 연결 및 동기화            | System | 5        |
| F3-001 | 턴 타이머 표시                | System | 4        |
| F4-001 | 무르기 요청/수락               | Player | 5        |
| F5-001 | 기권 기능                   | Player | 4        |
| F6-001 | 커서 이동 및 착수              | Player | 5        |
| F7-001 | 마지막 수 표시                | System | 3        |
| F8-001 | 채팅 송수신                  | Player | 4        |
| F9-001 | 닉네임 설정 및 표시             | Player | 3        |

---

## Appendix B: Representative Use Cases

| Use Case                 | Actors      | Trigger         | Success Condition |
| ------------------------ | ----------- | --------------- | ----------------- |
| Start Single Player Game | Player, AI  | 메뉴 선택           | 승패 결과 표시          |
| Connect LAN              | Host/Client | “2 Player Game” | 연결 및 대전 시작        |
| Request Undo             | Player 1/2  | 버튼 입력           | 복원 또는 거절 메시지      |
| Send Chat                | Player 1/2  | `Enter` 입력      | 메시지 송신 및 표시       |
| Resign Game              | Player      | 버튼 입력           | 상대방 승리 처리         |

---

## Appendix C: System Constraints

* **언어:** C (C11)
* **UI:** ncurses
* **OS:** Linux/macOS Terminal
* **Network:** TCP/IP LAN
* **Terminal Size:** 최소 100×30

---

## Appendix D: Future Enhancements

* AI Medium / Expert 난이도 추가
* 리플레이 저장 및 재생
* 온라인 매칭 서버 구축
* 플레이어 통계/기록 시스템
