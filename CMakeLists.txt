cmake_minimum_required (VERSION 3.7)

# Project name
project (Booma)

# Set the framework major and minor version here
set (Booma_VERSION_MAJOR 1)
set (Booma_VERSION_MINOR 0)
set (Booma_VERSION_BUILD 0)

# Define the version string
set (Booma_VERSION Booma_VERSION_MAJOR + "." + Booma_VERSION_MINOR + "." + Booma_VERSION_BUILD)

# Use the CLang compiler
set(CMAKE_CXX_COMPILER "clang++")

# Configure the booma.h header
configure_file (
  "${PROJECT_SOURCE_DIR}/booma/libbooma/include/booma.h.in"
  "${PROJECT_BINARY_DIR}/booma/libbooma/include/booma.h"
)

# Build projects in this folder
add_subdirectory (booma)
