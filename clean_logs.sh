#!/bin/bash

# Clean all .omk files in the project directory

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Find and remove all .omk files
find "$SCRIPT_DIR" -name "gomoku-*.omk" -type f -print -delete

echo "All .omk files have been removed."
