/*
 * By orbyfied (2022) 
 * License can be found at https://github.com/orbyfied
 */

#ifndef SERVER_LUA_UTIL_H
#define SERVER_LUA_UTIL_H

// include lua
#include "lua/include/lua.hpp"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"

#include <iostream>
#include <cstdlib>

/* ------ Table Manipulation ------ */

inline void ll_setcfunc(lua_State* L, const char* str, lua_CFunction func) {
    lua_pushstring(L, str);
    lua_pushcfunction(L, func);
    lua_settable(L, -3);
}

inline void ll_setstr(lua_State* L, const char* str, const char* val) {
    lua_pushstring(L, str);
    lua_pushstring(L, val);
    lua_settable(L, -3);
}

inline int checklua(lua_State* L, int r) {
    if (r != LUA_OK) {
        std::string err = lua_tostring(L, -1);
        std::cout << "LUA: " << err << std::endl;
        return 0;
    }

    return 1;
}

/* ------- Metatables And OOP ------- */

inline void ll_createmtc(lua_State* L, const char* name) {
    luaL_newmetatable(L, name);
    lua_pushstring(L, "__index");
    luaL_getmetatable(L, name);
    lua_settable(L, -3);
}

inline void ll_setmtc(lua_State* L, const char* name) {
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2);
}

template <typename T>
inline T* ll_self(lua_State* L, const char* mtcl) {
    void* ud = luaL_checkudata(L, 1, mtcl);
    if (ud == nullptr) {
        char* s;
        sprintf(s, "expected instance of type %s", mtcl);
        luaL_argerror(L, 1, s);
    }
    return (T*) ud;
}

template <typename T>
inline T* ll_getptr(lua_State* L) {
    void* ud = lua_touserdata(L, -1);
    return (T*)ud;
}

inline void ll_pushptr(lua_State* L, void* ptr, const char* mtcl = nullptr) {
    lua_pushlightuserdata(L, ptr);
    if (mtcl != nullptr)
        ll_setmtc(L, mtcl);
}

#endif //SERVER_LUA_UTIL_H
