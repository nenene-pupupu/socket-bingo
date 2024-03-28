#!/bin/bash

# Check if folder name is provided as the first argument
folder_name="$1"

# Display usage if folder name is not provided
if [ -z "$folder_name" ]; then
  echo "Usage: ./format.sh {folder_name}"
  exit 1
fi

# Check if the folder exists
if [ ! -d "$folder_name" ]; then
  echo "Folder '$folder_name' not found."
  exit 1
fi

# Find .c files and format them
find "$folder_name" -type f -name "*.c" | while read -r file; do
  echo "Formatting: $file"
  clang-format -i "$file"
done

echo "Formatting of all .c files is complete."
