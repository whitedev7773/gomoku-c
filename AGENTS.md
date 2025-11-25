# AGENTS.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Gomoku-C** is a terminal-based Gomoku (Five-in-a-Row) game written in C for Linux/Debian OS. The project supports single-player (vs AI), multiplayer (LAN-based), spectator mode, and replay functionality.

**Target Platform**: Linux/Debian OS
**UI**: Terminal-based (ncurses), minimum terminal size 120x30
**Network**: TCP socket communication with protobuf-c for serialization
**Build System**: CMake

## Building and Running

### Build Commands

```bash
# Standard build
mkdir -p build && cd build
cmake ..
make

# Run executable
./build/gomoku-c

# Clean build using script
./build_and_run.sh --clean

# Standard build and run
./build_and_run.sh
```

### Command Line Arguments

The game supports immediate launch with different modes:

```bash
# Singleplay modes
./build/gomoku-c --singleplay --easy
./build/gomoku-c --singleplay --hard

# Multiplayer modes
./build/gomoku-c --multiplay-host
./build/gomoku-c --multiplay-client -ip 192.168.0.2
./build/gomoku-c --multiplay-client -ip 192.168.0.2 -port 9000

# Spectator mode (max 3 spectators)
./build/gomoku-c --spectator -ip 192.168.0.2 -port 7773

# Help
./build/gomoku-c --help
```

**Default port**: 7773 (for multiplayer and spectator modes)

## Architecture

### Directory Structure

```
src/
├── main.c              # Entry point, mode routing, terminal size checking
├── utils/              # Utility modules (Phase 1 - COMPLETE)
│   ├── arg_parser.c/h      # CLI argument parsing
│   └── terminal_check.c/h  # Terminal size validation (120x30)
├── game/               # Game logic (Phase 2 - TODO)
│   └── (board, win detection, turn management)
├── ui/                 # UI rendering (Phase 3 - TODO)
│   └── (ncurses-based game board, chat, timers)
└── network/            # Network communication (Phase 5 - TODO)
    └── (TCP sockets, protobuf-c messages)
```

### Key Components

#### 1. Terminal Size Checking (src/utils/terminal_check.c)

- **Requirement**: Minimum 120x30 terminal size (hard requirement)
- Blocks execution until terminal is properly sized
- Uses `ioctl(STDOUT_FILENO, TIOCGWINSZ, &w)` to detect size

#### 2. CLI Argument Parser (src/utils/arg_parser.c)

- Parses 6 game modes: menu, singleplay (easy/hard), multiplay (host/client), spectator
- Handles optional `-port <PORT>` argument (default: 7773)
- Returns `ParsedArgs` struct with `GameMode`, IP address, port, and validation status

#### 3. Main Entry Point (src/main.c)

- Flow: Parse args → Check terminal size → Route to game mode
- Currently shows placeholder messages for unimplemented phases
- Each mode will be implemented according to `roadmap.md` phases

## Development Phases

The project follows a **14-phase roadmap** (see `roadmap.md`). Current status: **Phase 1 complete**.

### Phase Status

- **Phase 1** ✅: Project foundation (CMake, terminal check, CLI parser)
- **Phase 2-14** 🚧: In planning (game logic, UI, networking, etc.)

### Key Implementation Notes

#### Game Rules & Features

- **Board size**: 15x15 or 19x19 (to be decided in Phase 2)
- **Turn timer**: 20 seconds per turn
- **Player names**: Max 8 characters
- **Chat messages**: Max 15 characters
- **Spectators**: Max 3 per game
- **Game logs**: Auto-saved as `gomoku-{YYYYMMDD}-{HH:MM}.log`

#### Networking (Phase 5)

- **Protocol**: TCP sockets with protobuf-c serialization
- **Port forwarding**: Designed to work through NAT with port forwarding
- **Default port**: 7773
- **Domain support**: Should resolve domains like `multiplay.gomoku.kr` to IP

#### Chat Commands (Phase 6)

- `/quit` - Leave game
- `/undo` - Request take-back (10s timeout)
- `/giveup` - Forfeit game
- Command auto-completion with placeholder text

#### Advanced Rules (Phase 12 - Optional)

- **Renju Rule**: Prevents black's advantage (ban 3-3, 4-4, overline)
- **Swap Rule**: After 3 moves, white can swap colors

## Code Conventions

### File Organization

- Headers define public interfaces, implementations in .c files
- Use `#ifndef HEADER_NAME_H` guards for all headers
- Place module-specific constants in headers (e.g., `MIN_TERMINAL_WIDTH`)

### Error Handling

- Validation errors should display error message + usage information
- Network errors should show user-friendly messages and return to main menu
- Terminal size errors block execution with clear instructions

### Build System

- CMakeLists.txt uses `GLOB_RECURSE` to auto-discover source files
- New modules in `src/*/` are automatically included
- Link libraries: `ncurses` (current), `protobuf-c` (Phase 5), `pthread` (Phase 5)

## Important Constants

```c
// Terminal
MIN_TERMINAL_WIDTH  = 120
MIN_TERMINAL_HEIGHT = 30

// Network
DEFAULT_PORT        = 7773
MAX_SPECTATORS      = 3

// Game
TURN_TIMEOUT        = 20  // seconds
MAX_PLAYER_NAME     = 8   // characters
MAX_CHAT_MESSAGE    = 15  // characters
```

## Testing

When implementing new features:

1. Build successfully: `cd build && cmake .. && make`
2. Test all command-line argument combinations
3. Verify terminal size requirements (test with smaller terminals)
4. Check that mode routing works correctly in `main.c`

## Next Steps

Refer to `roadmap.md` for the complete implementation plan. The next phases are:

- **Phase 2**: Core game logic (board, win detection, turn management)
- **Phase 3**: Basic UI (ncurses rendering, input handling)
- **Phase 4**: AI implementation (Easy: heuristic, Hard: minimax)

See `features.txt` for detailed feature specifications.
