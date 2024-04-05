// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/interpreter.hpp"
using namespace kc;

int main()
{
    try
    {
        Interpreter interpreter;
        interpreter.execute("var a = 9;");
        interpreter.execute("var b = 10");
        interpreter.execute("var result = a + b;");
        std::cout << fmt::format("Execution result: {}\n", interpreter.execute("console.log(result);"));

        interpreter.reset();
        interpreter.execute("console.log(result);");
    }
    catch (const std::runtime_error& error)
    {
        std::cout << fmt::format("Internal error: {}\n", error.what());
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << fmt::format("Code execution error: {}\n", error.what());
    }
}
