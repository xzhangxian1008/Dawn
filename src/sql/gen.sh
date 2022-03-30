#!/bin/bash

if [ ! -d "lemon" ];
then cc -o lemon lemon.c
fi

./lemon parse.y

# Modify the suffix of source file
if test -e "parse.cpp"
then rm parse.cpp
fi
mv parse.c parse.cpp

# Move the header file to the corresponding place
if test -e "parse.h"
then mv parse.h ../include/sql/parse.h
fi

rm parse.out
