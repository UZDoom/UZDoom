#!/bin/bash

# Dependency check
if ! command -v msgfmt &> /dev/null; then
    echo "Error: 'msgfmt' (gettext) is not installed."
    exit 1
fi

error_count=0

while IFS= read -r file; do
    # -c: check format
    # -o /dev/null: discard binary output
    # 2>&1: capture errors
    if output=$(msgfmt -c -o /dev/null "$file" 2>&1); then
        echo "[OK] $file"
    else
        echo "[FAIL] $file"
        echo "$output" | sed 's/^/  /' # Indent the error message
        ((error_count++))
    fi
done < <(find . -type f -name "*.po")

if [ "$error_count" -gt 0 ]; then
    echo ""
    echo "Validation failed for $error_count file(s)."
    exit 1
fi

echo "All files valid."
