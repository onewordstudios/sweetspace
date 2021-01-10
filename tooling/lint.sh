#!/bin/bash

if [ $# == 0 ]
then
    echo ''
    echo '    To run the linter on a single file, pass in the name of the cpp file as the argument to this script (without the .cpp)'
    echo '    For example, run:         ./lint.sh ShipModel'
    echo '    Pass the flag "-all" as an argument to run on all source files (will take a while)'
    echo '    The flag "-ci" and "-ciDiff" is used by the CI pipeline; please ignore it'
    echo ''
elif [ $1 == '-all' ]
then
    clang-tidy ../source/*.cpp --header-filter=.*source\/[^\/]*\.h -- -I../cugl/include
elif [ $1 == '-fast' ]
then
    run-clang-tidy.py -header-filter=".*source\/[^\/]*\.h" -cpp ../source/*.cpp
	exit 0
elif [ $1 == '-ci' ]
then
    clang-tidy ../source/*.cpp --header-filter=.*source\/[^\/]*\.h -- -I../cugl/include 1>&2 2>/dev/null
    exit 0
elif [ $1 == '-ciDiff' ]
then
    cd ..
    LIST=$(git diff origin/master --diff-filter=ACMR --name-only --no-color | grep -E '\.cpp$')
    clang-tidy $LIST --header-filter=.*source\/[^\/]*\.h -- -Icugl/include 1>&2 2>/dev/null
elif [ $1 == '-ciDiffFast' ]
then
    cd ..
    LIST=$(git diff origin/master --diff-filter=ACMR --name-only --no-color | grep -E '\.cpp$')
    tooling/run-clang-tidy.py -header-filter=".*source\/[^\/]*\.h" -cpp $LIST 1>&2 2>/dev/null
else
    clang-tidy ../source/$1.cpp --header-filter=.*source\/[^\/]*\.h -- -I../cugl/include
fi
