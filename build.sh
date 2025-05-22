#!/bin/bash

# Update and install dependencies
pacman -Syu --noconfirm
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-make

# Clean any previous build
rm -rf build

# Configure project using MinGW Makefiles
cmake -S . -B build -G "MinGW Makefiles"

# Build the project
mingw32-make -C build
