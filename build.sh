#!/bin/bash

set -e

PROJECT=pinapl
COMPILER=g++
STANDARD=c++17
COMPILE_COMMANDS_FILE=compile_commands.json

mkdir -p bin

if [ $# -eq 0 ]; then
    command="build"
else
    if [ "$1" = "debug" ]; then
        command="build"
        subcommand="debug"
    else
        command="$1"
        subcommand="$2"
    fi
fi

os_name=$(uname -s)

function build() {
    if [ "$command" = "pvs-analyze" ]; then
        COMPILE_DB_JSON="-MJ $COMPILE_COMMANDS_FILE"
    fi
    if [ "$subcommand" = "debug" ]; then
        DEBUG="-DDEBUG -g3 -O0"
        WARNINGS="-Wall"
    else
        WARNINGS="-Wall -Werror -O2"
    fi

    case $os_name in
        Darwin | Linux)
            build_command="$COMPILER code/main.cpp -o bin/$PROJECT -I code/based -std=$STANDARD $WARNINGS $DEBUG $COMPILE_DB_JSON"
            exec $($build_command)
            echo "[$build_command]... Success"
            ;;
        *)
            echo "Unrecognazied os name ($os_name)"
            ;;
    esac
}

function run() {
    bin/$PROJECT
}

function pvs_analyze() {
    build

    db_contents=$(cat $COMPILE_COMMANDS_FILE)
    db_contents_except_last_comma=${db_contents%?}
    echo "[$db_contents_except_last_comma]" > $COMPILE_COMMANDS_FILE

    pvs-studio-analyzer analyze -o pvs_output.log -j2
    plog-converter -a GA:1,2 -t json -o pvs_report.json pvs_output.log
    less pvs_report.json
}

function contains_in() {
    for it in $2; do
        if [ "$1" = "$it" ]; then
            return 0
        fi
    done
    return 1
}

if contains_in $command "build run pvs-analyze"; then
    "$command"
else
    echo "Could not recognize command '$command'"
fi

