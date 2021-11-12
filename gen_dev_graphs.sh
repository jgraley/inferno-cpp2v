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
      echo "Usage: gen_dev_graphs.sh [-p] [-i] [-k] [-u]"
      exit 1
      ;;
  esac
done

rm -rf ${BASE_DIR}/*/*

if [ "$PATTERN" = "1" ]; then
    mkdir -p ${BASE_DIR}/pattern/
    ./inferno.exe -g${COLOUR}p ${BASE_DIR}/pattern/
    mkdir -p ${BASE_DIR}/pattern-trace/
    ./inferno.exe -gt${COLOUR}p ${BASE_DIR}/pattern-trace/
fi

if [ "$INTERMEDIATE" = "1" ]; then
    FILES=test/examples/*.cpp
fi
    
PROGRESS="I" # I means "in", i.e. input has been parsed
for FILE in $FILES
do
    CASE=`basename ${FILE} .cpp`

    mkdir -p ${BASE_DIR}/intermediate/
    ./inferno.exe -i test/examples/${CASE}.cpp -q${PROGRESS} -g${COLOUR}i > ${BASE_DIR}/intermediate/${CASE}_${PROGRESS}.dot
    mkdir -p ${BASE_DIR}/intermediate-trace/
    ./inferno.exe -i test/examples/${CASE}.cpp -q${PROGRESS} -gt${COLOUR}i > ${BASE_DIR}/intermediate-trace/${CASE}_${PROGRESS}.dot
done

./convert_all_dot.sh -d ${BASE_DIR}

echo Generated graphs into ${BASE_DIR}/
