#!/bin/bash

# Set UTF-8 locale for proper Unicode character display
export LANG=ko_KR.UTF-8
export LC_ALL=ko_KR.UTF-8

# Change to build directory
cd "$(dirname "$0")/build"

# Check which test to run
if [ "$1" = "menu" ]; then
    ./test_menu
elif [ "$1" = "phase3" ]; then
    ./test_phase3_ui
elif [ "$1" = "phase2" ]; then
    ./test_phase2
else
    echo "Usage: $0 [menu|phase3|phase2]"
    echo "  menu   - Run menu UI test"
    echo "  phase3 - Run Phase 3 UI test"
    echo "  phase2 - Run Phase 2 game logic test"
fi
