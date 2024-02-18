// STL modules
#include <iostream>

// Custom modules
#include "common/js_interpreter.hpp"
using namespace kc;

int main()
{
    try
    {
        JsInterpreter interpreter;
        interpreter.execute("function fib(n) { return n <= 1 ? n : fib(n - 1) + fib(n - 2); }");
        std::cout << interpreter.execute("console.log(fib(12))") << '\n';

        interpreter.reset();
        interpreter.execute("fib(12)");
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Runtime error: " << error.what() << '\n';
    }
    catch (const std::invalid_argument& error)
    {
        std::cerr << "Invalid argument: " << error.what() << '\n';
    }
    
}
