#pragma once
#include <curl/curl.h>
#include <string>
#include <stdexcept>

class CurlWrapper {
private:
    CURL* curl;
    struct curl_slist* headers;

public:
    CurlWrapper();
    ~CurlWrapper();

    void setOption(CURLoption option, const void* value);
    void setOption(CURLoption option, size_t (*value)(void*, size_t, size_t, void*)); // Overload for function pointers
    void addHeader(const std::string& header);
    void clearHeaders();

    CURLcode perform();
    long getResponseCode();
    std::string getResponse();

    // Disable copy semantics
    CurlWrapper(const CurlWrapper&) = delete;
    CurlWrapper& operator=(const CurlWrapper&) = delete;

    // Enable move semantics
    CurlWrapper(CurlWrapper&& other) noexcept;
    CurlWrapper& operator=(CurlWrapper&& other) noexcept;
};
