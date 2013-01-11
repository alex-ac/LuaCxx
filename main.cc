#include <iostream>

#include <Lua.hh>

void test() {
    std::cout << "Hello, world!" << std::endl;
}

int main(int argc, char *argv[]) {
    util::Lua l;

    util::export_function(l, "test", &test);

    l.file("test.lua");

    return 0;
}

