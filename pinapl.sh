#!/bin/bash

bin/pinapl $1 > tmp.s
as tmp.s -o tmp.o
ld tmp.o -o $2
rm tmp.o tmp.s

