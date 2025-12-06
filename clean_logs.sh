#!/bin/bash

# Clean all .log files in the project directory

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Find and remove all .log files
find "$SCRIPT_DIR" -name "gomoku-*.log" -type f -print -delete

echo "All .log files have been removed."
