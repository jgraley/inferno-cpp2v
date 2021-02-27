#! /bin/bash -ex

make -j 3

mkdir -p graphs/pattern/
mkdir -p graphs/pattern-trace/
mkdir -p graphs/intermediate/
mkdir -p graphs/intermediate-trace/
rm -rf graphs/*/*

./inferno.exe -gp graphs/pattern/
./inferno.exe -gtp graphs/pattern-trace/

FILES=test/examples/*.cpp
PROGRESS="I" # I means "in", i.e. input has been parsed
for FILE in $FILES
do
    CASE=`basename ${FILE} .cpp`
    ./inferno.exe -i test/examples/${CASE}.cpp -q${PROGRESS} -gti > graphs/intermediate-trace/${CASE}_${PROGRESS}.dot
    ./inferno.exe -i test/examples/${CASE}.cpp -q${PROGRESS} -gi > graphs/intermediate/${CASE}_${PROGRESS}.dot
done

# Nuclear dot to svg converter - attempts every dot file under . recursively, 
# and deletes the dot files after successful conversion.
find graphs -depth -name "*.dot" -exec sh -c 'dot -T svg < "$1" > "${1%.dot}.svg"' _ {} \;

