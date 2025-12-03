#!/bin/bash

# Set UTF-8 locale for proper Unicode character display
export LANG=ko_KR.UTF-8
export LC_ALL=ko_KR.UTF-8

# Change to build directory
cd "$(dirname "$0")/build"

# Run the game
./gomoku-c "$@"
