configure:
    @cmake -S . -B build

build:
    @cmake --build build

run: build
    @./build/main

clean:
    @rm -rf build compile_commands.json