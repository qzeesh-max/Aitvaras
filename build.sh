#!/usr/bin/env bash
set -e

export CXX=g++-16
export CC=gcc-16

mkdir -p build
cd build
cmake ..
cmake --build .
ctest --output-on-failure
