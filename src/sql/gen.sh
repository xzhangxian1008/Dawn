#!/bin/bash

./lemon parse.y

if test -e "parse.cpp"
then rm parse.cpp
fi
mv parse.c parse.cpp

if test -e "parse.h"
then mv parse.h ../include/sql/parse.h
fi
