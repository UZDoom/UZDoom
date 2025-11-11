#!/bin/bash

# ./compile.sh ./gzdoom_engine_strings > gzdoom_engine_strings.csv

ls -1 "$1/"*.po 2>/dev/null | xargs gawk -f "$(dirname "$0")/compile.awk" >"$2"
