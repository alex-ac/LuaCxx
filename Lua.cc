#include <Lua.hh>

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
};

using namespace util;

int Lua::call(lua_State *vm) {
    auto function = (std::function<int(Lua&)> *)
        lua_touserdata(vm, lua_upvalueindex(1));
    auto l = Lua(vm);
    return (*function)(l);
}

Lua::Lua(lua_State *vm):
    del(false),
    vm(vm)
{}

Lua::Lua():
    del(true),
    vm(luaL_newstate())
{
    luaL_openlibs(vm);
}

Lua::~Lua() {
    for (auto lambda : lambdas)
        delete lambda;
    if (del)
        lua_close(vm);
}

void Lua::lambda(std::function<int(Lua&)> *function, const std::string& name) {
    lambdas.push_back(function);
    userdata(function);
    closure(Lua::call);
    save(name);
}

void Lua::file(const std::string& name) {
    luaL_dofile(vm, name.c_str()) && lua_error(vm);
}

void Lua::load(const std::string& name, int i) {
    int t = i;
    if (lua_gettop(vm) - (i>0?i:-i) < 0 || !lua_istable(vm, i)) {
        if (-1 == i) 
            t = LUA_GLOBALSINDEX;
        else
            luaL_error(vm, "Invalid load operation (out of stack)!");
    }
    lua_getfield(vm, t, name.c_str());
}

void Lua::pop(const int i) {
    if (lua_gettop(vm) - i < 0)
        luaL_error(vm, "Invalid pop operation (out of stack)!");
    lua_pop(vm, i);
}

void Lua::remove(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid remove operation (out of stack)!");
    lua_remove(vm, i);
}

void Lua::table() {
    lua_newtable(vm);
}

void Lua::metatable(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0
        || !lua_istable(vm, i) && !lua_isuserdata(vm, i))
        luaL_error(vm, "Invalid set metatable operation (out of stack)!");
    lua_setmetatable(vm, i);
}

void Lua::closure(int (*callback)(lua_State *), const int i) {
    if (lua_gettop(vm) - i < 0)
        luaL_error(vm, "Invalid closure operation (out of stack)!");
    lua_pushcclosure(vm, callback, i);
}

void Lua::copy(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid copy operation (out of stack)!");
    lua_pushvalue(vm, i);
}

void Lua::save(const std::string& name, const int i) {
    int t = i;
    if (lua_gettop(vm) - (i>0?i:-i) < 0 || !lua_istable(vm, i)) {
        if (-2 == i)
            t = LUA_GLOBALSINDEX;
        else
            luaL_error(vm, "Invalid save operation (out of stack)!");
    }
    lua_setfield(vm, t, name.c_str());
}

bool Lua::is_nil(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid is nil operation (out of stack)!");
    return lua_isnil(vm, i);
}

static int collect(lua_State *vm) {
    if (lua_gettop(vm) != 1 || !lua_isuserdata(vm, 1))
        luaL_error(vm, "Invalid collect operation!");
    LuaClass * object = (LuaClass *)lua_touserdata(vm, 1);
    object->collect();
    return 0;
}

void Lua::object(const LuaClass *object, const std::string& name) {
    load(name);
    table();
    load("mtab", -2);
    metatable();
    remove(-2);
    userdata(object);
    table();
    lua_pushcfunction(vm, collect);
    save("__gc");
    metatable();
    save("__self__");
    const_cast<LuaClass *>(object)->reference();
}

LuaClass * Lua::object(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid object operation (out of stack)!");
    if (!lua_istable(vm, i))
        luaL_error(vm, "Invalid object (table expected)!");
    load("__self__", i);
    auto ret = (LuaClass *)userdata();
    pop();
    return ret;
}

void Lua::userdata(const void *d) {
    lua_pushlightuserdata(vm, const_cast<void *>(d));
}

void * Lua::userdata(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid userdata operation (out of stack)! %d, %d", lua_gettop(vm), i);
    if (!lua_isuserdata(vm, i))
        luaL_error(vm, "Invalid userdata operation (userdata expected)!");
    return lua_touserdata(vm, i);
}

void Lua::number(const lua_Number n) {
    lua_pushnumber(vm, n);
}

lua_Number Lua::tonumber(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid tonumber operation (out of stack)!");
    if (!lua_isnumber(vm, i))
        luaL_error(vm, "Invalid tonumber operation (number expected)!");
    return lua_tonumber(vm, i);
}

void Lua::string(const std::string& string) {
    lua_pushstring(vm, string.c_str());
}

std::string Lua::string(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid string operation (out of stack)!");
    if (!lua_isstring(vm, i))
        luaL_error(vm, "Invalid string operation (string expected)!");
    return lua_tostring(vm, i);
}

void Lua::boolean(const bool b) {
    lua_pushboolean(vm, b);
}

bool Lua::boolean(const int i) {
    if (lua_gettop(vm) - (i>0?i:-i) < 0)
        luaL_error(vm, "Invalid boolean operation (out of stack)!");
    if (!lua_isboolean(vm, i))
        luaL_error(vm, "Invalid boolean operation (boolean expected)!");
    return lua_toboolean(vm, i);
}

void Lua::LuaObject::export_class(Lua& vm) {
}

void Lua::LuaObject::export_me(Lua& vm) {
    vm.export_class<LuaObject>();
}

const std::string Lua::LuaObject::class_name() {
    return "Object";
}

void Lua::export_function(const std::string& name, void (*callback)()) {
    auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
        (*callback)();
        return 0;
    });
    lambda(function, name);
}

template <>
int Lua::ret<lua_Number>(const lua_Number r) {
    number(r);
    return 1;
}

template <>
int Lua::ret<std::string>(const std::string r) {
    string(r);
    return 1;
}

template <>
int Lua::ret<bool>(const bool r) {
    boolean(r);
    return 1;
}

template <>
int Lua::ret<int>(const int r) {
    number(r);
    return 1;
}

template <>
std::string Lua::arg<std::string>(const int i) {
    return string(i);
}

template <>
lua_Number Lua::arg<lua_Number>(const int i) {
    return tonumber(i);
}

template <>
bool Lua::arg<bool>(const int i) {
    return boolean(i);
}

template <>
lua_Integer Lua::arg<lua_Integer>(const int i) {
    return tonumber(i);
}

template <>
int Lua::arg<int>(const int i) {
    return tonumber(i);
}

