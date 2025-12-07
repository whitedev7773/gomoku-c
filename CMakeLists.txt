cmake_minimum_required(VERSION 3.10.0)
project(gomoku-c VERSION 0.1.0 LANGUAGES C)

# ncurses with wide character support
find_package(Curses REQUIRED)

# Prefer ncursesw (wide character support) for UTF-8
if(CURSES_HAVE_NCURSESW_NCURSES_H)
    set(CURSES_LIBRARIES ncursesw)
    message(STATUS "Using ncursesw for UTF-8 support")
elseif(EXISTS "/usr/lib/libncursesw.so" OR EXISTS "/usr/lib/x86_64-linux-gnu/libncursesw.so")
    set(CURSES_LIBRARIES ncursesw)
    message(STATUS "Found ncursesw library, using it for UTF-8 support")
else()
    message(WARNING "ncursesw not found, using regular ncurses. UTF-8 characters may not display correctly.")
endif()

# OpenSSL for SHA256 hashing
find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL version: ${OPENSSL_VERSION}")

include_directories(${CURSES_INCLUDE_DIR})

include_directories(include)
include_directories(src)

# Main executable source files (exclude test directory)
file(GLOB SRC_FILES "src/main.c")
file(GLOB_RECURSE UTIL_FILES "src/utils/*.c")
file(GLOB_RECURSE GAME_FILES "src/game/*.c")
file(GLOB_RECURSE UI_FILES "src/ui/*.c")
file(GLOB_RECURSE NETWORK_FILES "src/network/*.c")
file(GLOB_RECURSE SECURITY_FILES "src/security/*.c")

# Core game files without singleplay (for tests that don't need UI)
set(CORE_GAME_FILES
    src/game/core/board.c
    src/game/core/game_logic.c
    src/game/feature/game_logger.c
    src/game/core/turn_manager.c
    src/game/core/rules.c
    src/game/ai/ai_engine.c
)

add_executable(gomoku-c ${SRC_FILES} ${UTIL_FILES} ${GAME_FILES} ${UI_FILES} ${NETWORK_FILES} ${SECURITY_FILES})
target_link_libraries(gomoku-c ${CURSES_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)

# Test executables
file(GLOB TEST_PHASE2_FILES "src/test/test_phase2.c")
if(TEST_PHASE2_FILES)
    add_executable(test_phase2 ${TEST_PHASE2_FILES} ${CORE_GAME_FILES})
    target_link_libraries(test_phase2 ${CURSES_LIBRARIES})
endif()

file(GLOB TEST_PHASE3_FILES "src/test/test_phase3_ui.c")
if(TEST_PHASE3_FILES)
    add_executable(test_phase3_ui ${TEST_PHASE3_FILES} ${UTIL_FILES} ${GAME_FILES} ${UI_FILES} ${NETWORK_FILES})
    target_link_libraries(test_phase3_ui ${CURSES_LIBRARIES})
endif()

file(GLOB TEST_MENU_FILES "src/test/test_menu.c")
if(TEST_MENU_FILES)
    add_executable(test_menu ${TEST_MENU_FILES} ${UTIL_FILES} ${GAME_FILES} ${UI_FILES} ${NETWORK_FILES})
    target_link_libraries(test_menu ${CURSES_LIBRARIES})
endif()

file(GLOB TEST_AI_FILES "src/test/test_ai.c")
if(TEST_AI_FILES)
    add_executable(test_ai ${TEST_AI_FILES} ${CORE_GAME_FILES})
    target_link_libraries(test_ai ${CURSES_LIBRARIES})
endif()

file(GLOB TEST_GAMEPAD_FILES "src/test/test_gamepad.c")
if(TEST_GAMEPAD_FILES)
    add_executable(test_gamepad ${TEST_GAMEPAD_FILES} src/ui/core/gamepad_input.c)
    target_link_libraries(test_gamepad)
endif()

# Unit tests
file(GLOB TEST_FRAMEWORK_FILES "src/test/test_framework.c")
file(GLOB TEST_BOARD_FILES "src/test/test_board.c")
if(TEST_BOARD_FILES AND TEST_FRAMEWORK_FILES)
    add_executable(test_board ${TEST_BOARD_FILES} ${TEST_FRAMEWORK_FILES} ${CORE_GAME_FILES})
    target_link_libraries(test_board)
endif()

file(GLOB TEST_RULES_FILES "src/test/test_rules.c")
if(TEST_RULES_FILES AND TEST_FRAMEWORK_FILES)
    add_executable(test_rules ${TEST_RULES_FILES} ${TEST_FRAMEWORK_FILES} ${CORE_GAME_FILES})
    target_link_libraries(test_rules)
endif()

file(GLOB TEST_INGAME_BORDER_FILES "src/test/test_ingame_border.c")
if(TEST_INGAME_BORDER_FILES)
    add_executable(test_ingame_border ${TEST_INGAME_BORDER_FILES} src/ui/game/border/ingame_border.c src/ui/core/theme.c src/utils/config.c)
    target_link_libraries(test_ingame_border ${CURSES_LIBRARIES})
endif()