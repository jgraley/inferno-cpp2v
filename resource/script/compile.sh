#! /bin/bash -e

GCC_EXTRA_ARGS="-ggdb"
BASE_DIR="graphs"
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --exp-run)
      GCC_EXTRA_ARGS+=" -DEXPECTATION_RUN "
      shift # past argument
      ;;
    -c)
      shift
      GCC_EXTRA_ARGS+=" -c $1 "
      shift # past argument
      ;;
    -o)
      shift
      GCC_EXTRA_ARGS+=" -o $1 "
      shift # past argument
      ;;
    *)
      echo "Usage: compile.sh [--exp_run] [-c <source>] [-o <object>]"
      exit 1
      ;;
  esac
done

g++ $GCC_EXTRA_ARGS -Werror -Iresource/include -Isystemc/include -fPIC 
