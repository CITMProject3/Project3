
# Finding META files and deleting them
find Game/Assets/ -name '*.meta' | xargs rm -f
# Wipping out the entire Library folder
rm -fr Game/Library/
