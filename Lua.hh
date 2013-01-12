#pragma once

#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <type_traits>

#include <iostream>

extern "C" {
#include <lua.h>
};

namespace util {

class LuaClass {
private:
protected:
public:
    virtual const std::string obj_class_name() const = 0;
};

class Lua {
protected:
    template <typename T>
    int ret(const T r) {
        static_assert(std::is_convertible<T, LuaClass*>::value,
            "LuaClass * required!");
        object((LuaClass *)r, std::remove_pointer<T>::type::class_name());
        return 1;
    }
    template <class T>
    T arg(const int i) {
        if (std::is_base_of<util::LuaClass, T>::value) {
            return *(T *)object(i);
        } else {
            return *(T *)userdata(i);
        }
    }

    template <class T>
    T* argp(const int i) {
        if (std::is_base_of<util::LuaClass, T>::value) {
            return (T *)object(i);
        } else {
            return (T *)userdata(i);
        }
    }

    template <typename T, typename T1, typename... Args>
    std::tuple<T, T1, Args...> args(const int i = 1) {
        T t = arg<T>( i);
        return std::tuple_cat(t, args<T1, Args...>( i + 1));
    }

    template <typename T>
    std::tuple<T> args(const int i = 1) {
        return std::tuple<T>(arg<T>( i));
    }

    template <typename... Args> struct sizer {
        static const int size = sizeof...(Args);
    };

    template <int N> struct apply_method {
        template <class T, typename R, typename... MethodArgs,
            typename... TupleArgs, typename... Args>
        static R apply(T* o, R (T::*method)(MethodArgs...),
            std::tuple<TupleArgs...>& t, Args... args) {
            return
                apply_method<N-1>::apply(o, method, t, std::get<N-1>(t), args...);
        }
    };

    template <int N> struct apply_function {
        template <typename R, typename... FunctionArgs, typename... TupleArgs,
            typename... Args>
        static R apply(R (*function)(FunctionArgs...),
            std::tuple<TupleArgs...>& t, Args... args) {
            return
                apply_function<N-1>::apply(function, t, std::get<N-1>(t), args...);
        }
    };

private:
    bool del;
    lua_State * vm;
    std::vector<std::function<int(Lua&)> *> lambdas;

    static int call(lua_State *vm);
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

    class LuaObject : public LuaClass {
    private:
    protected:
    public:
        static void export_class(Lua& vm);
        static void export_me(Lua& vm);
        static const std::string class_name();
        virtual const std::string obj_class_name() const override;
    };
    /*
     * class LuaObject: public LuaClass {};
     * class P : public LuaClass {};
     * class T : public LuaClass {};
     *
     *  -> T::export_me(;
     *      -> export_class<T, P>();
     *          -> T::class_name();
     *          -> P::class_name();
     *              -> P::export_me(;
     *                  -> export_class<P>();
     *                      -> P::class_name();
     *                      -> LuaObject::class_name();
     *                          -> LuaObject::export_me(;
     *                              -> export_class<LuaObject>();
     *                                  -> LuaObject::class_name();
     *                                  -> LuaObject::class_name();
     *                                  -> LuaObject::export_class(;
     *                      -> P::export_class(;
     *          -> T::export_class(;
     */

    template<class T, class P = LuaObject>
    void export_class() {
        static_assert(std::is_base_of<util::LuaClass, T>::value,
            "LuaClass implementation expected!");

        auto name = T::class_name();
        auto parent_name = P::class_name();
        bool parent = name != parent_name;

        if (parent) {
            P::export_me(*this);
        }

        load(name);
        if (!is_nil()) {
            pop();
            return;
        }
        pop();

        table();
        table();
        copy(-2);
        save("__index");
        save("mtab");

        if (parent) {
            load(parent_name);
            load("mtab");
            metatable(-3);
            pop();
        }

        T::export_class(*this);
        save(name);
    }


    template <typename R, class T, typename... Args>
    void export_method(const std::string& name,
        R (T::*method)(Args...)) {
        auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
            auto tuple = vm.args<Args...>( 2);
            return vm.ret( 
                apply_method<std::tuple_size<decltype(tuple)>::value>
                    ::apply(vm.argp<T>( 1), method, tuple));
        });
        lambda(function, name);
    }

    template <class T, typename... Args>
    void export_method(const std::string& name,
        void (T::*method)(Args...)) {
        auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
            auto tuple = vm.args<Args...>( 2);
            apply_method<std::tuple_size<decltype(tuple)>::value>
                ::apply(vm.argp<T>( 1), method, tuple);
            return 0;
        });
        lambda(function, name);
    }

    template <typename R, class T>
    void export_method(const std::string& name, R (T::*method)()) {
        auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
            return vm.ret( (vm.argp<T>( 1)->*method)());
        });
        lambda(function, name);
    }

    template <class T>
    void export_method(const std::string& name, void (T::*method)()) {
        auto function = new std::function<int(Lua&)>([method] (Lua& vm) -> int {
            (vm.argp<T>( 1)->*method)();
            return 0;
        });
        lambda(function, name);
    }

    template <typename R, typename... Args>
    void export_function(const std::string& name,
        R (*callback)(Args...)) {
        auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
            auto tuple = vm.args<Args...>();
            return vm.ret(
                apply_function<std::tuple_size<decltype(tuple)>::value>
                    ::apply(callback, tuple));
        });
        lambda(function, name);
    }

    template <typename... Args>
    void export_function(const std::string& name,
        void (*callback)(Args...)) {
        auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
            auto tuple = vm.args<Args...>();
            apply_function<std::tuple_size<decltype(tuple)>::value>
                ::apply(callback, tuple);
            return 0;
        });
        lambda(function, name);
    }

    template <typename R>
    void export_function(const std::string& name, R (*callback)()) {
        auto function = new std::function<int(Lua&)>([callback] (Lua& vm) -> int {
            return vm.ret( (*callback)());
        });
        lambda(function, name);
    }

    void export_function(const std::string& name, void (*callback)());
};

template <>
int Lua::ret<lua_Number>(const lua_Number r);

template <>
int Lua::ret<std::string>(const std::string r);

template <>
int Lua::ret<bool>(const bool r);

template <>
int Lua::ret<int>(const int r);

template <>
std::string Lua::arg<std::string>(const int i);

template <>
lua_Number Lua::arg<lua_Number>(const int i);

template <>
lua_Integer Lua::arg<lua_Integer>(const int i);

template <>
int Lua::arg<int>(const int i);

template <>
bool Lua::arg<bool>(const int i);

template <> struct Lua::apply_method<0> {
    template <class T, typename R, typename... MethodArgs,
        typename... TupleArgs, typename... Args>
    static R apply(T* o, R (T::*method)(MethodArgs...),
        std::tuple<TupleArgs...>& t, Args... args) {
        return (o->*method)(args...);
    }
};

template <> struct Lua::apply_function<0> {
    template <typename R, typename... FunctionArgs, typename... TupleArgs,
        typename... Args>
    static R apply(R (*function)(FunctionArgs...),
        std::tuple<TupleArgs...>& t, Args... args) {
        return
            (*function)(args...);
    }
};

} // namespace util;

