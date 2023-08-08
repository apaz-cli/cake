#!/bin/sh

while true
do
    csmith > spew.c
    src/cake spew.c
    if [ ! "$?" = "0" ]; then
        creduce interestingness.sh spew.c
    fi
done


#usage: creduce [options] interestingness_test file_to_reduce [optionally, more files to reduce]
#       creduce --help for more information
