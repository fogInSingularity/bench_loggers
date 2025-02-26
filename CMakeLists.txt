cmake_minimum_required(VERSION 3.15)

project(logger_benchmark CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(ASAN_FLAGS "-fsanitize=address,leak,undefined")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fstack-protector-strong -fcheck-new -fstrict-overflow")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g3 -ggdb ${ASAN_FLAGS}") # -D_FORTIFY_SOURCE=3
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -march=native -flto -DNDEBUG")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Og -g3 -ggdb ${ASAN_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O2 -march=native -flto -DNDEBUG")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # to generate compile_commands.json

# to output colors
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always") # For all targets
endif()

include(cmake/CPM.cmake)

# Spdlog
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.15.1
)

# Quill
CPMAddPackage(
    NAME quill
    GITHUB_REPOSITORY odygrd/quill
    VERSION 8.2.0 
    OPTIONS "QUILL_BUILD_EXAMPLES OFF;QUILL_BUILD_TESTS OFF;QUILL_INSTALL OFF"
)

# --- Executable Target ---
add_executable(logger_benchmark logger_benchmark.cpp)

# --- Link Libraries ---
target_link_libraries(logger_benchmark
    PRIVATE
        spdlog::spdlog
        quill::quill
)
