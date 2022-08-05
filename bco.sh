#! /bin/bash -e

# Run as bco.sh <test base name> <before step> <sfter step>
# You will need beyond compare 4 pro for this.

NAME=$1
BEFORE=$2
AFTER=$3

CAT=sr
EXT=cpp

LEFT=test/reference/${CAT}/${NAME}/${NAME}_${AFTER}.${EXT}
RIGHT=test/results/${CAT}/${NAME}/${NAME}_${AFTER}.${EXT}
CENTRE=test/reference/${CAT}/${NAME}/${NAME}_${BEFORE}.${EXT}

bcompare ${LEFT} ${RIGHT} ${CENTRE} &
