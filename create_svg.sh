#! /bin/bash -ex

make -j 3

rm pattern-graphs*/*

./inferno.exe -gp pattern-graphs/
./inferno.exe -gtp pattern-graphs-trace/

FILES=test/examples/*.cpp
STEP=p
for FILE in $FILES
do
    CASE=`basename ${FILE} .cpp`
    ./inferno.exe -i test/examples/${CASE}.cpp -q${STEP} -gti > intermediate-graphs-trace/${CASE}_${STEP}.dot
    ./inferno.exe -i test/examples/${CASE}.cpp -q${STEP} -gi > intermediate-graphs/${CASE}_${STEP}.dot
done

# Nuclear dot to svg converter - attempts every dot file under . recursively, 
# and deletes the dot files after successful conversion.
find . -depth -name "*.dot" -exec sh -c 'dot -T svg < "$1" > "${1%.dot}.svg"' _ {} \;

