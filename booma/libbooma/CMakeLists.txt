set(CMAKE_MACOSX_RPATH 1)

find_package(Hardt CONFIG)

add_library(booma SHARED "" include/configoptionvalues.h)

include_directories(${Hardt_INCLUDE_DIRS})
target_link_libraries (booma ${Hardt_LIBRARIES} pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
set_target_properties( booma PROPERTIES
	VERSION ${Booma_VERSION_MAJOR}.${Booma_VERSION_MINOR}
	SOVERSION ${Booma_VERSION_MAJOR} )

target_sources(booma
    PRIVATE
		boomaapplication.cpp
		boomacwreceiver.cpp
		configoptions.cpp
		boomareceiver.cpp
		boomainput.cpp
		boomaoutput.cpp
		boomaauroralreceiver.cpp
		boomaamreceiver.cpp
		boomassbreceiver.cpp
)

include_directories("${PROJECT_BINARY_DIR}/booma/libbooma/include")

target_include_directories(booma PRIVATE . include)

include(GNUInstallDirs)
install(TARGETS booma
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

