# pips-solver

A C++ solver for [The New York Times Pips](https://www.nytimes.com/games/pips) puzzle.

## Features
- Solves Easy, Medium, and Hard daily puzzles
- Colorful terminal output with region highlighting
- Automatic download of today’s puzzle from NYT

## Requirements
- C++23 compiler (GCC 13+, Clang 16+)
- CMake ≥ 3.15
- `curl` command-line tool

## Build Instructions

You can either run the just commands or these CMake ones

```sh
# Configure
cmake -S . -B build

# Build
cmake --build build

# Run
./build/main
