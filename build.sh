#!/bin/bash

function main() {
    set -xe
    mkdir -p ./build
    c++ -std=c++23 -O3 -Wall -Wextra -Wpedantic -fPIC -shared -I. `pkg-config --cflags --libs fmt pixman-1 libdrm hyprland` -fno-gnu-unique -o ./build/hyprsquint.so ./hyprsquint.cpp
    exit 0
}

function clean() {
    set -xe
    rm -rf ./build
    exit 0
}

case $1 in
    clean)
        clean
        ;;

    *|build)
        main
        ;;
esac
