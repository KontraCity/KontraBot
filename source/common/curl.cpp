#include "common/curl.hpp"

namespace kc {

static size_t StringWriter(uint8_t* data, size_t itemSize, size_t itemCount, std::string* target)
{
    target->insert(target->end(), data, data + itemCount);
    return (itemSize * itemCount);
}

static Curl::Response Request(const std::string& url, const std::vector<std::string>& headers, const std::string& data)
{
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl.get())
        throw std::runtime_error("kc::Curl::Request(): Couldn't initialize Curl");

    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> slist(nullptr, curl_slist_free_all);
    for (const std::string& header : headers)
    {
        curl_slist* oldPointer = slist.release();
        curl_slist* newPointer = curl_slist_append(oldPointer, header.c_str());
        if (!newPointer)
        {
            curl_slist_free_all(oldPointer);
            throw std::runtime_error("kc::Curl::Request(): Couldn't read request headers");
        }
        slist.reset(newPointer);
    }

    Curl::Response response;
    CURLcode result = curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    if (result != CURLE_OK)
        throw std::runtime_error("kc::Curl::Request(): Couldn't configure request URL");

    result = curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &response.headers);
    if (result != CURLE_OK)
        throw std::runtime_error("kc::Curl::Request(): Couldn't configure headers write target");

    result = curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, &StringWriter);
    if (result != CURLE_OK)
        throw std::runtime_error("kc::Curl::Request(): Couldn't configure headers write function");

    result = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.data);
    if (result != CURLE_OK)
        throw std::runtime_error("kc::Curl::Request(): Couldn't configure data write target");

    result = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &StringWriter);
    if (result != CURLE_OK)
        throw std::runtime_error("kc::Curl::Request(): Couldn't configure data write function");

    if (!headers.empty())
    {
        result = curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, slist.get());
        if (result != CURLE_OK)
            throw std::runtime_error("kc::Curl::Request(): Couldn't configure request headers");
    }

    if (!data.empty())
    {
        result = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, data.c_str());
        if (result != CURLE_OK)
            throw std::runtime_error("kc::Curl::Request(): Couldn't configure request POST data");
    }

    result = curl_easy_perform(curl.get());
    if (result != CURLE_OK)
        throw std::invalid_argument("kc::Curl::Request(): Couldn't perform request");

    result = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.code);
    if (result != CURLE_OK)
        throw std::runtime_error("kc::Curl::Request(): Couldn't retrieve response code");

    return response;
}

Curl::Response Curl::Get(const std::string& url, const std::vector<std::string>& headers)
{
    return Request(url, headers, "");
}

Curl::Response Curl::Post(const std::string& url, const std::vector<std::string>& headers, const std::string& data)
{
    return Request(url, headers, data);
}

} // namespace kc
