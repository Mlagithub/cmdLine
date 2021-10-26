cmake_minimum_required(VERSION 3.0.0)
project(CGNSMeshCutter VERSION 0.1.0)

cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

# main
set(TgtExe "main.exe")
add_executable(${TgtExe} main.cpp)


# googletest
# export PKG_CONFIG_PATH=/home/one/.local/googletest-1.11.0/lib/pkgconfig/
find_package(PkgConfig)
pkg_search_module(GTEST REQUIRED gtest_main)
if(GTEST_FOUND)
    message(STATUS "Found GTEST_LIB: " ${GTEST_LDFLAGS})
    target_link_libraries(${TgtExe} ${GTEST_LDFLAGS})
    target_compile_options(${TgtExe} PUBLIC ${GTEST_CFLAGS})
endif(GTEST_FOUND)

