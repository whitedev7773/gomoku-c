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

include_directories(${CURSES_INCLUDE_DIR})

include_directories(include)

# Main executable source files (exclude test directory)
file(GLOB SRC_FILES "src/main.c")
file(GLOB_RECURSE UTIL_FILES "src/utils/*.c")
file(GLOB_RECURSE GAME_FILES "src/game/*.c")
file(GLOB_RECURSE UI_FILES "src/ui/*.c")
file(GLOB_RECURSE NETWORK_FILES "src/network/*.c")

# Core game files without singleplay (for tests that don't need UI)
set(CORE_GAME_FILES
    src/game/board.c
    src/game/game_logic.c
    src/game/game_logger.c
    src/game/turn_manager.c
    src/game/rules.c
    src/game/ai_engine.c
)

add_executable(gomoku-c ${SRC_FILES} ${UTIL_FILES} ${GAME_FILES} ${UI_FILES} ${NETWORK_FILES})
target_link_libraries(gomoku-c ${CURSES_LIBRARIES})

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