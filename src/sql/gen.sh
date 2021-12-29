#!/bin/bash

lex -i scanner.lex
if test -e "lex.yy.cpp"
then rm lex.yy.cpp
fi
mv lex.yy.c lex.yy.cpp

yacc parser.yy -d
if test -e "y.tab.cpp"
then rm y.tab.cpp
fi
mv y.tab.c y.tab.cpp