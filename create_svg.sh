#! /bin/bash -ex

# Nuclear dot to svg converter - attempts every dot file under . recursively, 
# and deletes the dot files after successful conversion.

find . -depth -name "*.dot" -exec sh -c 'dot -T svg < "$1" > "${1%.dot}.svg" && rm $1' _ {} \;

