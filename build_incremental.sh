#!/bin/bash

set -e

PROJECT=pinapl

COMPILER=gcc
ASSEMBLER=as
LINKER=gcc

STANDARD=c11

BIN_DIR=bin/

CRT="-DCRT=1"
WARNINGS="-Wall -Werror"
OPTIMIZATION=""
DEBUG="-g"

# C_SRC="main memory allocator array print string string_id parser primes"
C_SRC="main"
S_SRC="start"

OPTION_NOCRT=false
OPTION_BUILD=true
OPTION_REBUILD=false
OPTION_CLEAN=false

OS_NAME=$(uname -s)
ARCH=$(uname -m)

if [[ $ARCH == "aarch64" ]]
then
    ARCH="arm64"
fi

# case "$OS_NAME" in
#     Darwin)
#     ;;
#     Linux)
#     ;;
#     *)
#         echo "OS_NAME=$OS_NAME"
#         exit 1
#     ;;
# esac

echo "Compiling for $OS_NAME ($ARCH architecture)..."

while test $# -gt 0
do
    case "$1" in
        nocrt)
            LINKER=ld
            CRT="-nostdlib -fno-builtin"
            OPTION_NOCRT=true
        ;;
        build)
            OPTION_BUILD=true
        ;;
        rebuild)
            OPTION_REBUILD=true
            OPTION_BUILD=true
        ;;
        clean)
            OPTION_CLEAN=true
            OPTION_BUILD=false
        ;;
        *)
            echo "Bad option \"$1\""
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

function compile_c
{
    SOURCE_FILE=$1
    OUTPUT_FILE=$2
    OPTIONS=$3

    $CC $SOURCE_FILE -o $OUTPUT_FILE $OPTIONS
}

# # C_FILES=$(add_prefix_and_suffix "$C_SRC" "src/" ".c")
# # S_FILES=$(add_prefix_and_suffix "$S_SRC" "src/" ".s")
# O_FILES=$(add_prefix_and_suffix "$C_SRC" $BIN_DIR .o)

# if [ $nocrt = true ]; then
#     O_FILES="$O_FILES $(add_prefix_and_suffix "$S_SRC" $BIN_DIR .o)"
# fi

# rm -f compile_log.txt
# exec 1> >(tee -a compile_log.txt) 2> >(tee -a compile_log.txt >&2)

# ========= CLEAN =========
if [ $OPTION_CLEAN = true ]; then
    rm -rf bin
fi

# # ========= BUILD =========
if [ $OPTION_BUILD = true ]; then
    mkdir -p $BIN_DIR

    if [ $OPTION_NOCRT = true ]; then
        echo "Compiling assembly files:"

        for file in $S_SRC
        do
            source_path=$(add_prefix_and_suffix $file "src/arch_$ARCH/$OS_NAME/" .s)
            object_path=$(add_prefix_and_suffix $file $BIN_DIR .o)
            if [ $OPTION_REBUILD = true ] || [ $source_path -nt $object_path ] || [ "build.sh" -nt $object_path ]
            then
                echo "  $source_path -> $object_path"
                $ASSEMBLER $source_path -o $object_path
            fi
        done
    fi

    echo "Compiling C files:"

    for file in $C_SRC
    do
        source_path=$(add_prefix_and_suffix $file src/ .c)
        object_path=$(add_prefix_and_suffix $file $BIN_DIR .o)
        if [ $OPTION_REBUILD = true ] || [ $source_path -nt $object_path ] || [ "build.sh" -nt $object_path ]
        then
            echo "  $source_path -> $object_path"
            $COMPILER $source_path -o $object_path -c $OPTIMIZATION $DEBUG $CRT $WARNINGS -std=$STANDARD -Isrc/ -Isrc/based/
        fi
    done

    case "$OS_NAME" in
        Darwin)
            LINKER_OPTIONS="-macosx_version_min 13.0 -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch arm64"
        ;;
        Linux)
        ;;
        *)
            echo "Do not know how to link for the operating system $OS_NAME!"
            exit 1
        ;;
    esac

    echo "Linking executable $BIN_DIR$PROJECT:"

    echo "$LINKER $O_FILES -o $BIN_DIR$PROJECT $LINKER_OPTIONS"
    $LINKER $O_FILES -o $BIN_DIR$PROJECT $LINKER_OPTIONS
    echo "Output file: $BIN_DIR$PROJECT"
    # chmod +w $BIN_DIR$PROJECT
fi

# compile_c "src/elf_loader.c src/elf.c" "elf_loader" "$DEBUG $WARNINGS -Isrc/"

