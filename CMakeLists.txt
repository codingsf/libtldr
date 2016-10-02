cmake_minimum_required(VERSION 3.0)

include(GenerateExportHeader)

project(libtldr VERSION 1.0.0)

set(tldr_src_files src/loader.cpp
                   src/module.cpp
                   src/raw_module.cpp
                   src/system_loader.cpp)

if (UNIX)
	if (CMAKE_SYSTEM_PROCESSOR MATCHES x86_64|amd64)
		set(TLDR_HAS_ELF64_SUPPORT ON)
		set(TLDR_HAS_ELF32_SUPPORT ON)
	elseif (CMAKE_SYSTEM_PROCESSOR MATCHES x86)
		set(TLDR_HAS_ELF32_SUPPORT ON)
	endif()

	list(APPEND tldr_src_files src/detail/posix/lib_module.cpp
	                           src/detail/posix/vmemory.cpp)
elseif (WIN32)
	if (CMAKE_SYSTEM_PROCESSOR MATCHES x86_64|amd64)
		set(TLDR_HAS_PE64_SUPPORT ON)
		set(TLDR_HAS_PE32_SUPPORT ON)
	elseif (CMAKE_SYSTEM_PROCESSOR MATCHES x86)
		set(TLDR_HAS_PE32_SUPPORT ON)
	endif()

	list(APPEND tldr_src_files src/detail/windows/lib_module.cpp
	                           src/detail/windows/vmemory.cpp)
endif()

if (TLDR_HAS_ELF32_SUPPORT OR TLDR_HAS_ELF64_SUPPORT)
	list(APPEND tldr_src_files src/elf/hash.cpp)
endif()

configure_file(config.h.cmake config.h)
configure_file(tldr/version.hpp.cmake version.hpp)
include_directories(BEFORE "${PROJECT_BINARY_DIR}")
include_directories(BEFORE "${PROJECT_SOURCE_DIR}")

add_library(tldr SHARED ${tldr_src_files})
target_link_libraries(tldr ${CMAKE_DL_LIBS})
set_property(TARGET tldr PROPERTY CXX_STANDARD 14)
generate_export_header(tldr EXPORT_FILE_NAME tldr/export.h)

enable_testing()
add_subdirectory(test)
