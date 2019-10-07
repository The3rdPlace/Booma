include_directories("${PROJECT_BINARY_DIR}/booma/libbooma/include")
include_directories("${PROJECT_SOURCE_DIR}/booma/libbooma/include")
include_directories ("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/booma/booma-console/include")

# add the executable
add_executable (booma-gui main.cpp)
target_link_libraries (booma-gui booma pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

include(GNUInstallDirs)
install(FILES ${CMAKE_BINARY_DIR}/booma/booma-gui/booma-gui DESTINATION ${CMAKE_INSTALL_BINDIR})
