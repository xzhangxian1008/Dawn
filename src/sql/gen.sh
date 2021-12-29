#!/bin/bash

lex -i scanner.lex
rm lex.yy.cpp
mv lex.yy.c lex.yy.cpp

yacc parser.yy -d
rm y.tab.cpp
mv y.tab.c y.tab.cpp