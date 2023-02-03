#!/bin/bash

PROJECT=pinapl

CC=gcc
ASM=as
LINKER=gcc

C_STD=c11

BIN_DIR=bin/

CRT="-DCRT=1"
WARNINGS="-Wall -Werror"
OPTIMIZATION=""
DEBUG="-g"

C_SRC="main memory allocator array print string string_id parser primes"
S_SRC="start syscall"

nocrt=false
build=true
rebuild=false
clean=false

while test $# -gt 0
do
    case "$1" in
        nocrt)
            LINKER=ld
            CRT="-nostdlib -fno-builtin"
            nocrt=true
            ;;
        build)
            build=true
            ;;
        rebuild)
            rebuild=true
            build=true
            ;;
        clean) 
            clean=true
            build=false
            ;;
        *)
            echo "Bad option $1"
            exit 1
            ;;
    esac
    shift
done

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

    echo "$CC $SOURCE_FILE $OPTIONS -o $OUTPUT_FILE"
    $CC $SOURCE_FILE $OPTIONS -o $OUTPUT_FILE
    handle_errors
}

# C_FILES=$(add_prefix_and_suffix "$C_SRC" "src/" ".c")
# S_FILES=$(add_prefix_and_suffix "$S_SRC" "src/" ".s")
O_FILES=$(add_prefix_and_suffix "$C_SRC" $BIN_DIR .o)

if [ $nocrt = true ]; then
    O_FILES="$O_FILES $(add_prefix_and_suffix "$S_SRC" $BIN_DIR .o)"
fi

rm -f compile_log.txt
exec 1> >(tee -a compile_log.txt) 2> >(tee -a compile_log.txt >&2)

# ========= CLEAN =========
if [ $clean = true ]; then
    rm -rf bin
fi

# ========= BUILD =========
if [ $build = true ]; then
    mkdir -p $BIN_DIR
    if [ $nocrt = true ]; then
        for file in $S_SRC
        do
            source_path=$(add_prefix_and_suffix $file src/ .s)
            object_path=$(add_prefix_and_suffix $file $BIN_DIR .o)
            if [ $rebuild = true ] || [ $source_path -nt $object_path ] || [ "build.sh" -nt $object_path ]; then
                compile_assembly $source_path $object_path $DEBUG
            fi
        done
    fi

    for file in $C_SRC
    do
        source_path=$(add_prefix_and_suffix $file src/ .c)
        object_path=$(add_prefix_and_suffix $file $BIN_DIR .o)
        if [ $rebuild = true ] || [ $source_path -nt $object_path ] || [ "build.sh" -nt $object_path ]
        then
            compile_c $source_path $object_path "-c $OPTIMIZATION $DEBUG $CRT $WARNINGS -std=$C_STD -Isrc/"
        fi
    done


    echo "$LINKER $O_FILES -o $BIN_DIR$PROJECT"
    $LINKER $O_FILES -o $BIN_DIR$PROJECT
    handle_errors
    echo "Output file: $BIN_DIR$PROJECT"

    chmod +w $BIN_DIR$PROJECT
fi

# compile_c "src/elf_loader.c src/elf.c" "elf_loader" "$DEBUG $WARNINGS -Isrc/"

