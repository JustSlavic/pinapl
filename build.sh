#!/bin/bash

# set -ex

os_name=$(uname -s)
compiler="gcc"
assembler="as"
linker="ld"
cc_flags="-std=c89 -g"
cc_warnings="-Wall -Werror"

mkdir -p build
mkdir -p bin

function compile_ttb_asm_x86_64_linux() {
    assembly_source="code/ttb/ttb.S"
    assembly_object="build/ttb_asm.o"
    executable_name="bin/asm.elf"

    $assembler -msyntax=intel -mmnemonic=intel -mnaked-reg -g -o $assembly_object $assembly_source
    ld -nostdlib -o $executable_name $assembly_object -n
    echo "Done [$executable_name]"
}

# compile_ttb_asm_x86_64_linux

$compiler -o bin/ttbc code/ttb/ttb.c

$assembler -msyntax=intel -mmnemonic=intel -mnaked-reg -g -o build/ttb.o code/ttb/ttb.S
$linker -z noexecstack -o bin/ttb build/ttb.o
./bin/ttb ./code/ttb/input.txt ./bin/output.bin
echo "return value: $?"
./bin/output.bin
echo "return value: $?"
