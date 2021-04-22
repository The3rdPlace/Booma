find_package(Hardt CONFIG)

find_package(PkgConfig)

SET(OpenGL_GL_PREFERENCE GLVND)
find_package(FLTK)
	if(FLTK_FOUND)

	include_directories(${Hardt_INCLUDE_DIRS})
	include_directories("${PROJECT_BINARY_DIR}/booma/libbooma/include")
	include_directories("${PROJECT_SOURCE_DIR}/booma/libbooma/include")
	include_directories ("${PROJECT_SOURCE_DIR}")
	include_directories("${PROJECT_SOURCE_DIR}/booma/booma-gui/include")
	include_directories("${PROJECT_BINARY_DIR}/booma/booma-gui/include")
	include_directories(${FLTK_INCLUDE_DIR})

	# Set the application major and minor version here
	set (BoomaGui_VERSION_MAJOR 1)
	set (BoomaGui_VERSION_MINOR 0)
	set (BoomaGui_VERSION_BUILD 0)

	# Configure the main.h header
	configure_file (
	  "${PROJECT_SOURCE_DIR}/booma/booma-gui/include/main.h.in"
	  "${PROJECT_BINARY_DIR}/booma/booma-gui/include/main.h"
	)

	# add the executable
	add_executable (booma-gui main.cpp mainwindow.cpp mainwindow.cpp waterfall.cpp inputdialog.cpp)
	target_link_libraries (booma-gui booma pthread ${Hardt_LIBRARIES} ${FLTK_LIBRARIES})

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

	include(GNUInstallDirs)
	install(
		FILES ${CMAKE_BINARY_DIR}/booma/booma-gui/booma-gui
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)

else()
	message("*************************************************************************************************")
	message("If you want to build the graphical application, booma-gui, then 'sudo apt install libfltk1.3-dev'")
	message("*************************************************************************************************")
endif()
