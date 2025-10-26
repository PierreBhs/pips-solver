cmake_minimum_required(VERSION 3.15)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers)

include(FetchContent)

# CPR
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 1.12.0) # Replace with your desired git commit from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

# JSON
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(main src/main.cpp src/pips_data.cpp src/pips_game.cpp src/solver.cpp src/display.cpp)

target_link_libraries(main PRIVATE cpr::cpr nlohmann_json::nlohmann_json)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    RESULT_VARIABLE symlink_result
    OUTPUT_QUIET ERROR_QUIET
)
if(NOT symlink_result EQUAL 0)
    message(WARNING "Could not create symlink for compile_commands.json.")
endif()
