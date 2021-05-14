cmake_minimum_required (VERSION 3.7)

# Project name
project (Booma)

# Set the framework major and minor version here
set (Booma_VERSION_MAJOR 1)
set (Booma_VERSION_MINOR 1)
set (Booma_VERSION_BUILD 27)

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

# Package into debian package
if(UNIX AND NOT APPLE)
    ADD_CUSTOM_TARGET( dist
    COMMAND sudo rm -r -f dist
    COMMAND mkdir -p dist/booma
    COMMAND cp -r booma/debian dist/booma/.
    COMMAND tar -c -v -f 'dist/booma_${Booma_VERSION_MAJOR}.${Booma_VERSION_MINOR}.orig.tar.gz' -z 'CMakeLists.txt' 'booma'
    COMMAND cd dist/booma && debuild -us -uc --root-command=sudo
    COMMAND sudo chown -R $ENV{USER} dist
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

    add_dependencies( dist booma )

    ADD_CUSTOM_TARGET( publish
        COMMAND mkdir ../dist/site
        COMMAND cp ../site/index.html ../dist/site/index.html
        COMMAND ../publish.sh)

    add_dependencies( publish dist )

else()

    ADD_CUSTOM_TARGET( dist
    COMMAND sudo rm -r -f dist
    COMMAND mkdir -p dist/booma
    COMMAND tar -c -v -f 'dist/booma_${Booma_VERSION_MAJOR}.${Booma_VERSION_MINOR}.orig.tar.gz' -z 'CMakeLists.txt' 'booma'
    COMMAND sudo chown -R $ENV{USER} dist
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

    add_dependencies( dist booma )

    ADD_CUSTOM_TARGET( publish
        COMMAND mkdir ../dist/site
        COMMAND cp ../site/index.html ../dist/site/index.html
        COMMAND ../publish.sh)

    add_dependencies( publish dist )

endif()

ADD_CUSTOM_TARGET( distclean
    COMMAND sudo rm -r -f ../dist
    COMMAND sudo rm -r -f booma
WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
