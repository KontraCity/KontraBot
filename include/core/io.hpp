#pragma once

#include <string>

namespace kb {

namespace IO {
    void WriteFile(const std::string& fileName, const std::string& contents);

    std::string ReadFile(const std::string& fileName);
}

} // namespace kb
