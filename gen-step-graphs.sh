#! /bin/bash -ex

make -j 3

mkdir -p step-graphs/
rm -rf step-graphs/*
./inferno.exe -gp step-graphs/
find step-graphs/ -depth -name "*.dot" -exec sh -c 'dot -T pdf < "$1" > "${1%.dot}.pdf"' _ {} \;
rm -rf step-graphs/*.dot

mkdir -p step-graphs-trace/
rm -rf step-graphs-trace/*
./inferno.exe -gtp step-graphs-trace/
find step-graphs-trace/ -depth -name "*.dot" -exec sh -c 'dot -T pdf < "$1" > "${1%.dot}.pdf"' _ {} \;
rm -rf step-graphs-trace/*.dot
