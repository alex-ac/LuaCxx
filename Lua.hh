#pragma once

#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <type_traits>

extern "C" {
#include <lua.h>
};

namespace util {

class LuaClass {
private:
protected:
public:
};

class Lua {
private:
    bool del;
    lua_State *vm;
    std::vector<std::function<int(Lua&)> *> lambdas;

    static int call(lua_State *vm);
protected:
public:
    Lua(lua_State *vm);
    Lua();
    ~Lua();

    void lambda(std::function<int(Lua&)> *function, const std::string& name);

    void load(const std::string& name, const int i = -1);
    void pop(const int i = 1);
    void remove(const int i);
    void table();
    void metatable(const int i = -2);
    void closure(int (*)(lua_State *), const int i = 1);
    void copy(const int i = -1);
    void save(const std::string& name, const int i = -2);

    void file(const std::string& name);

    bool is_nil(const int i = -1);

    void object(const LuaClass *, const std::string& name);
    LuaClass * object(const int i = -1);

    void userdata(const void *);
    void * userdata(const int i = -1);

    void number(const lua_Number);
    lua_Number tonumber(const int i = -1);

    void string(const std::string&);
    std::string string(const int i = -1);

    void boolean(const bool);
    bool boolean(const int i = -1);
};

class LuaObject : public LuaClass {
private:
protected:
public:
    static void export_class(Lua& vm);
    static void export_me(Lua& vm);
    static const std::string class_name();
};

/*
 * class LuaObject: public LuaClass {};
 * class P : public LuaClass {};
 * class T : public LuaClass {};
 *
 *  -> T::export_me(vm);
 *      -> export_class<T, P>(vm);
 *          -> T::class_name();
 *          -> P::class_name();
 *              -> P::export_me(vm);
 *                  -> export_class<P>(vm);
 *                      -> P::class_name();
 *                      -> LuaObject::class_name();
 *                          -> LuaObject::export_me(vm);
 *                              -> export_class<LuaObject>(vm);
 *                                  -> LuaObject::class_name();
 *                                  -> LuaObject::class_name();
 *                                  -> LuaObject::export_class(vm);
 *                      -> P::export_class(vm);
 *          -> T::export_class(vm);
 */

template<class T, class P = util::LuaObject>
void export_class(Lua& vm) {

    static_assert(std::is_base_of<util::LuaClass, T>::value,
        "LuaClass implementation expected!");
    static_assert(T::class_name,
        "static const std::string& class_name() member required!");
    static_assert(T::export_class,
        "static void export_class(Lua& vm) member required!");
    static_assert(T::export_me,
        "static void export_me(Lua& vm) member required!");

    auto name = T::class_name();
    auto parent_name = P::class_name();
    bool parent = name != parent_name;

    if (parent) {
        P::export_me(vm);
    }

    vm.load(name);
    if (!vm.is_nil()) {
        vm.pop();
        return;
    }
    vm.pop();

    vm.table();
    vm.table();
    vm.copy(-2);
    vm.save("__index");
    vm.save("mtab");

    if (parent) {
        vm.load(parent_name);
        vm.load("mtab");
        vm.metatable(-3);
        vm.pop();
    }

    T::export_class(vm);
    vm.save(name);
}

namespace lua {

template <class T>
int ret(Lua& vm, const T r) {
    if (std::is_base_of<util::LuaClass, T>::value) {
        static_assert(T::class_name,
            "static const std::string& class_name() member required!");
        vm.object(r, T::class_name());
    } else {
        vm.userdata(r);
    }
    return 1;
}

template <>
int ret<lua_Number>(Lua& vm, const lua_Number r);

template <>
int ret<std::string>(Lua& vm, const std::string r);

template <>
int ret<bool>(Lua& vm, const bool r);

template <class T>
T arg(Lua& vm, const int i) {
    if (std::is_base_of<util::LuaClass, T>::value) {
        return *(T *)vm.object(i);
    } else {
        return *(T *)vm.userdata(i);
    }
}

template <>
std::string arg<std::string>(Lua& vm, const int i);

template <>
lua_Number arg<lua_Number>(Lua& vm, const int i);

template <>
bool arg<bool>(Lua& vm, const int i);

template <typename T, typename... Args>
std::tuple<T, Args...> args(Lua& vm, const int i = 1) {
    return std::tuple_cat(arg<T>(i), args<Args...>(vm, i + 1));
}

template <typename T>
T args(Lua& vm, const int i) {
    return arg<T>(i);
}

template <typename... Args> struct sizer {
    static const int size = sizeof...(Args);
};

template <typename... Args>
std::tuple<> args(Lua& vm, const int i = 1) {
    static_assert(!sizer<Args...>::size, "Arguments provided!");
    return std::tuple<>();
}

} // namespace util::lua

template <typename R, class T, typename... Args>
void export_method(Lua& vm, const std::string& name,
    R (T::*method)(Args...)) {
    auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
        return lua::ret<R>(vm,
            (lua::arg<T>(vm, 1)->*method)(lua::args<Args...>(vm, 2)));
    });
    vm.lambda(function, name);
}

template <class T, typename... Args>
void export_method(Lua& vm, const std::string& name,
    void (T::*method)(Args...)) {
    auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
        (lua::arg<T>(vm, 1)->*method)(lua::args<Args...>(vm, 2));
        return 0;
    });
    vm.lambda(function, name);
}

template <typename R, class T>
void export_method(Lua& vm, const std::string& name, R (T::*method)()) {
    auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
        return lua::ret<R>(vm, (lua::arg<T>(vm, 1)->*method)());
    });
    vm.lambda(function, name);
}

template <class T>
void export_method(Lua& vm, const std::string& name, void (T::*method)()) {
    auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
        (lua::arg<T>(vm, 1)->*method());
        return 0;
    });
    vm.lambda(function, name);
}

template <typename R, typename... Args>
void export_function(Lua& vm, const std::string& name,
    R (*callback)(Args...)) {
    auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
        return lua::ret<R>(vm, (*callback)(lua::args<Args...>(vm, 1)));
    });
    vm.lambda(function, name);
}

template <typename... Args>
void export_function(Lua& vm, const std::string& name,
    void (*callback)(Args...)) {
    auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
        (*callback)(lua::args<Args...>(vm));
        return 0;
    });
    vm.lambda(function, name);
}

template <typename R>
void export_function(Lua& vm, const std::string& name, R (*callback)()) {
    auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
        return lua::ret<R>((*callback)());
    });
    vm.lambda(function, name);
}

void export_function(Lua& vm, const std::string& name, void (*callback)());

} // namespace util;

