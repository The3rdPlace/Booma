find_package(Hardt CONFIG)

include_directories(${Hardt_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}/booma/libbooma/include")
include_directories("${PROJECT_SOURCE_DIR}/booma/libbooma/include")
include_directories ("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/booma/booma-console/include")

# add the executable
add_executable (booma-console main.cpp)
target_link_libraries (booma-console booma pthread ${Hardt_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

include(GNUInstallDirs)
install(FILES ${CMAKE_BINARY_DIR}/booma/booma-console/booma-console DESTINATION ${CMAKE_INSTALL_BINDIR})
