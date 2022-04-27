#include "src/parser.h"

int main() {
    std::filesystem::path path{R"(C:\Users\Simon\CLionProjects\sorth\test.sorth)"};
    const auto program = sorth::parse_program(path);
    return 0;
}
