#!/bin/bash -ex

I=$*
I_NO_SPACES=`echo ${I} | tr -d '[:space:]'`

make -j 32
P1=p1${I_NO_SPACES}
P2=p2${I_NO_SPACES}
rm -rf ${P1} ${GP1} ${P2} ${GP2}

# Render the built-in patterns 
mkdir -p ${P1} 
./inferno.exe ${I} -p ${P1}/

# Re-render the parsed patterns 
mkdir -p ${P2}
./inferno.exe ${I} ${P1} -p ${P2}/

# All ensuing tests stop after this step
QA=T130+

# Smoke tests: for one test case and one quit-after, compare the transformed outputs
#TC=small.c
#./inferno.exe ${I} ${P1} -i test/examples/${TC} -q${QA} > out_p1.cpp
#./inferno.exe ${I}       -i test/examples/${TC} -q${QA} > out_builtin.cpp
#diff --color out_builtin.cpp out_p1.cpp

# Full test run using patterns we previously wrote to p1/
make -j 32 test I="${I} ${P1} -q${QA}"
