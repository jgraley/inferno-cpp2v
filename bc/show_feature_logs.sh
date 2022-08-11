#! /bin/bash -e

#
# Use Beyond Compare to show a the effect of a feature flag. Only one 
# feature flag supported currently.
# You will need beyond compare 4 pro for this.
#
# Run as bc/show_feature_logs.sh <feature flag> <other args>
# <feature flag> is eg -uo
# <other args> can be multiple args: supplied to all invocations of inferno.exe
#
 
FFLAG=$1
shift
OFLAGS=$*

EXE=./inferno.exe
LDIR=/tmp
LOGL=${LDIR}/log.txt
LOGR=${LDIR}/log${FFLAG}.txt
make -j 24

${EXE} ${OFLAGS} 2> ${LOGL} &
${EXE} ${FFLAG} ${OFLAGS} 2> ${LOGR} &

wait

# Start bc and return immediately
bcompare ${LOGL} ${LOGR} &
