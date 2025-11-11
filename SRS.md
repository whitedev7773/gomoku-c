# Software Requirements Specification (SRS)
# Gomoku.C - Terminal-Based Network Gomoku Game

**Version:** 1.0
**Date:** 2025-11-11
**Project:** ELEC462 System Programming Team Project
**Team:** 임대자들 (권선우, 장기원)

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Overall Description](#2-overall-description)
3. [System Architecture](#3-system-architecture)
4. [Functional Requirements](#4-functional-requirements)
5. [Non-Functional Requirements](#5-non-functional-requirements)
6. [External Interface Requirements](#6-external-interface-requirements)
7. [System Features](#7-system-features)
8. [Implementation Details](#8-implementation-details)
9. [Development Plan](#9-development-plan)
10. [Appendix](#10-appendix)

---

## 1. Introduction

### 1.1 Purpose

This Software Requirements Specification (SRS) document provides a comprehensive description of the **Gomoku.C** project - a terminal-based network Gomoku (Five in a Row) game implemented in C. This document is intended for:

- Development team members
- Course instructors and evaluators
- Future maintainers and contributors
- AI-assisted development (Vibe-Coding) tools

### 1.2 Scope

**Gomoku.C** is a full-featured terminal-based multiplayer game that demonstrates advanced system programming concepts including:

- **Network Communication:** TCP/IP socket programming with protobuf-c serialization
- **Concurrency:** Multi-threading using POSIX threads (pthread)
- **Terminal UI:** Advanced ncurses-based user interface
- **Game Logic:** Complete Gomoku implementation with Renju and Swap rules
- **AI System:** Easy and Hard difficulty AI opponents
- **Logging & Replay:** Automatic game logging and replay functionality

**Key Features:**
- Single-player (vs AI), Multiplayer (LAN/WAN), and Spectator modes
- Real-time game state synchronization
- In-game chat system with command support
- Game logging and replay system
- Multiple board themes
- Gamepad input support
- Comprehensive error handling

### 1.3 Definitions, Acronyms, and Abbreviations

| Term | Definition |
|------|------------|
| **Gomoku** | A traditional board game also known as "Five in a Row" |
| **Renju** | A professional variant of Gomoku with additional rules for black stones |
| **TUI** | Text User Interface / Terminal User Interface |
| **TCP/IP** | Transmission Control Protocol / Internet Protocol |
| **AI** | Artificial Intelligence |
| **LAN** | Local Area Network |
| **WAN** | Wide Area Network |
| **protobuf-c** | Protocol Buffers C implementation for data serialization |
| **ncurses** | New curses - a library for terminal UI control |
| **pthread** | POSIX Threads - a threading API |
| **P1** | Player 1 (always the local player) |
| **P2** | Player 2 (opponent or remote player) |
| **CLI** | Command Line Interface |

### 1.4 References

1. **Course Materials:**
   - ELEC462 System Programming Course Guidelines
   - Team Project Overview and Proposal Guidelines

2. **Technical Documentation:**
   - POSIX.1-2017 (IEEE Std 1003.1-2017)
   - ncurses Programming HOWTO
   - Protocol Buffers Documentation
   - TCP/IP Illustrated, Volume 1 by W. Richard Stevens

3. **Project Documents:**
   - CLAUDE.md (Project Instructions)
   - Features.txt (Feature Specifications)
   - FAQ.txt (Course Q&A)
   - Gomoku-C.pdf (Proposal Presentation)

### 1.5 Overview

This document is organized into 10 major sections covering all aspects of the Gomoku.C system:

- **Sections 1-2:** Introduction and high-level description
- **Section 3:** System architecture and design principles
- **Sections 4-5:** Functional and non-functional requirements
- **Sections 6-7:** Interface requirements and detailed feature descriptions
- **Section 8:** Technical implementation details
- **Section 9:** Development timeline and milestones
- **Section 10:** Appendices with additional information

---

## 2. Overall Description

### 2.1 Product Perspective

Gomoku.C is a standalone terminal-based application that operates within the Linux/Debian OS environment. The system consists of:

1. **Client Application:** The main executable that users interact with
2. **Network Layer:** TCP/IP socket communication for multiplayer mode
3. **File System:** Game logs stored locally for replay functionality

**System Context:**

```
┌─────────────────────────────────────────────────────────┐
│                    User Environment                      │
│  ┌────────────┐         ┌────────────┐                 │
│  │  Terminal  │         │  Keyboard  │                 │
│  │  (120x30)  │         │  /Gamepad  │                 │
│  └─────┬──────┘         └──────┬─────┘                 │
│        │                       │                        │
│  ┌─────▼───────────────────────▼─────┐                 │
│  │      Gomoku.C Application          │                 │
│  │  (Presentation + Application +     │                 │
│  │   Domain + Infrastructure Layers)  │                 │
│  └────┬────────────────┬──────────┬───┘                 │
│       │                │          │                     │
│  ┌────▼────┐    ┌──────▼─────┐  ┌▼──────────┐         │
│  │ Network │    │ File System│  │ /dev/input│         │
│  │ (TCP/IP)│    │  (.log)    │  │ (gamepad) │         │
│  └────┬────┘    └────────────┘  └───────────┘         │
└───────┼──────────────────────────────────────────────┘
        │
   ┌────▼────┐
   │ Internet│
   │   /LAN  │
   └─────────┘
```

### 2.2 Product Functions

**Primary Functions:**

1. **Game Modes**
   - FR-GM-01: Single-player mode with AI opponents (Easy/Hard)
   - FR-GM-02: Multiplayer mode (Host/Client) over TCP/IP
   - FR-GM-03: Spectator mode for watching live games
   - FR-GM-04: Replay mode for viewing saved games

2. **Game Logic**
   - FR-GL-01: Standard Gomoku win detection (exactly 5 in a row)
   - FR-GL-02: Renju rule enforcement (3-3, 4-4, overline prohibition for black)
   - FR-GL-03: Swap rule implementation (color exchange after 3 moves)
   - FR-GL-04: Turn timer (20 seconds per turn)
   - FR-GL-05: Move validation and illegal move prevention

3. **Network Communication**
   - FR-NC-01: TCP socket-based client-server architecture
   - FR-NC-02: Real-time game state synchronization
   - FR-NC-03: Chat message transmission
   - FR-NC-04: Multiple spectator support
   - FR-NC-05: Connection status monitoring and latency display

4. **User Interface**
   - FR-UI-01: 15x15 Gomoku board with coordinate labels
   - FR-UI-02: Cursor-based stone placement
   - FR-UI-03: Chat system with command support
   - FR-UI-04: Timer display (progress bar + text)
   - FR-UI-05: Game information panel (opponent, spectators, ping, turn info)
   - FR-UI-06: Modal dialogs (undo requests, game end notifications)
   - FR-UI-07: System log display

5. **Logging & Replay**
   - FR-LR-01: Automatic game log generation
   - FR-LR-02: Log file management and listing
   - FR-LR-03: Step-by-step replay playback

6. **Additional Features**
   - FR-AF-01: Multiple board themes
   - FR-AF-02: Gamepad input support
   - FR-AF-03: Command-line argument support for direct mode launch

### 2.3 User Characteristics

**Primary Users:**

1. **Casual Players**
   - Experience Level: Familiar with terminal environments
   - Technical Skills: Basic Linux command usage
   - Usage: Single-player games against AI

2. **Competitive Players**
   - Experience Level: Intermediate to advanced Gomoku players
   - Technical Skills: Network configuration (port forwarding)
   - Usage: Multiplayer games, understanding of Renju/Swap rules

3. **Spectators**
   - Experience Level: Any level
   - Technical Skills: Basic networking knowledge
   - Usage: Watching live games

4. **Developers/Reviewers**
   - Experience Level: Advanced
   - Technical Skills: C programming, system programming concepts
   - Usage: Code review, testing, evaluation

### 2.4 Constraints

**Technical Constraints:**

1. **CT-01: Language Requirement**
   - Primary implementation language: C
   - Minimum 500 lines of C code (excluding comments)
   - AI components may use Python if necessary for complexity management

2. **CT-02: System Call Requirement**
   - Must use at least 5 different system calls
   - Examples: socket(), bind(), listen(), accept(), open(), read(), write(), etc.

3. **CT-03: Core Functionality Requirement**
   - Must implement at least 3 distinct core functionalities
   - Each functionality must be meaningful and independent

4. **CT-04: Platform Requirement**
   - Operating System: Ubuntu 24.04
   - Terminal: Fixed size 120 columns × 30 rows
   - Character Encoding: UTF-8

5. **CT-05: Build System**
   - Must provide Makefile for compilation
   - Must include README with build/run instructions

6. **CT-06: Development Timeline**
   - Total development period: 6 weeks
   - Final presentation: Week 6
   - Code submission deadline: Week 6 + 11 days

**Design Constraints:**

1. **CD-01: Clean Architecture**
   - Must follow Clean Architecture principles
   - Clear separation between Presentation, Application, Domain, and Infrastructure layers

2. **CD-02: Network Protocol**
   - Must use TCP/IP for network communication
   - Must use protobuf-c for message serialization

3. **CD-03: UI Framework**
   - Must use ncurses for terminal UI rendering
   - No GUI frameworks allowed

### 2.5 Assumptions and Dependencies

**Assumptions:**

1. **AS-01:** Users have a terminal emulator that supports:
   - 120×30 character display
   - UTF-8 encoding
   - ANSI color codes (for themes)
   - Keyboard input (arrow keys, Enter, etc.)

2. **AS-02:** For multiplayer:
   - Players have network connectivity
   - Firewall/router can be configured for port forwarding
   - Network latency is reasonable (<1000ms)

3. **AS-03:** Users have basic familiarity with:
   - Linux terminal operations
   - TCP/IP concepts (for hosting/joining games)

**Dependencies:**

1. **DP-01: Runtime Dependencies**
   - ncurses library (v6.0+)
   - pthread library (POSIX threads)
   - protobuf-c library (v1.3.0+)
   - Standard C library (glibc)

2. **DP-02: Build Dependencies**
   - GCC compiler (v9.0+)
   - Make build system
   - CMake (for dependency management)
   - pkg-config

3. **DP-03: Optional Dependencies**
   - Python 3.x (if AI is implemented separately)
   - Linux input subsystem (for gamepad support)

---

## 3. System Architecture

### 3.1 Architectural Style

Gomoku.C follows **Clean Architecture** principles with four distinct layers:

```
┌─────────────────────────────────────────────────────────┐
│              PRESENTATION LAYER                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ UI Renderer  │  │Input Handler │  │Theme Manager │ │
│  │  (ncurses)   │  │  (keyboard/  │  │              │ │
│  │              │  │   gamepad)   │  │              │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│              APPLICATION LAYER                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │Game Controller│ │Command       │  │State Manager │ │
│  │              │  │Processor     │  │              │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│  ┌──────────────┐  ┌──────────────┐                   │
│  │Timer Manager │  │Mode Manager  │                   │
│  └──────────────┘  └──────────────┘                   │
├─────────────────────────────────────────────────────────┤
│              DOMAIN LAYER                                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Game Logic   │  │  AI Engine   │  │Renju/Swap    │ │
│  │ (Move Valid.,│  │  (Easy/Hard) │  │Rule Engine   │ │
│  │  Win Detect.)│  │              │  │              │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│  ┌──────────────┐                                      │
│  │Board Data    │                                      │
│  │Structure     │                                      │
│  └──────────────┘                                      │
├─────────────────────────────────────────────────────────┤
│              INFRASTRUCTURE LAYER                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │Network       │  │File Manager  │  │System Call   │ │
│  │Manager       │  │(Log I/O)     │  │Wrappers      │ │
│  │(TCP Socket + │  │              │  │              │ │
│  │ protobuf-c)  │  │              │  │              │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│  ┌──────────────┐                                      │
│  │Configuration │                                      │
│  │Manager       │                                      │
│  └──────────────┘                                      │
└─────────────────────────────────────────────────────────┘
```

### 3.2 Layer Responsibilities

**3.2.1 Presentation Layer**

- **Responsibility:** User interface rendering and input handling
- **Key Components:**
  - `UI Renderer`: ncurses-based display output
  - `Input Handler`: Keyboard/gamepad event processing
  - `Theme Manager`: Board color scheme management
- **Dependencies:** Application Layer (receives game state, sends user actions)

**3.2.2 Application Layer**

- **Responsibility:** Game flow control, command processing, state coordination
- **Key Components:**
  - `Game Controller`: Main game loop coordination
  - `Command Processor`: Chat command parsing and execution
  - `State Manager`: Game state transitions
  - `Timer Manager`: Turn timer and game clock
  - `Mode Manager`: Game mode switching (single/multi/spectator/replay)
- **Dependencies:** Domain Layer (business logic), Infrastructure Layer (I/O operations)

**3.2.3 Domain Layer**

- **Responsibility:** Core game rules and algorithms (business logic)
- **Key Components:**
  - `Game Logic`: Move validation, win condition detection
  - `AI Engine`: Move evaluation algorithms (Easy/Hard)
  - `Renju/Swap Rule Engine`: Advanced rule enforcement
  - `Board Data Structure`: Game board state representation
- **Dependencies:** None (pure business logic)

**3.2.4 Infrastructure Layer**

- **Responsibility:** External system interactions (I/O, network, system calls)
- **Key Components:**
  - `Network Manager`: TCP socket communication, protobuf serialization
  - `File Manager`: Log file read/write operations
  - `System Call Wrappers`: Abstraction over POSIX system calls
  - `Configuration Manager`: Settings and preferences
- **Dependencies:** Operating system, external libraries

### 3.3 Design Principles

**DP-01: Dependency Rule**
- Inner layers (Domain) must not depend on outer layers (Infrastructure)
- All dependencies point inward toward the Domain Layer

**DP-02: Single Responsibility**
- Each module has one clear responsibility
- Separation of concerns across layers

**DP-03: Interface Abstraction**
- Domain Layer defines interfaces
- Infrastructure Layer provides implementations

**DP-04: Testability**
- Each layer can be tested independently
- Mock implementations for external dependencies

---

## 4. Functional Requirements

### 4.1 Game Mode Requirements

#### FR-GM-01: Single-Player Mode

**Description:** User can play against AI opponent

**Priority:** High (Core Functionality #1)

**Requirements:**
- FR-GM-01.1: System shall provide AI difficulty selection (Easy/Hard)
- FR-GM-01.2: User shall always play as black (first move)
- FR-GM-01.3: AI shall respond within 2 seconds per move
- FR-GM-01.4: AI shall comply with all Renju rules when playing as black

**Acceptance Criteria:**
- User can start a single-player game via main menu
- AI makes legal moves according to difficulty level
- Game ends with proper win/loss detection

#### FR-GM-02: Multiplayer Mode

**Description:** Two users can play over network

**Priority:** High (Core Functionality #2)

**Requirements:**
- FR-GM-02.1: System shall support Host mode (creates game server)
- FR-GM-02.2: System shall support Client mode (connects to host)
- FR-GM-02.3: Host shall display local IP address for sharing
- FR-GM-02.4: Client shall connect using IP address input
- FR-GM-02.5: System shall support LAN and WAN connections (with port forwarding)
- FR-GM-02.6: System shall accept player names (max 8 characters)
- FR-GM-02.7: System shall synchronize game state between players in real-time

**Acceptance Criteria:**
- Two users can successfully connect and play
- Game state remains synchronized
- Network disconnection is handled gracefully

#### FR-GM-03: Spectator Mode

**Description:** Users can watch ongoing games

**Priority:** Medium

**Requirements:**
- FR-GM-03.1: System shall allow spectator connection to active game
- FR-GM-03.2: System shall support multiple simultaneous spectators (up to 10)
- FR-GM-03.3: Spectators shall see real-time game updates
- FR-GM-03.4: Spectators shall have read-only access (cannot place stones)
- FR-GM-03.5: System shall display spectator count to players
- FR-GM-03.6: System shall return error if connecting to non-existent game

**Acceptance Criteria:**
- Spectators can join and watch games
- Spectator count is accurate
- Spectators receive all game updates

#### FR-GM-04: Replay Mode

**Description:** Users can view saved games

**Priority:** Medium (Core Functionality #3)

**Requirements:**
- FR-GM-04.1: System shall list available replay files
- FR-GM-04.2: User shall navigate replay list with arrow keys
- FR-GM-04.3: System shall play back moves step-by-step
- FR-GM-04.4: User shall exit replay with `/quit` command

**Acceptance Criteria:**
- Replay files are displayed in menu
- Replay playback shows moves sequentially
- User can exit replay at any time

### 4.2 Game Logic Requirements

#### FR-GL-01: Standard Gomoku Rules

**Description:** Implement basic Gomoku win condition

**Priority:** High

**Requirements:**
- FR-GL-01.1: System shall detect exactly 5 stones in a row as win
- FR-GL-01.2: System shall check horizontal, vertical, and both diagonals
- FR-GL-01.3: System shall check after each stone placement
- FR-GL-01.4: System shall end game immediately upon win detection
- FR-GL-01.5: System shall display winner information

**Acceptance Criteria:**
- All win patterns (horizontal, vertical, diagonal) are detected
- Game ends immediately when 5 in a row is achieved

#### FR-GL-02: Renju Rules

**Description:** Implement Renju rule restrictions for black stones

**Priority:** High

**Requirements:**
- FR-GL-02.1: System shall prohibit 3-3 pattern for black
  - 3-3: Creating two open three-in-a-rows simultaneously
- FR-GL-02.2: System shall prohibit 4-4 pattern for black
  - 4-4: Creating two open four-in-a-rows simultaneously
- FR-GL-02.3: System shall prohibit overline (6+ stones) for black
- FR-GL-02.4: White stones shall not be restricted by Renju rules
- FR-GL-02.5: System shall validate moves before placement
- FR-GL-02.6: System shall provide feedback for illegal moves

**Acceptance Criteria:**
- Black cannot make 3-3, 4-4, or overline moves
- White can make any legal move
- Illegal move attempts are rejected with explanation

#### FR-GL-03: Swap Rule

**Description:** Implement color exchange option after 3 moves

**Priority:** Medium

**Requirements:**
- FR-GL-03.1: After Black1, White1, Black2, white player can request swap
- FR-GL-03.2: System shall display swap option to white player
- FR-GL-03.3: If accepted, colors are exchanged
- FR-GL-03.4: Swap offer expires after 10 seconds (auto-decline)

**Acceptance Criteria:**
- Swap option appears after 3rd move
- Color exchange works correctly if accepted
- Game continues normally if declined

#### FR-GL-04: Turn Timer

**Description:** Implement time limit per turn

**Priority:** High

**Requirements:**
- FR-GL-04.1: Each turn shall have 20-second time limit
- FR-GL-04.2: Timer shall reset after each move
- FR-GL-04.3: Exceeding time limit results in automatic forfeit
- FR-GL-04.4: Timer shall display as progress bar (20 segments)
- FR-GL-04.5: Timer shall display as text (e.g., "15s")

**Acceptance Criteria:**
- Timer counts down from 20 seconds
- Auto-forfeit occurs at 0 seconds
- Visual feedback is clear

#### FR-GL-05: Move Validation

**Description:** Validate all move attempts

**Priority:** High

**Requirements:**
- FR-GL-05.1: System shall reject moves on occupied positions
- FR-GL-05.2: System shall reject moves outside board boundaries
- FR-GL-05.3: System shall reject moves during opponent's turn
- FR-GL-05.4: System shall validate Renju rules for black moves
- FR-GL-05.5: System shall provide error messages for invalid moves

**Acceptance Criteria:**
- All invalid move types are rejected
- Error messages are informative

### 4.3 Network Communication Requirements

#### FR-NC-01: TCP Socket Communication

**Description:** Implement client-server network architecture

**Priority:** High

**Requirements:**
- FR-NC-01.1: System shall use TCP/IP protocol
- FR-NC-01.2: Host shall bind to configurable port (default: 7773)
- FR-NC-01.3: Host shall listen for incoming connections
- FR-NC-01.4: Client shall connect to host IP:port
- FR-NC-01.5: System shall use protobuf-c for message serialization
- FR-NC-01.6: System shall support messages: MOVE, CHAT, SYSTEM, UNDO_REQUEST, UNDO_RESPONSE, FORFEIT, JOIN, SPECTATE

**Acceptance Criteria:**
- Connections establish successfully
- Messages are transmitted reliably
- Protocol is efficient and extensible

#### FR-NC-02: Game State Synchronization

**Description:** Keep game state consistent across network

**Priority:** High

**Requirements:**
- FR-NC-02.1: System shall broadcast moves to all connected clients
- FR-NC-02.2: System shall synchronize board state
- FR-NC-02.3: System shall synchronize turn information
- FR-NC-02.4: System shall synchronize timer state
- FR-NC-02.5: System shall handle out-of-order messages
- FR-NC-02.6: Updates shall occur within 100ms latency (local network)

**Acceptance Criteria:**
- All clients display identical game state
- No desynchronization under normal conditions

#### FR-NC-03: Chat Message Transmission

**Description:** Enable text communication between players

**Priority:** Medium

**Requirements:**
- FR-NC-03.1: System shall transmit chat messages (max 15 characters)
- FR-NC-03.2: System shall display sender identification
- FR-NC-03.3: System shall display timestamp for each message
- FR-NC-03.4: System shall support chat commands (see FR-UI-03)

**Acceptance Criteria:**
- Chat messages are received and displayed correctly
- Commands are processed appropriately

#### FR-NC-04: Spectator Support

**Description:** Allow multiple spectators per game

**Priority:** Medium

**Requirements:**
- FR-NC-04.1: System shall maintain list of spectators
- FR-NC-04.2: System shall broadcast game updates to spectators
- FR-NC-04.3: Spectators shall receive all game events
- FR-NC-04.4: Spectator join/leave shall be announced
- FR-NC-04.5: System shall support up to 10 spectators

**Acceptance Criteria:**
- Spectators receive real-time updates
- Spectator count is accurate

#### FR-NC-05: Connection Monitoring

**Description:** Monitor network connection status

**Priority:** High

**Requirements:**
- FR-NC-05.1: System shall detect connection loss
- FR-NC-05.2: System shall display connection error messages
- FR-NC-05.3: System shall measure and display network latency (PING)
- FR-NC-05.4: PING shall update every 1 second
- FR-NC-05.5: System shall handle graceful disconnection

**Acceptance Criteria:**
- Disconnections are detected within 5 seconds
- PING display is accurate (±10ms)

### 4.4 User Interface Requirements

#### FR-UI-01: Game Board Display

**Description:** Display 15×15 Gomoku board

**Priority:** High

**Requirements:**
- FR-UI-01.1: Board shall be 15×15 grid
- FR-UI-01.2: Columns shall be labeled A-O (skipping I)
- FR-UI-01.3: Rows shall be labeled 1-19
- FR-UI-01.4: Empty positions shall be displayed as dots (·)
- FR-UI-01.5: Black stones shall be displayed as filled circles (●)
- FR-UI-01.6: White stones shall be displayed as open circles (○)
- FR-UI-01.7: Cursor shall be visually distinct
- FR-UI-01.8: Last move shall be highlighted

**Acceptance Criteria:**
- Board is clearly readable
- Stones are visually distinct
- Coordinate system is accurate

#### FR-UI-02: Cursor Control

**Description:** Navigate board with cursor

**Priority:** High

**Requirements:**
- FR-UI-02.1: Arrow keys (↑↓←→) shall move cursor
- FR-UI-02.2: Spacebar shall place stone at cursor position
- FR-UI-02.3: Cursor shall wrap at board edges (optional)
- FR-UI-02.4: Cursor shall be disabled during opponent's turn
- FR-UI-02.5: Opponent's cursor shall be displayed dimly during their turn
- FR-UI-02.6: Cursor shall be disabled when modal is active

**Acceptance Criteria:**
- Cursor moves smoothly
- Stone placement is accurate
- Cursor visibility follows game state

#### FR-UI-03: Chat System

**Description:** In-game text communication

**Priority:** Medium

**Requirements:**
- FR-UI-03.1: Chat shall use speech bubble style UI
- FR-UI-03.2: Messages shall display sender name and timestamp
- FR-UI-03.3: Enter or 'T' key shall enter chat mode
- FR-UI-03.4: In chat mode, cursor movement shall be disabled
- FR-UI-03.5: System shall display available commands on first join
- FR-UI-03.6: System shall support commands:
  - `/quit`: Exit game
  - `/undo`: Request move takeback (10-second timeout)
  - `/giveup`: Forfeit immediately
- FR-UI-03.7: System shall show command auto-completion placeholders
  - Example: typing "/q" shows "uit" in gray
- FR-UI-03.8: System shall display error for invalid commands
- FR-UI-03.9: Message length shall be limited to 15 characters

**Acceptance Criteria:**
- Chat messages are readable and well-formatted
- Commands execute correctly
- Auto-completion aids usability

#### FR-UI-04: Timer Display

**Description:** Visual countdown timer

**Priority:** High

**Requirements:**
- FR-UI-04.1: Progress bar shall use 20 segments (█)
- FR-UI-04.2: Filled segments represent remaining time
- FR-UI-04.3: Empty segments (░) represent elapsed time
- FR-UI-04.4: Text display shall show seconds (e.g., "15s")
- FR-UI-04.5: Timer shall update every second
- FR-UI-04.6: Timer shall change color as time runs low (optional)

**Acceptance Criteria:**
- Timer is visually clear
- Progress bar accurately reflects time
- Text matches progress bar

#### FR-UI-05: Information Panel

**Description:** Display game information

**Priority:** Medium

**Requirements:**
- FR-UI-05.1: Shall display opponent name
- FR-UI-05.2: Shall display spectator count
- FR-UI-05.3: Shall display network latency (PING)
- FR-UI-05.4: Shall display port number (host mode)
- FR-UI-05.5: Shall display current turn (P1/P2)
- FR-UI-05.6: Shall display last move position (e.g., "A02")
- FR-UI-05.7: Shall display game uptime (MM:SS format)

**Acceptance Criteria:**
- All information is accurate and updated
- Display is organized and readable

#### FR-UI-06: Modal Dialogs

**Description:** Blocking notification dialogs

**Priority:** Medium

**Requirements:**
- FR-UI-06.1: Modals shall disable cursor movement
- FR-UI-06.2: Undo Request Modal:
  - Display requester name
  - Provide Accept/Decline buttons
  - 10-second timeout (auto-decline)
- FR-UI-06.3: Game End Modal:
  - Display winner/result
  - Display reason (win/forfeit/disconnect)
  - Provide OK button
- FR-UI-06.4: Forfeit Confirmation Modal:
  - Display forfeit warning
  - Provide Confirm/Cancel buttons

**Acceptance Criteria:**
- Modals block game input appropriately
- User choices are processed correctly
- Timeouts work as specified

#### FR-UI-07: System Log

**Description:** Event message display

**Priority:** Low

**Requirements:**
- FR-UI-07.1: Shall display system messages
- FR-UI-07.2: Message types:
  - `[SYSTEM] Room Created! (PORT: 7773)`
  - `[SYSTEM] Player2 joined the game!`
  - `[SYSTEM] P2 placed at A02`
  - `[SYSTEM] SPECTATOR is spectating the game`
  - `[SYSTEM] Connection lost`
- FR-UI-07.3: Shall auto-scroll to show recent messages
- FR-UI-07.4: Shall maintain history of last 10 messages

**Acceptance Criteria:**
- System events are logged
- Messages are timestamped and clear

### 4.5 Logging & Replay Requirements

#### FR-LR-01: Automatic Game Logging

**Description:** Record all game events to file

**Priority:** Medium

**Requirements:**
- FR-LR-01.1: System shall create log file at game start
- FR-LR-01.2: Filename format: `gomoku-YYYYMMDD-HH:MM.log`
- FR-LR-01.3: Shall log all moves (coordinate, player, timestamp)
- FR-LR-01.4: Shall log game events (joins, forfeit, disconnect)
- FR-LR-01.5: Shall log final result (winner, reason)
- FR-LR-01.6: Log file shall be shareable (portable format)

**Acceptance Criteria:**
- Log files are created for all games
- Log format is consistent and parseable
- Logs can be shared between systems

#### FR-LR-02: Replay Playback

**Description:** View saved games step-by-step

**Priority:** Medium

**Requirements:**
- FR-LR-02.1: Shall display list of available log files
- FR-LR-02.2: User shall select log with arrow keys + Enter
- FR-LR-02.3: Shall display moves in chronological order
- FR-LR-02.4: Shall support navigation (forward/backward)
- FR-LR-02.5: Shall display move numbers and timestamps
- FR-LR-02.6: User shall exit with `/quit` command

**Acceptance Criteria:**
- Replay accurately recreates games
- Navigation is intuitive
- Exit returns to main menu

### 4.6 Additional Feature Requirements

#### FR-AF-01: Board Themes

**Description:** Customizable visual styles

**Priority:** Low

**Requirements:**
- FR-AF-01.1: Shall support multiple themes:
  - White (default)
  - Hacker (green on black)
  - Gold (gold/yellow tones)
  - Skyblue (blue tones)
  - Pink (pink tones)
  - Rainbow (multi-color)
- FR-AF-01.2: Theme shall be client-side setting
- FR-AF-01.3: Theme shall not affect game logic
- FR-AF-01.4: Theme shall apply to board, UI elements

**Acceptance Criteria:**
- All themes are visually distinct
- Theme changes do not cause glitches
- Theme preference is saved

#### FR-AF-02: Gamepad Support

**Description:** Alternative input method

**Priority:** Low

**Requirements:**
- FR-AF-02.1: Shall support Xbox-compatible gamepads
- FR-AF-02.2: D-pad/Left stick shall map to arrow keys
- FR-AF-02.3: A button shall map to stone placement
- FR-AF-02.4: Start button shall map to chat entry
- FR-AF-02.5: Gamepad and keyboard shall work simultaneously

**Acceptance Criteria:**
- Gamepad controls function correctly
- Button mappings are intuitive
- No conflicts with keyboard input

#### FR-AF-03: Command-Line Arguments

**Description:** Direct mode launch

**Priority:** Medium

**Requirements:**
- FR-AF-03.1: `./gomoku` - Launch main menu
- FR-AF-03.2: `./gomoku --singleplay --easy` - Easy AI game
- FR-AF-03.3: `./gomoku --singleplay --hard` - Hard AI game
- FR-AF-03.4: `./gomoku --multiplay-host` - Host game
- FR-AF-03.5: `./gomoku --multiplay-client -ip <IP>` - Join game
- FR-AF-03.6: `./gomoku --spectator -ip <IP>` - Spectate game
- FR-AF-03.7: Shall validate arguments and show usage on error

**Acceptance Criteria:**
- All argument combinations work
- Error messages are helpful
- Launch speed is improved for repeat use

---

## 5. Non-Functional Requirements

### 5.1 Performance Requirements

#### NFR-PF-01: Response Time

- **NFR-PF-01.1:** Input lag shall not exceed 50ms
- **NFR-PF-01.2:** AI move calculation (Easy) shall complete within 1 second
- **NFR-PF-01.3:** AI move calculation (Hard) shall complete within 3 seconds
- **NFR-PF-01.4:** Network message transmission latency shall not exceed 100ms (LAN)
- **NFR-PF-01.5:** UI refresh rate shall be at minimum 30 FPS equivalent

#### NFR-PF-02: Throughput

- **NFR-PF-02.1:** System shall support up to 10 concurrent spectators per game
- **NFR-PF-02.2:** System shall handle at least 100 messages per second in chat

#### NFR-PF-03: Resource Usage

- **NFR-PF-03.1:** Memory usage shall not exceed 50MB
- **NFR-PF-03.2:** CPU usage shall not exceed 10% during idle
- **NFR-PF-03.3:** Log files shall not exceed 1MB per game

### 5.2 Reliability Requirements

#### NFR-RL-01: Availability

- **NFR-RL-01.1:** Application shall start successfully 99% of the time
- **NFR-RL-01.2:** Network disconnection shall not cause data loss

#### NFR-RL-02: Fault Tolerance

- **NFR-RL-02.1:** System shall detect and handle network disconnections gracefully
- **NFR-RL-02.2:** System shall recover from invalid input without crashing
- **NFR-RL-02.3:** System shall save game state periodically (for recovery)

#### NFR-RL-03: Error Handling

- **NFR-RL-03.1:** All error conditions shall be caught and logged
- **NFR-RL-03.2:** User-facing error messages shall be clear and actionable
- **NFR-RL-03.3:** Critical errors shall be logged with stack traces

### 5.3 Usability Requirements

#### NFR-US-01: Learnability

- **NFR-US-01.1:** First-time users shall understand basic gameplay within 2 minutes
- **NFR-US-01.2:** Help text shall be displayed for all commands
- **NFR-US-01.3:** UI shall provide visual feedback for all user actions

#### NFR-US-02: Efficiency

- **NFR-US-02.1:** Experienced users shall complete common tasks with minimal keystrokes
- **NFR-US-02.2:** Command auto-completion shall reduce typing

#### NFR-US-03: Accessibility

- **NFR-US-03.1:** UI text shall be readable at terminal default font size
- **NFR-US-03.2:** Color themes shall support colorblind users (high contrast option)

### 5.4 Maintainability Requirements

#### NFR-MT-01: Modularity

- **NFR-MT-01.1:** Code shall follow Clean Architecture layer separation
- **NFR-MT-01.2:** Each module shall have single responsibility
- **NFR-MT-01.3:** Modules shall be loosely coupled

#### NFR-MT-02: Readability

- **NFR-MT-02.1:** Code shall follow consistent naming conventions
- **NFR-MT-02.2:** Complex logic shall include explanatory comments
- **NFR-MT-02.3:** Function length shall not exceed 100 lines

#### NFR-MT-03: Testability

- **NFR-MT-03.1:** Each layer shall be independently testable
- **NFR-MT-03.2:** Business logic shall be testable without I/O dependencies
- **NFR-MT-03.3:** Test coverage shall exceed 70% for core logic

### 5.5 Portability Requirements

#### NFR-PT-01: Platform Compatibility

- **NFR-PT-01.1:** Application shall run on Ubuntu 24.04
- **NFR-PT-01.2:** Application should run on other Linux distributions (Debian-based)
- **NFR-PT-01.3:** Application shall detect and report incompatible environments

#### NFR-PT-02: Build Portability

- **NFR-PT-02.1:** Makefile shall work with GNU Make
- **NFR-PT-02.2:** Build process shall not require manual configuration (auto-detect dependencies)

### 5.6 Security Requirements

#### NFR-SC-01: Input Validation

- **NFR-SC-01.1:** All user input shall be validated before processing
- **NFR-SC-01.2:** Network messages shall be validated before deserialization
- **NFR-SC-01.3:** Buffer overflow protection shall be implemented

#### NFR-SC-02: Data Integrity

- **NFR-SC-02.1:** Game state shall be validated after network transmission
- **NFR-SC-02.2:** Log files shall be write-protected after creation

### 5.7 Compliance Requirements

#### NFR-CP-01: Course Requirements

- **NFR-CP-01.1:** Minimum 500 lines of C code (excluding comments)
- **NFR-CP-01.2:** Minimum 5 different system calls used
- **NFR-CP-01.3:** Minimum 3 core functionalities implemented
- **NFR-CP-01.4:** README and Makefile provided

---

## 6. External Interface Requirements

### 6.1 User Interface

**Terminal Requirements:**
- **Size:** Fixed 120 columns × 30 rows
- **Encoding:** UTF-8
- **Color Support:** ANSI color codes (for themes)

**Input Devices:**
- Keyboard (primary)
- Gamepad (optional, Xbox-compatible)

**UI Layout:**

```
┏━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃   GAME BOARD (15x15)  ┃  INFO PANEL                     ┃
┃                       ┃  - Opponent Name                ┃
┃                       ┃  - Spectator Count              ┃
┃                       ┃  - Ping/Latency                 ┃
┃                       ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                       ┃  CHAT AREA                      ┃
┃                       ┃  - Message bubbles              ┃
┃                       ┃  - Command input                ┃
┣━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ TURN INFO | TIMER    ┃  SYSTEM LOG                     ┃
┃ Last Move | Progress ┃  - Event messages               ┃
┣━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ PLAYER INDICATORS | GAME TIME | ACTION BUTTONS         ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### 6.2 Hardware Interface

**Network Interface:**
- Protocol: TCP/IP
- Default Port: 7773
- Firewall: Port forwarding required for WAN

**Storage:**
- Log files: `./logs/gomoku-YYYYMMDD-HH:MM.log`
- Configuration: `~/.gomoku/config` (optional)

**Input Devices:**
- Keyboard: Standard USB/PS2 keyboard
- Gamepad: `/dev/input/js0` (Linux input subsystem)

### 6.3 Software Interface

**Operating System:**
- Name: Ubuntu
- Version: 24.04 LTS
- Kernel: Linux 5.15+

**Libraries:**
- ncurses (v6.0+): Terminal UI rendering
- pthread (POSIX): Multi-threading
- protobuf-c (v1.3.0+): Message serialization
- glibc: Standard C library

**Network Protocols:**
- TCP/IP: Reliable stream transport
- Protocol Buffers: Message format

### 6.4 Communication Interface

**Network Protocol Specification:**

**Message Types:**

```c
enum MessageType {
    MOVE = 0,           // Stone placement
    CHAT = 1,           // Chat message
    SYSTEM = 2,         // System notification
    UNDO_REQUEST = 3,   // Undo request
    UNDO_RESPONSE = 4,  // Undo accept/decline
    FORFEIT = 5,        // Player forfeit
    JOIN = 6,           // Player join
    SPECTATE = 7        // Spectator join
};
```

**Message Format (protobuf):**

```protobuf
message GameMessage {
    MessageType type = 1;
    string player_name = 2;
    int32 x = 3;                // Board x-coordinate
    int32 y = 4;                // Board y-coordinate
    string message = 5;         // Chat text
    bool accepted = 6;          // Undo response
    int64 timestamp = 7;        // Message timestamp
}
```

**Connection Flow:**

1. **Host Initialization:**
   - `socket()` - Create socket
   - `bind()` - Bind to port 7773
   - `listen()` - Listen for connections
   - `accept()` - Accept client

2. **Client Connection:**
   - `socket()` - Create socket
   - `connect()` - Connect to host IP:port
   - Send JOIN message

3. **Game Communication:**
   - Bidirectional message exchange
   - Heartbeat every 5 seconds
   - Timeout after 30 seconds

4. **Disconnection:**
   - Send FORFEIT message (graceful)
   - `close()` socket

---

## 7. System Features

### 7.1 Feature: Single-Player AI Game

**Description:** Play against computer opponent

**Functional Requirements:**
- FR-GM-01 (Single-Player Mode)
- FR-GL-01 to FR-GL-05 (Game Logic)
- FR-UI-01 to FR-UI-07 (User Interface)

**Priority:** High

**Stimulus/Response:**

| Stimulus | Response |
|----------|----------|
| User selects "1 Player Game" | System displays difficulty selection |
| User selects "Easy" | AI opponent initialized with basic algorithm |
| User places stone | AI calculates and places response within 2s |
| User creates 5 in a row | System detects win, displays result modal |

**AI Algorithms:**

**Easy Mode:**
- Random valid move selection
- Basic blocking (if opponent has 4 in a row, block)
- Renju rule compliance

**Hard Mode:**
- Minimax algorithm with alpha-beta pruning (depth 3-4)
- Position evaluation heuristics:
  - Threat detection (4-in-a-row, 3-in-a-row)
  - Pattern recognition (open 3, broken 4)
  - Center control bonus
- Opening book (first 3 moves)
- Renju rule compliance

### 7.2 Feature: Multiplayer Network Game

**Description:** Two players compete over network

**Functional Requirements:**
- FR-GM-02 (Multiplayer Mode)
- FR-NC-01 to FR-NC-05 (Network Communication)
- FR-GL-01 to FR-GL-05 (Game Logic)
- FR-UI-01 to FR-UI-07 (User Interface)

**Priority:** High

**Stimulus/Response:**

| Stimulus | Response |
|----------|----------|
| Host selects "2 Player Game (LAN)" | System creates server, displays IP |
| Client enters host IP | System attempts connection |
| Connection successful | Game starts, host plays black |
| Player places stone | Stone broadcasted to opponent |
| Network disconnection | System displays error, ends game |

**Network Architecture:**

```
┌─────────────┐         ┌─────────────┐
│   Host      │◄───────►│   Client    │
│  (Player 1) │  TCP/IP │  (Player 2) │
│   Server    │  7773   │             │
└──────┬──────┘         └─────────────┘
       │
       │  TCP/IP
       │
┌──────▼──────┐
│ Spectator 1 │
└─────────────┘
```

**Host Responsibilities:**
- Validate all moves (authoritative server)
- Broadcast game state to all clients
- Manage spectator list
- Handle disconnections

**Client Responsibilities:**
- Send move requests to host
- Receive game state updates
- Display synchronized board

### 7.3 Feature: Spectator Mode

**Description:** Watch live games without participating

**Functional Requirements:**
- FR-GM-03 (Spectator Mode)
- FR-NC-04 (Spectator Support)

**Priority:** Medium

**Stimulus/Response:**

| Stimulus | Response |
|----------|----------|
| User enters spectator mode | System prompts for host IP |
| User enters valid game IP | Connection established |
| Game state update | Spectator UI refreshes |
| Player makes move | Move displayed to spectator |
| Game ends | System shows result, returns to menu |

**Spectator Limitations:**
- Cannot place stones
- Cannot send chat messages (read-only chat)
- Cannot request undo
- No cursor displayed

### 7.4 Feature: Game Logging & Replay

**Description:** Record and playback games

**Functional Requirements:**
- FR-LR-01 (Automatic Logging)
- FR-LR-02 (Replay Playback)

**Priority:** Medium

**Log File Format:**

```
# Gomoku Game Log
# Date: 2025-11-11 15:30:00
# Players: Player1 (Black) vs Player2 (White)
# Result: Player1 wins

[00:00] GAME_START
[00:05] MOVE Black A05
[00:12] MOVE White B06
[00:18] MOVE Black C07
...
[05:23] WIN Black (5 in a row: C07-G11)
[05:23] GAME_END
```

**Replay Controls:**
- Space: Next move
- Backspace: Previous move
- Enter: Play/Pause auto-play
- `/quit`: Exit replay

### 7.5 Feature: In-Game Chat System

**Description:** Text communication during games

**Functional Requirements:**
- FR-UI-03 (Chat System)
- FR-NC-03 (Chat Transmission)

**Priority:** Medium

**Commands:**

| Command | Description | Timeout |
|---------|-------------|---------|
| `/quit` | Exit game | Immediate |
| `/undo` | Request move takeback | 10s |
| `/giveup` | Forfeit game | Confirmation modal |

**Chat UI:**

```
┌───────────────────────────────┐
│                               │
│    ┌──────────────┐  ─── YOU │
│    │ Hello!       │  00:05    │
│    └──────────────┘           │
│                               │
│  ── ┌──────────────┐          │
│  P2 │ Hi there!    │  00:07   │
│  ── └──────────────┘          │
│                               │
│  ┌────────────────────────┐  │
│  │ Enter to chat...  0/15 │  │
│  └────────────────────────┘  │
└───────────────────────────────┘
```

### 7.6 Feature: Renju & Swap Rules

**Description:** Advanced competitive rules

**Functional Requirements:**
- FR-GL-02 (Renju Rules)
- FR-GL-03 (Swap Rule)

**Priority:** High

**Renju Patterns to Detect:**

**3-3 Pattern (Forbidden for Black):**
```
  · · · · ·
  · ● · ● ·
  · · [●] · ·   ← Placing stone here creates two open-3s
  · ● · ● ·
  · · · · ·
```

**4-4 Pattern (Forbidden for Black):**
```
  · · ● ● ● ·
  · · · · · ·
  · · · [●] · ·   ← Placing stone here creates two open-4s
  · · · · · ·
  · ● ● ● · ·
```

**Overline (Forbidden for Black):**
```
  ● ● ● ● ● [●]   ← 6 in a row (invalid for black)
```

**Swap Rule Flow:**

1. Black places first stone (Black1)
2. White places second stone (White1)
3. Black places third stone (Black2)
4. **Swap Offer:** White can choose to swap colors
5. Game continues

### 7.7 Feature: Multiple Themes

**Description:** Customizable visual appearance

**Functional Requirements:**
- FR-AF-01 (Board Themes)

**Priority:** Low

**Available Themes:**

| Theme | Colors | Description |
|-------|--------|-------------|
| White | Black/White | Classic, high contrast |
| Hacker | Green on Black | Terminal aesthetic |
| Gold | Gold/Brown | Warm tones |
| Skyblue | Blue/Cyan | Cool tones |
| Pink | Pink/Magenta | Soft tones |
| Rainbow | Multi-color | Colorful, animated |

**Theme Application:**
- Board background
- Stone colors
- UI element colors
- Text highlighting

### 7.8 Feature: Gamepad Input

**Description:** Alternative control method

**Functional Requirements:**
- FR-AF-02 (Gamepad Support)

**Priority:** Low

**Button Mapping:**

| Gamepad | Function | Keyboard Equivalent |
|---------|----------|---------------------|
| D-Pad / Left Stick | Move cursor | Arrow keys |
| A Button | Place stone | Spacebar |
| B Button | Cancel / Back | Escape |
| Start | Open chat | Enter / T |
| Select | Menu | M |

**Gamepad Detection:**
- Automatic detection via `/dev/input/js0`
- Fallback to keyboard if not detected
- Hot-plug support (connect during gameplay)

---

## 8. Implementation Details

### 8.1 System Calls

**Required System Calls (5+ minimum):**

**Network Communication:**
1. `socket()` - Create communication endpoint
2. `bind()` - Bind socket to address
3. `listen()` - Listen for connections
4. `accept()` - Accept incoming connection
5. `connect()` - Connect to remote socket
6. `send()` / `sendto()` - Send data
7. `recv()` / `recvfrom()` - Receive data

**File I/O:**
8. `open()` - Open file
9. `close()` - Close file descriptor
10. `read()` - Read from file
11. `write()` - Write to file
12. `lseek()` - Reposition file offset
13. `stat()` / `fstat()` - Get file status

**Process/Thread Management:**
14. `fork()` - Create child process (optional)
15. `wait()` / `waitpid()` - Wait for process (optional)
16. `kill()` - Send signal to process

**Signal Handling:**
17. `signal()` - Set signal handler
18. `sigaction()` - Examine/change signal action

**Time Management:**
19. `gettimeofday()` - Get current time
20. `time()` - Get time in seconds

**Terminal Control:**
21. `ioctl()` - Device-specific I/O control

### 8.2 Data Structures

**Core Data Structures:**

```c
// Game Board State
typedef struct {
    int board[15][15];           // 0: empty, 1: black, 2: white
    int current_player;          // 1: black, 2: white
    int cursor_x;                // Current cursor X position
    int cursor_y;                // Current cursor Y position
    int last_move_x;             // Last move X coordinate
    int last_move_y;             // Last move Y coordinate
    int move_count;              // Total moves made
    bool game_over;              // Game end flag
    int winner;                  // 0: none, 1: black, 2: white, 3: draw
} GameBoard;

// Game State (Application Layer)
typedef struct {
    GameBoard* board;            // Board reference
    int game_time;               // Elapsed time (seconds)
    int turn_time_left;          // Remaining turn time (seconds)
    char player1_name[9];        // Player 1 name (max 8 chars + null)
    char player2_name[9];        // Player 2 name
    int spectator_count;         // Number of spectators
    int network_latency;         // Ping in milliseconds
    GameMode mode;               // SINGLE, MULTI_HOST, MULTI_CLIENT, SPECTATOR, REPLAY
} GameState;

// Network Message (protobuf definition)
message GameMessage {
    enum MessageType {
        MOVE = 0;
        CHAT = 1;
        SYSTEM = 2;
        UNDO_REQUEST = 3;
        UNDO_RESPONSE = 4;
        FORFEIT = 5;
        JOIN = 6;
        SPECTATE = 7;
    }
    MessageType type = 1;
    string player_name = 2;
    int32 x = 3;
    int32 y = 4;
    string message = 5;
    bool accepted = 6;
    int64 timestamp = 7;
}

// Chat Message
typedef struct {
    char sender_name[9];         // Sender name
    char message[16];            // Message text (max 15 chars + null)
    time_t timestamp;            // Message timestamp
    bool is_system;              // System message flag
} ChatMessage;

// Log Entry
typedef struct {
    time_t timestamp;            // Event time
    LogEventType type;           // MOVE, JOIN, FORFEIT, etc.
    int x, y;                    // Move coordinates (if applicable)
    int player;                  // Player number
    char description[64];        // Event description
} LogEntry;
```

### 8.3 Algorithms

**8.3.1 Win Detection Algorithm**

```c
// Check for 5 in a row in all directions
bool check_win(GameBoard* board, int x, int y) {
    int player = board->board[x][y];
    if (player == 0) return false;

    // Directions: horizontal, vertical, diagonal-\, diagonal-/
    int dx[] = {1, 0, 1, 1};
    int dy[] = {0, 1, 1, -1};

    for (int dir = 0; dir < 4; dir++) {
        int count = 1;  // Current stone

        // Count in positive direction
        for (int i = 1; i < 5; i++) {
            int nx = x + dx[dir] * i;
            int ny = y + dy[dir] * i;
            if (!in_bounds(nx, ny) || board->board[nx][ny] != player)
                break;
            count++;
        }

        // Count in negative direction
        for (int i = 1; i < 5; i++) {
            int nx = x - dx[dir] * i;
            int ny = y - dy[dir] * i;
            if (!in_bounds(nx, ny) || board->board[nx][ny] != player)
                break;
            count++;
        }

        // Check for exactly 5 (not 6+)
        if (count == 5) return true;
    }

    return false;
}
```

**8.3.2 Renju Rule Validation**

```c
// Check if move is legal under Renju rules
bool is_renju_legal(GameBoard* board, int x, int y) {
    // White stones are not restricted
    if (board->current_player != BLACK) return true;

    // Check 3-3, 4-4, and overline patterns
    if (has_double_three(board, x, y)) return false;
    if (has_double_four(board, x, y)) return false;
    if (has_overline(board, x, y)) return false;

    return true;
}
```

**8.3.3 AI Move Selection (Hard Mode)**

```c
// Minimax with alpha-beta pruning
int minimax(GameBoard* board, int depth, int alpha, int beta, bool is_max) {
    if (depth == 0 || is_terminal(board)) {
        return evaluate_position(board);
    }

    if (is_max) {
        int max_eval = INT_MIN;
        for (each valid move) {
            make_move(board, move);
            int eval = minimax(board, depth - 1, alpha, beta, false);
            undo_move(board, move);
            max_eval = max(max_eval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break;  // Prune
        }
        return max_eval;
    } else {
        int min_eval = INT_MAX;
        for (each valid move) {
            make_move(board, move);
            int eval = minimax(board, depth - 1, alpha, beta, true);
            undo_move(board, move);
            min_eval = min(min_eval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break;  // Prune
        }
        return min_eval;
    }
}

// Position evaluation heuristic
int evaluate_position(GameBoard* board) {
    int score = 0;

    // Evaluate all positions
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board->board[x][y] == AI_PLAYER) {
                score += evaluate_threats(board, x, y, AI_PLAYER);
                score += evaluate_patterns(board, x, y, AI_PLAYER);
            } else if (board->board[x][y] == OPPONENT_PLAYER) {
                score -= evaluate_threats(board, x, y, OPPONENT_PLAYER);
                score -= evaluate_patterns(board, x, y, OPPONENT_PLAYER);
            }
        }
    }

    return score;
}
```

### 8.4 Multi-Threading Architecture

**Thread Model:**

```
┌───────────────────────────────────────────────┐
│             Main Thread                        │
│  - Game logic                                  │
│  - UI rendering (ncurses)                      │
│  - Input handling                              │
└──────────┬────────────────────────────────────┘
           │
           ├──────► Network Receive Thread
           │        - recv() blocking call
           │        - Parse incoming messages
           │        - Update game state (mutex protected)
           │
           ├──────► Timer Thread
           │        - Update turn timer every second
           │        - Update game clock
           │        - Trigger auto-forfeit on timeout
           │
           └──────► Input Thread (optional)
                    - Handle gamepad input
                    - Non-blocking input processing
```

**Synchronization:**

```c
// Shared game state with mutex protection
pthread_mutex_t game_state_mutex = PTHREAD_MUTEX_INITIALIZER;

void update_game_state(GameState* state, /* ... */) {
    pthread_mutex_lock(&game_state_mutex);
    // Modify game state
    pthread_mutex_unlock(&game_state_mutex);
}
```

### 8.5 Network Protocol Details

**Message Flow:**

**1. Connection Establishment:**
```
Client                          Host
  |                              |
  |--- connect() --------------->|
  |<-- accept() -----------------|
  |--- JOIN (player_name) ------>|
  |<-- SYSTEM (welcome) ---------|
  |<-- MOVE (board sync) --------|
  |                              |
```

**2. Move Transmission:**
```
Player 1                        Player 2
  |                              |
  |--- MOVE (x, y) ------------->|
  |<-- MOVE (acknowledged) ------|
  |                              |
  |<-- MOVE (x, y) --------------|
  |--- MOVE (acknowledged) ----->|
  |                              |
```

**3. Chat Message:**
```
Player                          Host
  |                              |
  |--- CHAT (message) ---------->|
  |                              |--- CHAT (broadcast) ---> All clients
  |<-- CHAT (echo) --------------|
  |                              |
```

**4. Undo Request:**
```
Player 1                        Player 2
  |                              |
  |--- UNDO_REQUEST ------------>|
  |                              |<-- Modal displayed
  |                              |    (10s timeout)
  |<-- UNDO_RESPONSE (accepted) -|
  |--- MOVE (undo applied) ----->|
  |                              |
```

### 8.6 File I/O Specifications

**Log File Structure:**

```
# Gomoku Game Log v1.0
# Generated by Gomoku.C
[METADATA]
date=2025-11-11T15:30:00Z
player1=Alice (Black)
player2=Bob (White)
mode=multiplayer
result=player1_win
duration=300

[MOVES]
00:05,BLACK,7,7
00:12,WHITE,8,8
00:18,BLACK,7,8
...

[EVENTS]
00:02,GAME_START
00:45,PLAYER2_JOINED
05:00,WIN_DETECTED,player1,(7,7)-(11,11)
05:00,GAME_END
```

**Configuration File (Optional):**

```ini
[display]
theme=white
terminal_size_check=true

[network]
default_port=7773
connection_timeout=30

[gameplay]
turn_time_limit=20
enable_renju=true
enable_swap=true

[ai]
default_difficulty=easy
```

---

## 9. Development Plan

### 9.1 Timeline Overview

**Total Duration:** 6 weeks (November 12 - December 21)

| Phase | Duration | Deliverables |
|-------|----------|-------------|
| **Phase 1:** Core Architecture | Week 1-2 | Basic game logic, UI framework |
| **Phase 2:** Network & AI | Week 3-4 | Multiplayer, AI opponents |
| **Phase 3:** Polish & Features | Week 5-6 | UI polish, additional features |
| **Presentation** | Week 6 | Final demo, slides |
| **Code Submission** | Week 6+11 days | Complete source code, README |

### 9.2 Phase 1: Core Architecture (Weeks 1-2)

**Week 1: Foundation**
- [ ] Project structure setup (directories, Makefile)
- [ ] Core data structures (GameBoard, GameState)
- [ ] Basic ncurses UI initialization
- [ ] Board rendering (15×15 grid)
- [ ] Cursor movement (arrow keys)
- [ ] Stone placement (spacebar)

**Week 2: Game Logic**
- [ ] Win detection algorithm (5 in a row)
- [ ] Move validation (bounds, occupied positions)
- [ ] Turn management
- [ ] Basic timer implementation
- [ ] Game end detection and modal
- [ ] Single-player local mode (2 human players, no AI)

**Deliverables:**
- Playable 2-player local game
- Clean architecture layers defined
- Basic UI components functional

### 9.3 Phase 2: Network & AI (Weeks 3-4)

**Week 3: Network Communication**
- [ ] TCP socket implementation (socket, bind, listen, accept)
- [ ] protobuf-c integration
- [ ] Message serialization/deserialization
- [ ] Host/Client modes
- [ ] Basic game state synchronization
- [ ] Connection handling (connect, disconnect)
- [ ] Multi-threading (network receive thread)

**Week 4: AI & Advanced Logic**
- [ ] AI Engine: Easy mode (random + basic blocking)
- [ ] AI Engine: Hard mode (minimax algorithm)
- [ ] Renju rule implementation (3-3, 4-4, overline detection)
- [ ] Swap rule implementation
- [ ] Spectator mode
- [ ] Chat system (basic)

**Deliverables:**
- Functional multiplayer over network
- Working AI opponents (Easy & Hard)
- Renju/Swap rules enforced

### 9.4 Phase 3: Polish & Features (Weeks 5-6)

**Week 5: UI Enhancement**
- [ ] Chat UI polish (speech bubbles, timestamps)
- [ ] Command system (/quit, /undo, /giveup)
- [ ] Command auto-completion
- [ ] Modal dialogs (undo request, game end)
- [ ] Theme system (multiple color schemes)
- [ ] Timer visual improvements (progress bar)
- [ ] Information panel (opponent, spectators, ping)

**Week 6: Final Features & Testing**
- [ ] Game logging system
- [ ] Replay mode (log playback)
- [ ] Gamepad input support
- [ ] Command-line arguments
- [ ] Terminal size validation
- [ ] Comprehensive error handling
- [ ] Performance optimization
- [ ] Bug fixes
- [ ] Documentation (README, code comments)
- [ ] Final testing

**Deliverables:**
- Complete, polished application
- All features implemented
- Documentation complete
- Presentation ready

### 9.5 Milestones

**M1: Playable Local Game (End of Week 2)**
- Criteria: 2 human players can play Gomoku locally with win detection

**M2: Network Multiplayer (End of Week 3)**
- Criteria: 2 players on different machines can play over LAN

**M3: AI & Rules (End of Week 4)**
- Criteria: AI opponents work, Renju rules enforced

**M4: Feature Complete (End of Week 5)**
- Criteria: All planned features implemented

**M5: Final Release (End of Week 6)**
- Criteria: Polished, tested, documented, presentation-ready

### 9.6 Risk Management

**Risk Assessment:**

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Network synchronization bugs | Medium | High | Extensive testing, robust error handling |
| AI performance issues | Low | Medium | Optimize algorithms, reduce search depth |
| ncurses rendering glitches | Medium | Medium | Test on multiple terminals, fallback options |
| Renju rule complexity | Medium | Medium | Study existing implementations, unit tests |
| Timeline slippage | Medium | High | Prioritize core features, cut low-priority items |
| Protobuf-c integration issues | Low | Medium | Early integration, fallback to custom protocol |

**Contingency Plans:**

1. **If network sync is problematic:**
   - Simplify protocol
   - Use JSON instead of protobuf (easier debugging)
   - Implement basic client-side prediction

2. **If AI is too slow:**
   - Reduce search depth
   - Implement iterative deepening
   - Use opening book to skip early moves

3. **If timeline is tight:**
   - Cut: Themes, gamepad support, swap rule
   - Keep: Core gameplay, multiplayer, AI, logging

---

## 10. Appendix

### 10.1 Glossary

**Gomoku:** A traditional East Asian board game where two players alternate placing black and white stones on a board, aiming to create an unbroken line of five stones.

**Renju:** A professional variant of Gomoku with additional restrictions on black stones to balance the first-move advantage.

**3-3 (San-San):** A forbidden pattern where black creates two open three-in-a-rows simultaneously.

**4-4 (Yon-Yon):** A forbidden pattern where black creates two open four-in-a-rows simultaneously.

**Overline:** Six or more stones in a row, which is a forbidden pattern for black in Renju.

**Swap Rule:** After the first three moves, the white player may choose to exchange colors with black.

**Clean Architecture:** A software design philosophy that emphasizes separation of concerns through layered architecture.

**protobuf-c:** A C implementation of Google's Protocol Buffers, a language-neutral data serialization format.

**ncurses:** A programming library providing an API for text-based user interfaces.

**pthread:** POSIX Threads, a standardized threading API.

### 10.2 Course Compliance Matrix

**ELEC462 Requirements Compliance:**

| Requirement | Implementation | Status |
|-------------|----------------|--------|
| **Minimum 500 lines of C code** | Estimated 2000+ lines across all modules | ✓ Exceeds |
| **5+ system calls** | 20+ system calls (socket, open, read, etc.) | ✓ Exceeds |
| **3+ core functionalities** | 7 core functionalities (Single/Multi/Spectator/Replay/AI/Network/Logging) | ✓ Exceeds |
| **System programming related** | Network sockets, threading, file I/O, system calls | ✓ Compliant |
| **Bigger than labs** | Comprehensive multiplayer game with AI and networking | ✓ Compliant |
| **README & Makefile** | Complete documentation and build system | ✓ Planned |
| **GitHub submission** | Repository with all source code | ✓ Planned |

### 10.3 Evaluation Criteria Alignment

**Final Presentation and Code (200 pts):**

1. **Code Requirements (100 pts):**
   - Minimum 500 lines: **~2000+ lines** ✓
   - 5+ system calls: **20+ system calls** ✓
   - 3+ core functionalities: **7 functionalities** ✓
   - **Expected Score: 100/100**

2. **Documentation (20 pts):**
   - README: Comprehensive build/run instructions ✓
   - Makefile: Automated build system ✓
   - Code comments: Inline documentation ✓
   - **Expected Score: 20/20**

3. **Complexity (30 pts):**
   - 5+ core functionalities: **7 functionalities** ✓
   - Advanced features: **TCP sockets, pthread, protobuf-c** ✓
   - **Expected Score: 30/30**

4. **Creativity (30 pts):**
   - Unique approach: Clean Architecture, comprehensive UI ✓
   - Beyond basic game: Themes, gamepad, advanced rules ✓
   - Polish: High-quality terminal UI, error handling ✓
   - **Expected Score: 25-30/30**

5. **Presentation (20 pts):**
   - Live demo: Working multiplayer game ✓
   - Slide content: Clear explanation of features ✓
   - **Expected Score: 18-20/20**

**Estimated Total: 193-200/200 points**

### 10.4 References & Resources

**Technical References:**

1. **Socket Programming:**
   - Beej's Guide to Network Programming
   - UNIX Network Programming (Stevens)

2. **ncurses:**
   - ncurses Programming HOWTO
   - NCURSES Programming Guide

3. **Protocol Buffers:**
   - Protocol Buffers Documentation (Google)
   - protobuf-c GitHub Repository

4. **Clean Architecture:**
   - Clean Architecture (Robert C. Martin)
   - Hexagonal Architecture Pattern

5. **Gomoku Strategy:**
   - Renju International Federation Rules
   - Gomoku AI Algorithms Research Papers

**Development Tools:**

- GCC 9.0+
- GNU Make
- CMake
- GDB (debugging)
- Valgrind (memory leak detection)
- Git (version control)

### 10.5 UI Mockups

See separate files in `.ui-sample/` directory:
- `intro.txt` - Main menu screen
- `ingame.txt` - Game play screen
- `modal.txt` - Modal dialog template
- `confirm.txt` - Confirmation dialog
- `logo.txt` - ASCII logo art
- `timebar.txt` - Timer progress bar designs
- `letters.txt` - Text styling samples

### 10.6 Contact Information

**Team:** 임대자들

**Members:**
- 권선우 (컴퓨터학부 플랫폼SW)
- 장기원 (컴퓨터학부 플랫폼SW)

**Repository:** https://github.com/whitedev7773/gomoku-c

**Course:** ELEC462 System Programming
**Institution:** Kyungpook National University
**Instructor:** Prof. Myeonggyun Han

---

**End of SRS Document**

**Document Version:** 1.0
**Last Updated:** 2025-11-11
**Status:** Final for Implementation
