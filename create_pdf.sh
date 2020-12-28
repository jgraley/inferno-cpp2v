#! /bin/bash -ex

# Nuclear dot to pdf converter - attempts every dot file under . recursively, 
# and deletes the dot files after successful conversion.

find . -depth -name "*.dot" -exec sh -c 'dot -T pdf < "$1" > "${1%.dot}.pdf" && rm $1' _ {} \;

