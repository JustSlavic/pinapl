#!/bin/bash

set -ex

script_path=$(dirname -- "$( readlink -f -- "$0"; )")
os_name=$(uname -s)
compiler="gcc"
assembler="as"
linker="ld"
cc_flags="-std=c89 -g"
cc_warnings="-Wall -Werror"

mkdir -p build
mkdir -p bin

function compile_ttb_asm_x86_64_linux() {
    source="$script_path/code/ttb/ttb.S"
    object="$script_path/build/ttba.o"
    result="$script_path/bin/ttba"

    $assembler -msyntax=intel -mmnemonic=intel -mnaked-reg -o $object $source
    ld -nostdlib -o $result $object
    echo "Done [$result]"

    echo "Checking..."
    $result $script_path/code/ttb/ttb.txt $script_path/bin/ttb1
    $script_path/bin/ttb1 $script_path/code/ttb/ttb.txt $script_path/bin/ttb2
    cmp $script_path/bin/ttb1 $script_path/bin/ttb2
    echo "Compare result: $?"
}

function compile_ttb_C_x86_64_linux() {
    source="$script_path/code/ttb/ttb.c"
    result="$script_path/bin/ttbc"

    $compiler $cc_flags $cc_warnings -o $result $source
    echo "Done [$result]"

    echo "Checking..."
    $result $script_path/code/ttb/ttb.txt $script_path/bin/ttb1
    $script_path/bin/ttb1 $script_path/code/ttb/ttb.txt $script_path/bin/ttb2
    cmp $script_path/bin/ttb1 $script_path/bin/ttb2
    echo "Compare result: $?"
}

# compile_ttb_asm_x86_64_linux
compile_ttb_C_x86_64_linux
