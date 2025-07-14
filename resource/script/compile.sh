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

#JSG Due to SystemC bug discussed here: http://www.accellera.org/Discussion_Forums/systemc-forum/archive/msg?list_name=systemc-forum&monthdir=201002&msg=msg00026.html
# I have to use -fpermissive and cannot use -Werror (in $OPTIONS) even though I want to. Apparently a fix is in SystemC 2.3 which is in betaa presently. TODO ensure using SC2.3+ and revert bodge here.

g++ $GCC_EXTRA_ARGS -fpermissive -Iresource/include -Isystemc/include -fPIC 
