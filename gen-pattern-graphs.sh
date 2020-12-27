#! /bin/bash -ex

make -j 3

mkdir -p pattern-graphs/
rm -rf pattern-graphs/*
./inferno.exe -gp pattern-graphs/
find pattern-graphs/ -depth -name "*.dot" -exec sh -c 'dot -T pdf < "$1" > "${1%.dot}.pdf"' _ {} \;
rm -rf pattern-graphs/*.dot

mkdir -p pattern-graphs-trace/
rm -rf pattern-graphs-trace/*
./inferno.exe -gtp pattern-graphs-trace/
find pattern-graphs-trace/ -depth -name "*.dot" -exec sh -c 'dot -T pdf < "$1" > "${1%.dot}.pdf"' _ {} \;
rm -rf pattern-graphs-trace/*.dot
