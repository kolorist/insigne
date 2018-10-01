cmake_minimum_required (VERSION 3.0)

include ("project_configs.cmake")

project (${PROJECT_NAME})

include_directories ("${PROJECT_SOURCE_DIR}/include")

set (file_list
	"${PROJECT_SOURCE_DIR}/src/generated_code/proxy.cpp"
	"${PROJECT_SOURCE_DIR}/src/generated_code/oglapis.cpp"
	"${PROJECT_SOURCE_DIR}/src/context.cpp"
	"${PROJECT_SOURCE_DIR}/src/render.cpp"
	"${PROJECT_SOURCE_DIR}/src/memory.cpp"
	"${PROJECT_SOURCE_DIR}/src/detail/render_states.cpp"
	"${PROJECT_SOURCE_DIR}/src/detail/renderer_states_gl.cpp"
	"${PROJECT_SOURCE_DIR}/src/counters.cpp"
	
	"${PROJECT_SOURCE_DIR}/src/ut_shading.cpp"
	"${PROJECT_SOURCE_DIR}/src/ut_buffers.cpp"
	"${PROJECT_SOURCE_DIR}/src/detail/rt_shading.cpp"
	"${PROJECT_SOURCE_DIR}/src/detail/rt_buffers.cpp"
	)

add_definitions(
	-D_CRT_SECURE_NO_WARNINGS)
	
set (lib_list
	floral
	calyx
	clover
	helich
	lotus)

if (${ANDROID_BUILD})
	message(STATUS ${PROJECT_NAME} " will be built using Android configs")
	add_definitions (
		-DPLATFORM_POSIX)
		
	# platform abi
	if (${ANDROID_ABI} STREQUAL "arm64-v8a")
		message(STATUS ${PROJECT_NAME} " Android ABI: arm64")
		add_definitions (
			-DPOSIX64)
	else ()
		message(STATUS ${PROJECT_NAME} " Android ABI: arm")
		add_definitions (
			-DPOSIX32)
	endif (${ANDROID_ABI} STREQUAL "arm64-v8a")	
	
	set (file_list
		${file_list}
		"${PROJECT_SOURCE_DIR}/src/driver_egl.cpp"
		"${PROJECT_SOURCE_DIR}/src/renderer_egl.cpp")
	set (lib_list
		${lib_list}
		EGL)
	
else ()
	message(STATUS ${PROJECT_NAME} " will be built using Windows configs")
	find_package (OpenGL)
	add_definitions (
		-DPLATFORM_WINDOWS)
	set (file_list
		${file_list}
		"${PROJECT_SOURCE_DIR}/src/driver_windows.cpp"
		"${PROJECT_SOURCE_DIR}/src/renderer_gl.cpp")
	set (lib_list
		${lib_list}
		opengl32)
endif (${ANDROID_BUILD})

add_library (${PROJECT_NAME} ${file_list})

target_link_libraries (${PROJECT_NAME}
	${lib_list})

target_include_directories (${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
