#! /bin/bash -ex

make -j 3

mkdir -p graphs/pattern/
mkdir -p graphs/pattern-trace/
mkdir -p graphs/intermediate/
mkdir -p graphs/intermediate-trace/
rm -rf graphs/*/*

# "k" for dark, "" for light 
COLOUR=""

./inferno.exe -g${COLOUR}p graphs/pattern/
./inferno.exe -gt${COLOUR}p graphs/pattern-trace/

FILES=test/examples/*.cpp
PROGRESS="I" # I means "in", i.e. input has been parsed
for FILE in $FILES
do
    CASE=`basename ${FILE} .cpp`
    ./inferno.exe -i test/examples/${CASE}.cpp -q${PROGRESS} -gt${COLOUR}i > graphs/intermediate-trace/${CASE}_${PROGRESS}.dot
    ./inferno.exe -i test/examples/${CASE}.cpp -q${PROGRESS} -g${COLOUR}i > graphs/intermediate/${CASE}_${PROGRESS}.dot
done

./convert_all_svg.sh
