cmake_minimum_required(VERSION 3.20)

project(logger_benchmark CXX)

add_executable(logger_benchmark logger_benchmark.cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -fstack-protector-strong
        -fcheck-new
        -fstrict-overflow

        $<$<CONFIG:Debug>:
            -Og
            -g3
            -ggdb
            # -fsanitize=address,leak,undefined
        >

        $<$<CONFIG:Release>:
            -O2
            -march=native
            -flto
            -DNDEBUG
        >        

        $<$<CONFIG:RelWithDebInfo>:
            -g
            -O2
            -march=native
            -flto
            -DNDEBUG
        >

)

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

# --- Link Libraries ---
target_link_libraries(logger_benchmark
    PRIVATE
        spdlog::spdlog
        quill::quill
)
