cmake_minimum_required(VERSION 3.6)

project(pong)

add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(polar)

set(PONG_SRCS
	src/pong/app.cpp
	src/pong/main.cpp
)

# pong
add_executable(pong WIN32 MACOSX_BUNDLE ${PONG_SRCS})
target_include_directories(pong PRIVATE include polar/include ${POLAR_INCLUDE_DIRS})
target_link_libraries(pong ${POLAR_LIBS} $<TARGET_FILE:polar>)
set_property(TARGET pong PROPERTY CXX_STANDARD 11)
set_property(TARGET pong PROPERTY CXX_STANDARD_REQUIRED ON)
add_dependencies(pong polar)

# copy dynamic libraries
add_custom_command(TARGET pong POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${POLAR_DYLIBS} $<TARGET_FILE_DIR:pong>
)

# copy macOS frameworks
# TODO: somehow generalize this in polar CMake infrastructure
if(APPLE)
	set(PONG_FRAMEWORKS_DIR ${CMAKE_CURRENT_BINARY_DIR}/pong.app/Contents/Frameworks)
	add_custom_command(TARGET pong POST_BUILD
		COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/../Frameworks" $<TARGET_FILE:pong>
	)

	set(CMDS "")
	foreach(V ${POLAR_FRAMEWORKS})
		get_filename_component(BARENAME ${V} NAME)
		set(CMDS ${CMDS} COMMAND ${CMAKE_COMMAND} -E copy_directory ${V} ${PONG_FRAMEWORKS_DIR}/${BARENAME})
	endforeach()
	add_custom_target(appframeworks ALL ${CMDS})
endif()

# target assets directory
if(APPLE)
	set(PONG_RESOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}/pong.app/Contents/Resources)
else()
	set(PONG_RESOURCES_DIR $<TARGET_FILE_DIR:pong>)
endif()

# assets
add_custom_target(assets ALL
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${PONG_RESOURCES_DIR}/assets
	COMMAND $<TARGET_FILE:assetbuilder> ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/assets ${PONG_RESOURCES_DIR}/assets
)
add_dependencies(assets pong)
add_dependencies(assets assetbuilder)
