#!/bin/bash

CC=gcc
ASM=as
LINKER=ld

BIN_DIR=bin/

C_SRC="main memory string parser"
S_SRC="start syscall"

function add_prefix_and_suffix
{
    STRING=$1
    PREFIX=$2
    SUFFIX=$3
    echo "$PREFIX${STRING// /$SUFFIX $PREFIX}$SUFFIX"
}

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

C_FILES=$(add_prefix_and_suffix "$C_SRC" "src/" ".c")
S_FILES=$(add_prefix_and_suffix "$S_SRC" "src/" ".s")
O_FILES=$(add_prefix_and_suffix "$S_SRC $C_SRC" "$BIN_DIR" ".o")

mkdir -p $BIN_DIR

for file in $S_SRC
do
    source_path=$(add_prefix_and_suffix "$file" "src/" ".s")
    object_path=$(add_prefix_and_suffix "$file" "$BIN_DIR" ".o")
    compile_assembly $source_path $object_path "-ggdb"
done

for file in $C_SRC
do
    source_path=$(add_prefix_and_suffix $file src/ .c)
    object_path=$(add_prefix_and_suffix $file $BIN_DIR .o)
    compile_c $source_path $object_path "-c -ggdb -nostdlib -Wall -Werror -std=c11 -Isrc/"
done

$LINKER $O_FILES -o $BIN_DIR/main
handle_errors

chmod +w $BIN_DIR/main

