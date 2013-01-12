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
    static void export_me(util::Lua& vm) {
        vm.export_class<test_class>();
    }

    static void export_class(util::Lua& vm) {
        vm.export_constructor<test_class>();
        vm.export_method("test", &test_class::test);
        vm.export_method("test1", &test_class::test1);
        vm.export_method("test2", &test_class::test2);
        vm.export_method("test3", &test_class::test3);
    }

    static const std::string class_name() {
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

    l.export_function("test", &test);
    l.export_function("test1", &test1);
    l.export_function("test2", &test2);
    l.export_function("test3", &test3);

    test_class::export_me(l);

    l.file("test.lua");

    return 0;
}

