#include <iostream>

#include <Lua.hh>

void test() {
    std::cout << "Hello, world! " << std::endl;
}

void test1(int a) {
    std::cout << a << std::endl;
}

int main(int argc, char *argv[]) {
    util::Lua l;

    util::export_function(l, "test", &test);
    util::export_function(l, "test1", &test1);

    l.file("test.lua");

    return 0;
}

