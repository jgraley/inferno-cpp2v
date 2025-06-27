#! /bin/bash -e

# Nuclear dot to svg converter - attempts every dot file under graphs/ or supplied dir recursively, 

DIR="graphs"
export FORMAT="svg"
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -d|--dir)
      DIR="$2"
      shift # past argument
      shift # past arg
      ;;
    -f|--format)
      export FORMAT="$2"
      shift # past argument
      shift # past arg
      ;;
    *)
      echo "Usage: convert_all_dot.sh [-d <dir>] [-f <format>]"
      exit 1
      ;;
  esac
done

find $DIR/ -depth -name "*.dot" -exec sh -c 'echo "$1" && dot -T ${FORMAT} < "$1" > "${1%.dot}.${FORMAT}" && rm "$1"' _ {} \;
