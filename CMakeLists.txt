cmake_minimum_required(VERSION 3.10.0)
project(gomoku-c VERSION 0.1.0 LANGUAGES C)

# ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

include_directories(include)

# Main executable source files (exclude test directory)
file(GLOB SRC_FILES "src/main.c")
file(GLOB_RECURSE UTIL_FILES "src/utils/*.c")
file(GLOB_RECURSE GAME_FILES "src/game/*.c")
file(GLOB_RECURSE UI_FILES "src/ui/*.c")
file(GLOB_RECURSE NETWORK_FILES "src/network/*.c")

add_executable(gomoku-c ${SRC_FILES} ${UTIL_FILES} ${GAME_FILES} ${UI_FILES} ${NETWORK_FILES})
target_link_libraries(gomoku-c ${CURSES_LIBRARIES})

# Test executable
file(GLOB TEST_FILES "src/test/*.c")
if(TEST_FILES)
    add_executable(test_phase2 ${TEST_FILES} ${GAME_FILES})
    target_link_libraries(test_phase2 ${CURSES_LIBRARIES})
endif()