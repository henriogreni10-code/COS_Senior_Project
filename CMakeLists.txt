cmake_minimum_required(VERSION 3.15)
project(MazeSolverProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set SFML directory for Homebrew installation
set(SFML_DIR "/opt/homebrew/opt/sfml/lib/cmake/SFML")

# Find SFML package
find_package(SFML 3 COMPONENTS System Window Graphics REQUIRED)

# Add executable with all source files
add_executable(MazeSolver
        src/main.cpp
        src/maze/Maze.cpp
        src/maze/MazeGenerator.cpp
        src/algorithms/source/dfs.cpp
        src/algorithms/source/bfs.cpp
        src/algorithms/source/dijkstra.cpp
        src/algorithms/source/AStar.cpp
        src/metrics/metrics.cpp
        src/ui/RendererSFML.cpp
        tests/test_utils.h
        tests/test_maze.cpp
        tests/test_containers.cpp
        tests/test_algorithms.cpp
        tests/test_generator.cpp
)

# Include directories
target_include_directories(MazeSolver PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SFML_INCLUDE_DIR}
)

# Link SFML libraries (SFML 3 uses SFML:: prefix)
target_link_libraries(MazeSolver PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)

# Enable warnings (optional but recommended)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(MazeSolver PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy resources to build directory if needed (uncomment if you have resources)
# file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})