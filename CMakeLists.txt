cmake_minimum_required (VERSION 3.0)

include ("project_configs.cmake")

project (${PROJECT_NAME})

include_directories ("${PROJECT_SOURCE_DIR}/include")

if (${MSVC_PROJECT})
	file (GLOB_RECURSE file_list
		LIST_DIRECTORIES false
		"${PROJECT_SOURCE_DIR}/include/*.h"
		"${PROJECT_SOURCE_DIR}/include/*.inl"
		"${PROJECT_SOURCE_DIR}/src/*.cpp")
else ()
	set (file_list
		"${PROJECT_SOURCE_DIR}/src/generated_code/proxy.cpp"
		"${PROJECT_SOURCE_DIR}/src/generated_code/oglapis.cpp"

		"${PROJECT_SOURCE_DIR}/src/configs.cpp"
		"${PROJECT_SOURCE_DIR}/src/context.cpp"
		"${PROJECT_SOURCE_DIR}/src/driver.cpp"
		"${PROJECT_SOURCE_DIR}/src/memory.cpp"
		"${PROJECT_SOURCE_DIR}/src/system.cpp"
		"${PROJECT_SOURCE_DIR}/src/counters.cpp"

		"${PROJECT_SOURCE_DIR}/src/internal_states.cpp"
		"${PROJECT_SOURCE_DIR}/src/ut_shading.cpp"
		"${PROJECT_SOURCE_DIR}/src/ut_buffers.cpp"
		"${PROJECT_SOURCE_DIR}/src/ut_render.cpp"
		"${PROJECT_SOURCE_DIR}/src/ut_textures.cpp"
		"${PROJECT_SOURCE_DIR}/src/detail/rt_shading.cpp"
		"${PROJECT_SOURCE_DIR}/src/detail/rt_buffers.cpp"
		"${PROJECT_SOURCE_DIR}/src/detail/rt_render.cpp"
		"${PROJECT_SOURCE_DIR}/src/detail/rt_textures.cpp"
		)
endif (${MSVC_PROJECT})

add_definitions(
	-D_CRT_SECURE_NO_WARNINGS)
	#-DUSE_BUSY_LOCK)

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
		"${PROJECT_SOURCE_DIR}/src/driver_egl.cpp")
	set (lib_list
		${lib_list}
		EGL)

else ()
	message(STATUS ${PROJECT_NAME} " will be built using Windows configs")
	find_package (OpenGL)
	add_definitions (
		-DPLATFORM_WINDOWS)
	if (${MSVC_PROJECT})
		list (REMOVE_ITEM file_list
			"${PROJECT_SOURCE_DIR}/src/driver_egl.cpp")
	else ()
		set (file_list
			${file_list}
			"${PROJECT_SOURCE_DIR}/src/driver_windows.cpp")
	endif (${MSVC_PROJECT})
	set (lib_list
		${lib_list}
		opengl32)
endif (${ANDROID_BUILD})

add_library (${PROJECT_NAME} ${file_list})

target_link_libraries (${PROJECT_NAME}
	${lib_list})

target_include_directories (${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (${MSVC_PROJECT})
	# organize filters
	foreach(_source IN ITEMS ${file_list})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${PROJECT_SOURCE_DIR}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endif (${MSVC_PROJECT})
