find_package(Hardt CONFIG)

include_directories(${Hardt_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}/booma/libbooma/include")
include_directories("${PROJECT_SOURCE_DIR}/booma/libbooma/include")
include_directories ("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/booma/booma-remote/include")
include_directories("${PROJECT_BINARY_DIR}/booma/booma-remote")

# Set the application major and minor version here
set (BoomaRemote_VERSION_MAJOR 1)
set (BoomaRemote_VERSION_MINOR 0)
set (BoomaRemote_VERSION_BUILD 0)

# Configure the main.h header
configure_file (
  "${PROJECT_SOURCE_DIR}/booma/booma-remote/main.h.in"
  "${PROJECT_BINARY_DIR}/booma/booma-remote/main.h"
)

# add the executable
add_executable (booma-remote main.cpp)
target_link_libraries (booma-remote booma pthread ${Hardt_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

include(GNUInstallDirs)
install(FILES ${CMAKE_BINARY_DIR}/booma/booma-remote/booma-remote DESTINATION ${CMAKE_INSTALL_BINDIR})
