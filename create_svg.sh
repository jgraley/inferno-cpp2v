#! /bin/bash -ex

make -j 3

rm pattern-graphs*/*

./inferno.exe -gp pattern-graphs/
./inferno.exe -gtp pattern-graphs-trace/

# Nuclear dot to svg converter - attempts every dot file under . recursively, 
# and deletes the dot files after successful conversion.
find . -depth -name "*.dot" -exec sh -c 'dot -T svg < "$1" > "${1%.dot}.svg"' _ {} \;

