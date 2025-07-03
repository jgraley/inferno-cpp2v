#!/bin/bash -ex

INTERMEDIATE=0
PATTERN=0
COLOUR=""
BASE_DIR="graphs"
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -p|--pattern)
      PATTERN=1
      shift # past argument
      ;;
    -d|--docs)
      DOCS=1
      shift # past argument
      ;;
    -i|--intermediate)
      INTERMEDIATE=1
      shift # past argument
      ;;
    -k|--dark)
      COLOUR="k"
      shift # past argument
      ;;
    -u|--update)
      BASE_DIR="test/reference/graphs"
      shift # past argument
      ;;
    *)
      echo "Usage: gen_dev_graphs.sh [-p] [-d] [-i] [-k] [-u]"
      exit 1
      ;;
  esac
done

if [ "$DOCS" == "1" ]; then
    mkdir -p ${BASE_DIR}/docs/
	rm -rf ${BASE_DIR}/docs/*
    mkdir -p ${BASE_DIR}/docs-trace/
	rm -rf ${BASE_DIR}/docs-trace/*
    ./inferno.exe -g${COLOUR}d ${BASE_DIR}/docs/ &
    ./inferno.exe -gt${COLOUR}d ${BASE_DIR}/docs-trace/ &
    wait
fi

if [ "$PATTERN" == "1" ]; then
    mkdir -p ${BASE_DIR}/pattern/
	rm -rf ${BASE_DIR}/pattern/*
    mkdir -p ${BASE_DIR}/pattern-trace/
	rm -rf ${BASE_DIR}/pattern-trace/*
    ./inferno.exe -g${COLOUR}p ${BASE_DIR}/pattern/ &
    ./inferno.exe -gt${COLOUR}p ${BASE_DIR}/pattern-trace/ &
    wait
fi

if [ "$INTERMEDIATE" == "1" ]; then
    FILES="test/examples/*.cpp test/examples/*.c"
   
	# #757 to fix test04.cpp
	SKIPS=("test04.cpp") 
		
	STEP_NUMBERS=($(seq 1 104)) # TODO obtain this from inferno.exe

	# I means "in", i.e. input has been parsed; T means transformation
	PROGRESSES="I ${STEP_NUMBERS[@]/#/T}"
	echo ${PROGRESSES}
	for FILE in $FILES
	do
		CASE=`basename ${FILE}`
		if [[ $(echo ${SKIPS[@]} | fgrep -w ${CASE}) ]]; then
			continue
		fi
		
		mkdir -p ${BASE_DIR}/intermediate/${CASE}/
		rm -rf ${BASE_DIR}/intermediate/${CASE}/*
		mkdir -p ${BASE_DIR}/intermediate-trace/${CASE}/
		rm -rf ${BASE_DIR}/intermediate-trace/${CASE}/*
		for PROGRESS in ${PROGRESSES}; do
			./inferno.exe -i ${FILE} -q${PROGRESS} -gt${COLOUR}i > ${BASE_DIR}/intermediate-trace/${CASE}/${PROGRESS}.dot &
			./inferno.exe -i ${FILE} -q${PROGRESS} -g${COLOUR}i > ${BASE_DIR}/intermediate/${CASE}/${PROGRESS}.dot &
			wait
		done
	done
fi

./convert_all_dot.sh -d ${BASE_DIR}

echo Generated graphs into ${BASE_DIR}/
