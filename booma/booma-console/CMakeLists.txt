find_package(Hardt CONFIG)

include_directories(${Hardt_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}/booma/libbooma/include")
include_directories("${PROJECT_SOURCE_DIR}/booma/libbooma/include")
include_directories ("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/booma/booma-console/include")
include_directories("${PROJECT_BINARY_DIR}/booma/booma-console")

# Set the application major and minor version here
set (BoomaConsole_VERSION_MAJOR 1)
set (BoomaConsole_VERSION_MINOR 0)
set (BoomaConsole_VERSION_BUILD 11)

# Configure the main.h header
configure_file (
  "${PROJECT_SOURCE_DIR}/booma/booma-console/main.h.in"
  "${PROJECT_BINARY_DIR}/booma/booma-console/main.h"
)

# add the executable
add_executable (booma-console main.cpp info.cpp)
target_link_libraries (booma-console booma pthread ${Hardt_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

include(GNUInstallDirs)
install(
	FILES ${CMAKE_BINARY_DIR}/booma/booma-console/booma-console 
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
