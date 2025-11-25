cmake_minimum_required(VERSION 3.10.0)
project(gomoku-c VERSION 0.1.0 LANGUAGES C)

# ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

include_directories(include)

file(GLOB SRC_FILES
    "src/*.c"
    "src/application/*.c"
    "src/application/**/*.c"

    "src/domain/*.c"
    "src/domain/**/*.c"

    "src/infrastructure/*.c"
    "src/infrastructure/**/*.c"

    "src/presentation/*.c"
    "src/presentation/**/*.c"
)

add_executable(gomoku-c ${SRC_FILES})
target_link_libraries(gomoku-c ${CURSES_LIBRARIES})