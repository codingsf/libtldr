include(ExternalProject)

find_package(Threads REQUIRED)

ExternalProject_Add(googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	CMAKE_ARGS -DBUILD_GTEST=ON INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googletest SOURCE_DIR)
ExternalProject_Get_Property(googletest BINARY_DIR)

include_directories("${SOURCE_DIR}/googletest/include")
include_directories("${SOURCE_DIR}/googlemock/include")

set(TLDR_TEST_MODULE_PATH libfoo.so)
set(TLDR_TEST_MODULE_DATA_SYMBOL foo_data)
set(TLDR_TEST_MODULE_PROC_SYMBOL foo_fn)

configure_file(config.h.cmake config.h)
include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

add_library(gtest UNKNOWN IMPORTED)
set(gtest_path "${BINARY_DIR}/googlemock/gtest/libgtest.a")
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION "${gtest_path}")
add_dependencies(gtest googletest)

add_library(gmock UNKNOWN IMPORTED)
set(gmock_path "${BINARY_DIR}/googlemock/libgmock.a")
set_target_properties(gmock PROPERTIES IMPORTED_LOCATION "${gmock_path}")
add_dependencies(gmock googletest)

add_library(gtest_main UNKNOWN IMPORTED)
set(gtest_main_path "${BINARY_DIR}/googlemock/gtest/libgtest_main.a")
set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION "${gtest_main_path}")
add_dependencies(gtest_main googletest)

add_executable(endian_tests endian.cpp)
set_target_properties(endian_tests PROPERTIES CXX_STANDARD 14)
set_target_properties(endian_tests PROPERTIES OUTPUT_NAME endian-tests)
target_link_libraries(endian_tests ${CMAKE_THREAD_LIBS_INIT} gtest_main gtest gmock)
add_test(NAME endian-tests COMMAND $<TARGET_FILE:endian_tests>)

add_executable(loader_tests loader.cpp)
set_target_properties(loader_tests PROPERTIES CXX_STANDARD 14)
set_target_properties(loader_tests PROPERTIES OUTPUT_NAME loader-tests)
target_link_libraries(loader_tests ${CMAKE_THREAD_LIBS_INIT} tldr gtest_main gtest gmock)
add_test(NAME loader-tests COMMAND $<TARGET_FILE:loader_tests>)

add_executable(lib_module_tests lib_module.cpp)
set_target_properties(lib_module_tests PROPERTIES CXX_STANDARD 14)
set_target_properties(lib_module_tests PROPERTIES OUTPUT_NAME lib_module-tests)
target_link_libraries(lib_module_tests ${CMAKE_THREAD_LIBS_INIT} tldr gtest_main gtest gmock)
add_test(NAME lib_module-tests COMMAND $<TARGET_FILE:lib_module_tests>)

add_library(foo SHARED foo.cpp)
add_executable(raw_module_tests raw_module.cpp)
set_target_properties(raw_module_tests PROPERTIES CXX_STANDARD 14)
set_target_properties(raw_module_tests PROPERTIES OUTPUT_NAME raw_module-tests)
target_link_libraries(raw_module_tests ${CMAKE_THREAD_LIBS_INIT} tldr gtest_main gtest gmock)
add_test(NAME raw_module-tests COMMAND $<TARGET_FILE:raw_module_tests>)
add_dependencies(raw_module_tests foo)
