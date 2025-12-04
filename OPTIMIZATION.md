# UI 렌더링 최적화

## 개요

Phase 13의 추가 개선 작업으로 UI 렌더링 최적화를 수행했습니다. 전체 화면을 매 프레임 다시 그리는 대신, **변경된 부분만 선택적으로 업데이트**하도록 개선했습니다.

## 최적화 원칙

1. **상태 추적**: 이전 상태와 현재 상태를 비교
2. **선택적 렌더링**: 변경된 부분만 다시 그리기
3. **테두리 최소화**: Box/테두리는 초기화 시 한 번만 렌더링
4. **wclear 제거**: 전체 화면 지우기 대신 부분 업데이트

## 주요 변경사항

### 1. board_ui 최적화

#### 구조 변경
```c
typedef struct {
    int cursor_row;
    int cursor_col;
    int prev_cursor_row;  // 이전 커서 위치 추적
    int prev_cursor_col;
} BoardCursor;
```

#### 새로운 API
- `board_ui_init()` - 테두리 포함 초기 렌더링 (한 번만 호출)
- `board_ui_update()` - 변경된 부분만 업데이트 (최적화)
- `board_ui_redraw_cell()` - 특정 셀만 다시 그리기
- `board_ui_render()` - 전체 렌더링 (호환성 유지)

#### 최적화 효과
- ✅ 테두리는 초기화 시 한 번만 렌더링
- ✅ 커서 이동 시 이전 위치와 현재 위치만 업데이트
- ✅ 돌이 놓여질 때 해당 셀만 업데이트
- ✅ 전체 보드 wclear() 제거

#### 사용 예시
```c
// 초기화 (게임 시작 시 한 번)
board_ui_init(ui_mgr.board_win);

// 게임 루프에서 (매 프레임)
board_ui_update(ui_mgr.board_win, &board, &cursor);  // 최적화
// 또는
board_ui_render(ui_mgr.board_win, &board, &cursor);  // 전체 렌더링
```

### 2. game_info_ui 최적화

#### 구조 변경
```c
typedef struct {
    time_t game_start_time;
    // 이전 상태 추적
    int prev_last_row;
    int prev_last_col;
    Stone prev_current_player;
    int prev_elapsed_seconds;
} GameInfoUI;
```

#### 새로운 API
- `game_info_ui_init_bottom()` - 테두리 초기화 (한 번만)
- `game_info_ui_update_bottom()` - 변경된 정보만 업데이트 (최적화)
- `game_info_ui_render_bottom()` - 전체 렌더링 (호환성 유지)

#### 최적화 로직
1. **Last Move 체크**: 이전 수와 비교하여 변경 시에만 업데이트
2. **현재 턴 체크**: 턴이 바뀔 때만 업데이트
3. **타이머**: 매 초 업데이트 필요 (항상 업데이트)
4. **경과 시간**: 초 단위로 변경될 때만 업데이트

#### 최적화 효과
- ✅ 테두리는 초기화 시 한 번만 렌더링
- ✅ 변경된 정보만 선택적으로 업데이트
- ✅ 불필요한 wrefresh() 호출 최소화
- ✅ wclear() 제거

#### 사용 예시
```c
// 초기화 (게임 시작 시 한 번)
game_info_ui_init_bottom(ui_mgr.bottom_win);

// 게임 루프에서 (매 프레임)
game_info_ui_update_bottom(ui_mgr.bottom_win, &board, &turn_mgr, &info_ui);  // 최적화
// 또는
game_info_ui_render_bottom(ui_mgr.bottom_win, &board, &turn_mgr, &info_ui);  // 전체 렌더링
```

## 성능 개선

### Before (최적화 전)
- 매 프레임마다 전체 화면 wclear()
- 모든 UI 요소 전체 재렌더링
- 테두리도 매번 다시 그리기
- 불필요한 wrefresh() 호출 다수

### After (최적화 후)
- wclear()는 초기화 시에만
- 변경된 셀/정보만 선택적 업데이트
- 테두리는 한 번만 렌더링
- 필요할 때만 wrefresh() 호출

### 예상 효과
- **렌더링 호출 감소**: 약 80-90% 감소
- **화면 깜빡임 감소**: 전체 clear 제거로 부드러운 화면
- **CPU 사용률 감소**: 불필요한 렌더링 제거
- **터미널 반응성 향상**: 빠른 UI 업데이트

## 호환성

기존 코드와의 호환성을 위해 `_render()` 함수는 그대로 유지됩니다:
- `board_ui_render()` - 전체 렌더링
- `game_info_ui_render_bottom()` - 전체 렌더링

최적화를 적용하려면 새로운 API를 사용하면 됩니다:
- `board_ui_update()` - 최적화된 업데이트
- `game_info_ui_update_bottom()` - 최적화된 업데이트

## 향후 개선 가능성

1. **chat_ui 최적화**: 새 메시지만 추가하도록 개선
2. **ui_manager 최적화**: 전체 clear 대신 선택적 clear
3. **더블 버퍼링**: ncurses 더블 버퍼링 활용
4. **부분 리프레시**: wnoutrefresh + doupdate 패턴

## 주의사항

- 최적화된 함수 사용 시 초기화 함수를 반드시 호출해야 합니다
  - `board_ui_init()`
  - `game_info_ui_init_bottom()`
- 상태 추적 필드가 추가되었으므로 초기화 시 올바르게 설정해야 합니다
- 호환성을 위해 기존 `_render()` 함수도 계속 사용 가능합니다

## 파일 변경 내역

- `src/ui/board_ui.h` - BoardCursor 구조 확장, 새 API 추가
- `src/ui/board_ui.c` - 최적화 함수 구현
- `src/ui/game_info_ui.h` - GameInfoUI 구조 확장, 새 API 추가
- `src/ui/game_info_ui.c` - 최적화 함수 구현

## 테스트

모든 기존 테스트 통과:
```bash
$ ./run_tests.sh
======================================
  Test Summary
======================================
Test Suites Passed: 3
Test Suites Failed: 0
======================================
🎉 ALL TEST SUITES PASSED!
```

빌드 성공:
```bash
$ make -j4
[100%] Built target gomoku-c
```
