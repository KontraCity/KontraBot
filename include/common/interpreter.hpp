#pragma once

// STL modules
#include <string>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>

// Library MuJS
#include <mujs.h>

// Library {fmt}
#include <fmt/format.h>

namespace kc {

class Interpreter
{
private:
    // JavaScript interpreter root pointer variable name
    static constexpr const char* RootPointer = "_RootPointer";

private:
    /// @brief Output data from JavaScript to C++
    /// @param state Interpreter state
    static void Output(js_State* state);

private:
    std::mutex m_mutex;
    std::unique_ptr<js_State, decltype(&js_freestate)> m_state;
    std::string m_lastOutput;

private:
    /// @brief Update interpreter pointer
    void updatePointer();

public:
    /// @brief Initialize JavaScript interpreter
    /// @throw std::runtime_error if internal error occurs
    Interpreter();

    /// @brief Reset interpreter context
    /// @throw std::runtime_error if internal error occurs
    void reset();

    /// @brief Execute JavaScript code
    /// @param code JavaScript code to execute
    /// @throw std::runtime_error if internal error occurs
    /// @throw std::invalid_argument if code execution error occurs
    /// @return Execution output
    std::string execute(const std::string& code);
};

} // namespace kc
