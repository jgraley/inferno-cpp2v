#! /bin/bash -e

#
# Use Beyond Compare to show a 3-way diff for when, after a srtest run,
# the result cpp diverges from the reference cpp after a certain step.
# You will need beyond compare 4 pro for this.
#
# Run as bc/show_divergent_sr_step.sh <test base name> <before step> <after step> [<option>]
# <before step> is 0-padded step number from before the divergence
# <after step> is 0-padded step number from after the divergence
# <option> is eg -uk
#
 
NAME=$1
BEFORE=$2
AFTER=$3
OPTION=$4

CAT=sr
EXT=cpp

REFERENCE=test/reference
RESULTS=test/results${OPTION}

LEFT=${REFERENCE}/${CAT}/${NAME}/${NAME}_${AFTER}.${EXT}
RIGHT=${RESULTS}/${CAT}/${NAME}/${NAME}_${AFTER}.${EXT}
CENTRE=${REFERENCE}/${CAT}/${NAME}/${NAME}_${BEFORE}.${EXT}
CHECK=${RESULTS}/${CAT}/${NAME}/${NAME}_${BEFORE}.${EXT}

# 3-way diff will be misleading if ref and result diverged before <before step>
diff -q ${CENTRE} ${CHECK}

# Start bc and return immediately
bcompare ${LEFT} ${RIGHT} ${CENTRE} &
