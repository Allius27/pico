
cmake_minimum_required(VERSION 3.10)

set(DEFAULT_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)

project(libpicornt
    LANGUAGES CXX
)

add_library( ${PROJECT_NAME} SHARED rnt/picornt.cpp )

set_target_properties ( ${PROJECT_NAME}
	PROPERTIES PUBLIC_HEADER rnt/picornt.h )

INSTALL(
	TARGETS ${PROJECT_NAME}

	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)

INSTALL(
	FILES
	rnt/cascades/facefinder DESTINATION share/${PROJECT_NAME}
)

