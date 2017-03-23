

# This script deletes all META files recursively within 
# Assets folder and completely removes Library folder

# You can use it in Git Bash doing:
# ./resetAssets.sh

# Finding META files and deleting them
find Game/Assets/ -name '*.meta' | xargs rm -f
# Wipping out the entire Library folder
rm -fr Game/Library

