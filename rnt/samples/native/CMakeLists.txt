cmake_minimum_required(VERSION 3.10)

set(DEFAULT_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)

project( picornt_demo
    LANGUAGES CXX
)

set(CMAKE_INSTALL_RPATH "../lib")

add_executable( ${PROJECT_NAME} sample.cpp )


include_directories ( ${PROJECT_NAME} ${CMAKE_PREFIX_PATH}/include/opencv4 )
target_link_libraries( ${PROJECT_NAME} opencv_world picornt)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-L${CMAKE_PREFIX_PATH}/lib")


INSTALL(
	TARGETS ${PROJECT_NAME}

	RUNTIME DESTINATION bin
)