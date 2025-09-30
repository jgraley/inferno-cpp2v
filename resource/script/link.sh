#! /bin/bash -e

GCC_EXTRA_ARGS=""
BASE_DIR="graphs"
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -i)
      shift
      GCC_EXTRA_ARGS+="$1 "
      shift # past argument
      ;;
    -o)
      shift
      GCC_EXTRA_ARGS+="-o $1 "
      shift # past argument
      ;;
    *)
      echo "Usage: compile.sh [--exp_run] [-i <object>] [-o <executable>]"
      exit 1
      ;;
  esac
done

g++ $GCC_EXTRA_ARGS resource/lib/systemc_extensions.o -Lsystemc/lib-linux64/ -lsystemc -no-pie
