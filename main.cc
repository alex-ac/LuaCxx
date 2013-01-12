#include <iostream>

#include <Lua.hh>

void test() {
    std::cout << "Hello, world! " << std::endl;
}

void test1(int a) {
    std::cout << a << std::endl;
}

int test2() {
    return 2;
}

int test3(int a) {
    return a + 1;
}

class test_class : public util::LuaClass {
public:
    static test_class *construct() {
        return new test_class();
    }

    void deconstruct() {
        delete this;
    }

    static void export_me(util::Lua& vm) {
        util::export_class<test_class>(vm);
    }

    static void export_class(util::Lua& vm) {
        util::export_function(vm, "new", &test_class::construct);
        util::export_method(vm, "delete", &test_class::deconstruct);
        util::export_method(vm, "test", &test_class::test);
        util::export_method(vm, "test1", &test_class::test1);
        util::export_method(vm, "test2", &test_class::test2);
        util::export_method(vm, "test3", &test_class::test3);
    }

    static const std::string class_name() {
        return "test_class";
    }

    virtual const  std::string obj_class_name() const override {
        return "test_class";
    }

    void test() {
        std::cout << "Hello, world [test_class]" << std::endl;
    }

    int test1() {
        return 1;
    }

    int test2(int a) {
        return a*2;
    }

    void test3(int a) {
        std::cout << a << std::endl;
    }
};

int main(int argc, char *argv[]) {
    util::Lua l;

    util::export_function(l, "test", &test);
    util::export_function(l, "test1", &test1);
    util::export_function(l, "test2", &test2);
    util::export_function(l, "test3", &test3);

    test_class::export_me(l);

    l.file("test.lua");

    return 0;
}

