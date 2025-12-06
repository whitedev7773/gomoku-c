# AGENTS.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Gomoku-C** is a terminal-based Gomoku (Five-in-a-Row) game written in C for Linux/WSL2. The project features single-player (vs AI), multiplayer (LAN-based), spectator mode, and replay functionality with a sophisticated ncurses-based UI.

**Target Platform**: Linux/WSL2
**UI**: Terminal-based (ncurses with wide character support), minimum terminal size 100x31
**Network**: TCP socket communication with custom binary protocol
**Build System**: CMake 3.10+

## Building and Running

### Standard Build

```bash
# Using build script (recommended)
./build_and_run.sh

# Clean build
./build_and_run.sh --clean

# Manual build
mkdir -p build && cd build
cmake ..
make
./gomoku-c
```

### Running Tests

```bash
# Run all tests
./run_tests.sh

# Individual tests
cd build
./test_board       # Board unit tests
./test_rules       # Rules unit tests
./test_ai          # AI engine tests
./test_phase2      # Phase 2 integration tests
./test_phase3_ui   # UI tests
./test_menu        # Menu tests
```

### Command Line Options

```bash
# Menu mode (default)
./gomoku-c

# Singleplay modes
./gomoku-c --singleplay --easy
./gomoku-c --singleplay --hard

# Multiplayer modes
./gomoku-c --multiplay-host
./gomoku-c --multiplay-client -ip 192.168.0.2
./gomoku-c --multiplay-client -ip 192.168.0.2 -port 9000

# Spectator mode (max 3 spectators)
./gomoku-c --spectator -ip 192.168.0.2 -port 7773
```

**Default port**: 7773

## Architecture Overview

This codebase follows a clean, modular architecture with clear separation between game logic, UI, and networking.

### Core Module Structure

```
src/
├── main.c                    # Entry point, mode routing
├── game/
│   ├── core/                 # Core game logic
│   │   ├── board.c/h            # 19x19 board state, move validation
│   │   ├── game_logic.c/h       # Win detection, five-in-a-row checking
│   │   ├── rules.c/h            # Renju rules (3-3, 4-4, overline forbidden)
│   │   └── turn_manager.c/h     # Turn control, timer management
│   ├── ai/                   # AI engines
│   │   └── ai_engine.c/h        # Easy (heuristic) + Hard (minimax/alpha-beta)
│   ├── mode/                 # Game modes
│   │   ├── singleplay/          # Player vs AI
│   │   ├── multiplay/           # Online PvP (host/client)
│   │   └── general/             # Shared mode utilities
│   └── feature/              # Game features
│       ├── game_logger.c/h      # Log moves to file (gomoku-YYYYMMDD-HHMM.log)
│       ├── replay.c/h           # Replay playback system
│       ├── replay_viewer.c/h    # Replay UI
│       └── command.c/h          # Chat commands (/quit, /undo, /giveup)
├── ui/
│   ├── core/                 # UI infrastructure
│   │   ├── ui_manager.c/h       # Window layout manager (100x31 grid)
│   │   ├── theme.c/h            # Color theme system
│   │   ├── input_handler.c/h    # Unified input handling (keyboard + gamepad)
│   │   └── gamepad_input.c/h    # Gamepad support
│   ├── game/                 # In-game UI components
│   │   ├── board/               # Board rendering
│   │   ├── border/              # UI borders and decorations
│   │   ├── chat/                # Chat box UI
│   │   ├── info/                # Game info panel
│   │   └── log/                 # Move log UI
│   └── menu/                 # Menu system
│       ├── menu_ui.c/h          # Main menu
│       ├── modal_ui.c/h         # Modal dialogs
│       └── theme_selector_ui.c/h
├── network/
│   ├── core/                 # Network layer
│   │   ├── network.c/h          # TCP socket management, server/client
│   │   └── protocol.c/h         # Message serialization/deserialization
│   └── messages/             # Message definitions
│       ├── message.h            # Message union type
│       ├── message_types.h      # MessageType enum, MessageHeader
│       ├── connection.h         # Connection handshake messages
│       ├── game.h               # Game state messages
│       ├── chat.h               # Chat messages
│       └── spectator.h          # Spectator messages
└── utils/
    ├── config.c/h            # Configuration management
    ├── arg_parser.c/h        # CLI argument parsing
    └── terminal_check.c/h    # Terminal size validation
```

### Key Architectural Patterns

#### 1. UI Layout System (100x31 Grid)

The game uses a fixed 100x31 terminal layout defined in `ui/core/ui_manager.h`:

- **Board area** (left): 49x25 (columns 0-48, rows 0-24)
- **Info panel** (top-right): 52x3 (columns 48-99, rows 0-2)
- **Chat area** (middle-right): 52x21 (columns 48-99, rows 2-22)
- **Bottom panel**: 100x6 (columns 0-99, rows 24-29)
  - Last move display, current turn indicator
  - Timer, play time counter
  - System log

All UI components respect these boundaries. When modifying UI, ensure changes align with the layout in `src/ui/game/border/ingame_border.c`.

#### 2. Network Protocol Architecture

The network layer uses a custom binary protocol with a message-based architecture:

**Message Structure**:

```c
Message {
    MessageHeader header;  // type, sequence, timestamp, payload_size
    union payload {
        // Connection: ConnectMessage, ConnectAckMessage, PlayerInfoMessage, etc.
        // Game: GameStartMessage, MoveMessage, MoveAckMessage, etc.
        // Chat: ChatMessage, CommandMessage, etc.
        // Spectator: SpectatorConnectMessage, etc.
    }
}
```

**Protocol Flow** (Multiplayer):

1. Client → Server: `MSG_CONNECT` (player name)
2. Server → Client: `MSG_CONNECT_ACK` (assigned color, rule)
3. Server → Client: `MSG_GAME_START` (player info)
4. During game:
   - Player → Opponent: `MSG_MOVE` (row, col)
   - Opponent → Player: `MSG_MOVE_ACK` (success/forbidden)
   - Player ↔ Opponent: `MSG_CURSOR_UPDATE` (cursor position)
   - Player ↔ Opponent: `MSG_CHAT` (message)
5. End: `MSG_GAME_RESULT` (winner, reason)

**Spectator Flow**:

- Spectator → Server: `MSG_SPECTATOR_CONNECT`
- Server → Spectator: `MSG_SPECTATOR_CONNECT_ACK` + state sync
- Server → All Spectators: Broadcasts of all game events

See `src/network/core/network.h` and `src/network/messages/` for full protocol details.

#### 3. Game State Management

Game state flows through these core modules:

1. **Board** (`game/core/board.c`): 19x19 grid state, move validation, forbidden move marking
2. **GameLogic** (`game/core/game_logic.c`): Win detection (five-in-a-row checking)
3. **Rules** (`game/core/rules.c`): Renju rules (3-3, 4-4, overline forbidden for BLACK)
4. **TurnManager** (`game/core/turn_manager.c`): Turn switching, timer management (20s per turn)

**Renju Rules**: When `GameRule = RULE_RENJU`, BLACK cannot make:

- **3-3**: Two open-three patterns simultaneously
- **4-4**: Two open-four patterns simultaneously
- **Overline**: More than 5 stones in a row

Forbidden positions are marked on the board and displayed in the UI.

#### 4. Input Handling System

Unified input system (`ui/core/input_handler.c`) supports both keyboard and gamepad:

**Keyboard Controls**:

- Arrow keys / WASD / HJKL: Move cursor
- Enter / Space: Place stone / Select
- ESC / Q: Quit / Back
- C: Chat mode
- R: Request undo

**Gamepad Support**: Xbox/PlayStation controller support via Linux input event interface (`ui/core/gamepad_input.c`).

#### 5. Multiplayer Game Flow

**Host** (`game/mode/multiplay/multiplayer.c` - `multiplayer_run_host()`):

1. Initialize network server, listen on port
2. Accept client connection
3. Assign colors (host = BLACK, client = WHITE)
4. Send `MSG_GAME_START` with player info
5. Game loop: handle moves, chat, cursor updates
6. Broadcast to spectators

**Client** (`multiplayer_run_client()`):

1. Connect to server IP:port
2. Receive color assignment and rule from server
3. Receive `MSG_GAME_START`
4. Game loop: synchronized with host

**Key files**:

- `mp_types.h`: Shared types (`MultiplayerGame`, `PlayerData`)
- `mp_game.c/h`: Core game loop
- `mp_network.c/h`: Network event handling
- `mp_turn.c/h`: Turn management
- `mp_input.c/h`: Input processing

#### 6. AI Engine

Two difficulty levels (`game/ai/ai_engine.c`):

- **AI_EASY**: Heuristic-based evaluation

  - Pattern recognition (open-four, threat detection)
  - Position scoring
  - Quick response (~instant)

- **AI_HARD**: Minimax with alpha-beta pruning
  - Depth-limited search (adjustable depth)
  - Board evaluation function
  - Considers offensive and defensive moves
  - Response time: ~1-3 seconds

## Important Constants

```c
// Board
#define BOARD_SIZE 19                    // 19x19 board

// Terminal
#define UI_MIN_WIDTH 100
#define UI_MIN_HEIGHT 31

// Network
#define DEFAULT_PORT 7773
#define MAX_SPECTATORS 3
#define MAX_PLAYER_NAME 8

// Game
#define TURN_TIMEOUT 20                  // seconds
#define MAX_MOVE_HISTORY (BOARD_SIZE * BOARD_SIZE)

// UI Layout (from ui_manager.h)
#define BOARD_WINDOW_WIDTH 47
#define BOARD_WINDOW_HEIGHT 23
#define CHAT_WINDOW_WIDTH 50
#define CHAT_WINDOW_HEIGHT 18
```

## Development Guidelines

### Adding New Game Modes

1. Create mode directory in `src/game/mode/<mode_name>/`
2. Implement mode entry point: `<mode_name>_run(args...)`
3. Add mode enum to `utils/arg_parser.h`
4. Wire mode in `main.c` switch statement
5. Use `MultiplayerGame` struct pattern for state management

### Working with ncurses UI

- Always use ncursesw (wide character support) for UTF-8
- Respect the 100x31 layout defined in `ui_manager.h`
- Use theme system (`ui/core/theme.c`) for colors
- Test with `export LANG=ko_KR.UTF-8` for Korean character support
- Call `wresize(stdscr, 31, 100)` to enforce terminal size

### Network Message Development

1. Define message struct in appropriate header (`network/messages/*.h`)
2. Add message type to `MessageType` enum in `message_types.h`
3. Add union member in `message.h`
4. Implement serialization/deserialization in `protocol.c`
5. Update `protocol_get_message_size()` for new message type

### Testing Strategy

- Unit tests use `test_framework.c/h` (custom test framework)
- Integration tests in `test/test_phase*.c`
- Test both RULE_STANDARD and RULE_RENJU for rule-dependent features
- Network tests: test both host and client roles

## Common Tasks

### Run the game in menu mode

```bash
./build_and_run.sh
```

### Build and run a specific test

```bash
cd build
cmake ..
make test_board
./test_board
```

### Debug network issues

Enable verbose logging in `network/core/network.c` and check socket state transitions.

### Add a new UI component

1. Create `.c/.h` in appropriate `ui/` subdirectory
2. Define window dimensions in `ui_manager.h`
3. Integrate in game loop (singleplay or multiplayer)
4. Test with `test_ingame_border` or `test_phase3_ui`

### Fix forbidden move detection

The logic is in `game/core/rules.c`. Update `board_update_forbidden_marks()` and test with `test_rules`.

## Dependencies

- **ncursesw**: Wide character ncurses for UTF-8 support
- **pthread**: For future async networking (currently unused)
- **Standard C libraries**: socket, netinet, arpa/inet

Install on Ubuntu/Debian:

```bash
sudo apt-get install build-essential cmake libncursesw5-dev
```

## Localization Notes

The codebase contains Korean comments and UI text. When modifying:

- Preserve UTF-8 encoding
- Use `setlocale(LC_ALL, "")` before ncurses initialization
- Test Korean character rendering with `export LC_ALL=ko_KR.UTF-8`
