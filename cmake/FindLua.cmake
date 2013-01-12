find_path(LUA_INCLUDE_DIR lua.h)
find_library(LUA_LIBRARY NAMES lua)

if (LUA_INCLUDE_DIR AND LUA_LIBRARY)
    set(LUA_FOUND TRUE)
endif (LUA_INCLUDE_DIR AND LUA_LIBRARY)

if (LUA_FOUND)
    if (NOT lua_FIND_QUIETLY)
        message(STATUS "Found lua: ${LUA_LIBRARY}")
    endif (NOT lua_FIND_QUIETLY)
else (LUA_FOUND)
    message(FAIL_ERROR "lua was not found on your system, but lua module build requested.")
endif (LUA_FOUND)

