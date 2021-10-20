cmake_minimum_required(VERSION 3.10)
project(SlidePuzzle
	VERSION 1.0.0
	LANGUAGES C
)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(EXE ${CMAKE_PROJECT_NAME})
set(EXE_SOURCES
	SlidePuzzle.h
	SlidePuzzle.c
	App.c
)
add_executable(${EXE} ${EXE_SOURCES})
target_include_directories(${EXE} PRIVATE ${CMAKE_SOURCE_DIR})

if(MSVC)
	target_link_options(${EXE}
		PRIVATE
			$<$<NOT:$<CONFIG:Debug>>:
				/ENTRY:mainCRTStartup
				/SUBSYSTEM:WINDOWS
			>
	)
	set_target_properties(${EXE} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME}
	)
endif()

if(VCPKG_TOOLCHAIN)
	find_package(SDL2 REQUIRED)
	find_package(sdl2-image REQUIRED)
	target_link_libraries(${EXE}
		PUBLIC
			SDL2::SDL2
			SDL2::SDL2main
			SDL2::SDL2_image
	)
else()
	find_package(PkgConfig)
	if(PKG_CONFIG_FOUND)
		function(target_pkg_config_import TARGET DEPENDENCY)
			string(TOUPPER ${DEPENDENCY} DEPENDENCY_VARIABLE)
			pkg_check_modules(${DEPENDENCY_VARIABLE} REQUIRED ${DEPENDENCY})
			target_compile_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_CFLAGS}")
			target_compile_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_CFLAGS_OTHER}")
			target_include_directories(${TARGET} SYSTEM PRIVATE "${${DEPENDENCY_VARIABLE}_INCLUDE_DIRS}")
			target_link_directories(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LIBRARY_DIRS}")
			target_link_libraries(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LIBRARIES}")
			target_link_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LDFLAGS}")
			target_link_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LDFLAGS_OTHER}")
		endfunction()
	
		target_link_libraries(${EXE} PRIVATE
			m
			${CMAKE_DL_LIBS}
		)

		set(DEPENDENCIES
			sdl2
			SDL2_image
		)
		foreach(DEPENDENCY ${DEPENDENCIES})
			target_pkg_config_import(${EXE} ${DEPENDENCY})
		endforeach()
	else()
		message(FATAL_ERROR "pkg-config not found")
	endif()
endif()
