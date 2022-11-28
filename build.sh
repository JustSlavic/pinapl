#!/bin/bash

CC=gcc
ASM=as
LINKER=ld

BIN_DIR=bin

function handle_errors
{
    rc=$?
    if [[ "$rc" -ne 0 ]]
    then
        echo "ERROR (return code $rc)"
        exit $rc
    fi
}

function compile_assembly
{
    SOURCE_FILE=$1
    OUTPUT_FILE=$2
    DEBUG=$3

    $ASM $SOURCE_FILE $DEBUG -o $OUTPUT_FILE
    handle_errors
}

function compile_c
{
    SOURCE_FILE=$1
    OUTPUT_FILE=$2
    OPTIONS=$3

    $CC $SOURCE_FILE $OPTIONS -o $OUTPUT_FILE
    handle_errors
}

mkdir -p $BIN_DIR

compile_assembly "start.s" "$BIN_DIR/start.o" "-ggdb"
compile_assembly "syscall.s" "$BIN_DIR/syscall.o" "-ggdb"

compile_c "memory.c" "$BIN_DIR/memory.o" "-c -ggdb -nostdlib -Wall -Werror -std=c11"
compile_c "main.c" "$BIN_DIR/main.o" "-c -ggdb -nostdlib -Wall -Werror -std=c11"

$LINKER $BIN_DIR/start.o $BIN_DIR/syscall.o $BIN_DIR/main.o $BIN_DIR/memory.o -o $BIN_DIR/main
handle_errors

chmod +w $BIN_DIR/main

