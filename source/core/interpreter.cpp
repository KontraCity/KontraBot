#include "core/interpreter.hpp"

// STL modules
#include <stdexcept>

// Library {fmt}
#include <fmt/format.h>

namespace kb {

void Interpreter::Output(js_State* state)
{
    std::string data;
    int parameters = js_gettop(state);
    for (int index = 1; index < parameters; ++index)
    {
        data += js_tostring(state, index);
        if (index != parameters - 1)
            data += ' ';
    }

    js_getglobal(state, Interpreter::RootPointer);
    reinterpret_cast<Interpreter*>(std::stoull(js_tostring(state, -1)))->m_lastOutput += data;
}

Interpreter::Interpreter()
    : m_state(nullptr, js_freestate)
{
    reset();
}

void Interpreter::updatePointer()
{
    js_dostring(m_state.get(), fmt::format(
        "var {} = \"{}\"",
        RootPointer,
        reinterpret_cast<std::uintptr_t>(this)).c_str()
    );
}

void Interpreter::reset()
{
    std::lock_guard lock(m_mutex);
    m_state.reset(js_newstate(NULL, NULL, 0));
    if (!m_state.get())
        throw std::runtime_error("kb::Interpreter::reset(): Couldn't create interpreter state");

    updatePointer();
    js_newcfunction(m_state.get(), Interpreter::Output, "output", 0);
    js_setglobal(m_state.get(), "output");
    js_dostring(m_state.get(), "var console = { log: output, debug: output, warn: output, error: output };");
}

std::string Interpreter::execute(const std::string& code)
{
    std::lock_guard lock(m_mutex);
    m_lastOutput.clear();
    updatePointer();

    int result = js_ploadstring(m_state.get(), "[stdin]", code.c_str());
    if (result != 0)
    {
        throw std::runtime_error(fmt::format(
            "kb::Interpreter::execute(): "
            "Couldn't compile JavaScript code [return code: {}]",
            result
        ));
    }

    js_pushundefined(m_state.get());
    result = js_pcall(m_state.get(), 0);
    js_pop(m_state.get(), 1);
    if (result != 0)
    {
        throw std::invalid_argument(fmt::format(
            "kb::Interpreter::execute(): "
            "Couldn't execute JavaScript code [return code: {}, message: \"{}\"]",
            result, std::string(js_trystring(m_state.get(), -1, "Error"))
        ));
    }

    return m_lastOutput;
}

} // namespace kb
