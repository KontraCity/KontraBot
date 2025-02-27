#include "core/io.hpp"

#include <fstream>
#include <sstream>

#include "core/error.hpp"

namespace kb {
    
void IO::WriteFile(const std::string& fileName, const std::string& contents) {
    std::ofstream file(fileName, std::ios::trunc);
    if (!file)
        throw KB_LOCATED_FORMATTED_ERROR("Couldn't create \"{}\" file", fileName);
    file << contents;
}

std::string IO::ReadFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file)
        throw KB_LOCATED_FORMATTED_ERROR("Couldn't open \"{}\" file", fileName);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace kb
