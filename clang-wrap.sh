#!/bin/bash

# clang wrapper that doesn't allow unsupported options

options=()  # the buffer array for the parameters

while [[ $1 ]]
do
	case "$1" in
	  -fno-tree-vrp|-fno-tree-sink)
	      shift
	      ;;
	  *)
	      options+=("$1")
	      shift
	      ;;
	esac
done

"${options[@]}"
