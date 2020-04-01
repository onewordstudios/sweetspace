#!/bin/bash

if [ $# == 0 ]
then
    echo ''
    echo '    To run the linter on a single file, pass in the name of the cpp file as the argument to this script (without the .cpp)'
    echo '    For example, run:         ./lint.sh GLaDOS'
    echo '    Pass the word "all" as an argument to run on all source files (will take a while)'
    echo ''
elif [ $1 == 'all' ]
then
    clang-tidy ../source/*.cpp -- -I../cugl/include
else
    clang-tidy ../source/$1.cpp -- -I../cugl/include
fi
