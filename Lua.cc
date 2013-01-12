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
    if (lua_gettop(vm) - (i>0?i:-i) < 0 || !lua_istable(vm, i))
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

void Lua::object(const LuaClass *object, const std::string& name) {
    load(name);
    table();
    load("mtab", -2);
    metatable();
    remove(-2);
    userdata(object);
    save("__self__");
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

void LuaObject::export_class(Lua& vm) {
}

void LuaObject::export_me(Lua& vm) {
    util::export_class<LuaObject>(vm);
}

const std::string LuaObject::class_name() {
    return "Object";
}

const std::string LuaObject::obj_class_name() const {
    return "Object";
}

void util::export_function(Lua& vm, const std::string& name, void (*callback)()) {
    auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
        (*callback)();
        return 0;
    });
    vm.lambda(function, name);
}

using namespace util::lua;

template <>
int ret<lua_Number>(Lua& vm, const lua_Number r) {
    vm.number(r);
    return 1;
}

template <>
int ret<std::string>(Lua& vm, const std::string r) {
    vm.string(r);
    return 1;
}

template <>
int ret<bool>(Lua& vm, const bool r) {
    vm.boolean(r);
    return 1;
}

template <>
int ret<int>(Lua& vm, const int r) {
    vm.number(r);
    return 1;
}

template <>
std::string arg<std::string>(Lua& vm, const int i) {
    return vm.string(i);
}

template <>
lua_Number arg<lua_Number>(Lua& vm, const int i) {
    return vm.tonumber(i);
}

template <>
bool arg<bool>(Lua& vm, const int i) {
    return vm.boolean(i);
}

template <>
lua_Integer arg<lua_Integer>(Lua& vm, const int i) {
    return vm.tonumber(i);
}

template <>
int arg<int>(Lua& vm, const int i) {
    return vm.tonumber(i);
}
