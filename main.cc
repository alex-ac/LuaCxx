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

int main(int argc, char *argv[]) {
    util::Lua l;

    util::export_function(l, "test", &test);
    util::export_function(l, "test1", &test1);
    util::export_function(l, "test2", &test2);
    util::export_function(l, "test3", &test3);

    l.file("test.lua");

    return 0;
}

