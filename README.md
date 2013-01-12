LuaCxx
======

C++ wrapper for lua interpretator.

Install
=======

    cmake . && make && make install

Using
=====

Include Lua.hh.

Create interpretator instance (util::Lua).

Add util::LuaClass base class to your one.
Add static void export_me(Lua& vm) method to your class.
It must looks like this one:

    static void SomeClass::export_me(Lua& vm) {
        vm.export_class<SomeClass, SomeBaseClass>();
    }

Add static void export_class(Lua& vm) method to your class.
It must looks like this one:

    static void SomeClass::export_class(Lua& vm) {
        vm.export_function("static_method", &SomeClass::static_method);
        vm.export_method("simple_method", &SomeClass::simple_method);
    }

Add static const std::string class_name() method to your
class.

    static const std::string SomeClass::class_name() {
        return "SomeClass";
    }

Export prepared classes by export_me methods.
Export functions and static methods by util::Lua::export_function method.
Export methods by util::Lua::export_method method.

