#pragma once

// STL modules
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

// Library {fmt}
#include <fmt/format.h>

// Library Curl
#include <curl/curl.h>

namespace kc {

namespace Curl
{
    struct Response
    {
        long code = -1;
        std::string headers;
        std::string data;
    };

    /// @brief Perform HTTP GET request
    /// @param url URL to access
    /// @param headers Request headers
    /// @throw std::runtime_error if internal error occurs
    /// @throw std::invalid_argument if request couldn't be performed
    /// @return Request response
    Response Get(const std::string& url, const std::vector<std::string>& headers = {});

    /// @brief Perform HTTP POST request
    /// @param url URL to access
    /// @param headers Request headers
    /// @param data Request POST data
    /// @throw std::runtime_error if internal error occurs
    /// @throw std::invalid_argument if request couldn't be performed
    /// @return Request response
    Response Post(const std::string& url, const std::vector<std::string>& headers, const std::string& data);
}

} // namespace kc
