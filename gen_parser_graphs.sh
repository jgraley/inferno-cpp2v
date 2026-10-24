#!/bin/bash -ex

I=$*
I_NO_SPACES=`echo ${I} | tr -d '[:space:]'`

make -j 32
P1=p1${I_NO_SPACES}
GP1=gp1${I_NO_SPACES}
P2=p2${I_NO_SPACES}
GP2=gp2${I_NO_SPACES}
rm -rf ${P1} ${GP1} ${GP2}

# Gen graphs for built-in patterns
mkdir -p ${GP1} &&
./inferno.exe ${I} -gtp ${GP1}/
./convert_all_dot.sh -d ${GP1}

# Render the built-in patterns 
mkdir -p ${P1} 
./inferno.exe ${I} -p ${P1}/

# Gen graphs for rendered+parsed patterns
mkdir -p ${GP2}
./inferno.exe ${I} ${P1} -gtp ${GP2}/
./convert_all_dot.sh -d ${GP2}
