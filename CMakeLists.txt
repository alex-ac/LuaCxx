cmake_minimum_required(VERSION 2.8.9)

project (LuaCxx CXX)

set (LuaCxx_VERSION_MAJOR 0)
set (LuaCxx_VERSION_MINOR 1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CheckCXX11Features)
find_package(Lua REQUIRED)

if (DEFINED CXX11_COMPILER_FLAGS)
    add_definitions(${CXX11_COMPILER_FLAGS})
endif()
include_directories(${PROJECT_SOURCE_DIR} ${LUA_INCLUDE_DIRECTORY})

add_library(LuaCxx_static STATIC Lua.cc)
target_link_libraries(LuaCxx_static ${LUA_LIBRARY})
add_library(LuaCxx SHARED Lua.cc)
target_link_libraries(LuaCxx ${LUA_LIBRARY})

install(TARGETS LuaCxx_static LuaCxx
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES "${PROJECT_SOURCE_DIR}/Lua.hh" DESTINATION include)

set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${LuaCxx_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${LuaCxx_VERSION_MINOR}")
include (CPack)

include (CTest)
set (CTEST_PROJECT_NAME "LuaCxx")

