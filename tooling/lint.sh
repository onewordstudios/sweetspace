#!/bin/bash

if [ $# == 0 ]
then
    echo ''
    echo '    To run the linter on a single file, pass in the name of the cpp file as the argument to this script (without the .cpp)'
    echo '    For example, run:         ./lint.sh GLaDOS'
    echo '    Pass the flag "-all" as an argument to run on all source files (will take a while)'
	echo '    The flag "-ci" is used by the CI pipeline; please ignore it'
    echo ''
elif [ $1 == '-all' ]
then
    clang-tidy ../source/*.cpp -- -I../cugl/include
elif [ $1 == '-ci' ]
then
	clang-tidy ../source/*.cpp -- -I../cugl/include 2>&1 >/dev/null | grep -v "warnings generated." >&2
else
    clang-tidy ../source/$1.cpp -- -I../cugl/include
fi
