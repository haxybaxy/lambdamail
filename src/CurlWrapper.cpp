#include "CurlWrapper.h"

CurlWrapper::CurlWrapper() : curl(curl_easy_init()), headers(nullptr) {
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

CurlWrapper::~CurlWrapper() {
    if (headers) {
        curl_slist_free_all(headers);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

void CurlWrapper::setOption(CURLoption option, const void* value) {
    if (curl_easy_setopt(curl, option, value) != CURLE_OK) {
        throw std::runtime_error("Failed to set CURL option");
    }
}

void CurlWrapper::setOption(CURLoption option, size_t (*value)(void*, size_t, size_t, void*)) {
    if (curl_easy_setopt(curl, option, value) != CURLE_OK) {
        throw std::runtime_error("Failed to set CURL function option");
    }
}

void CurlWrapper::addHeader(const std::string& header) {
    headers = curl_slist_append(headers, header.c_str());
    if (!headers) {
        throw std::runtime_error("Failed to add CURL header");
    }
}

void CurlWrapper::clearHeaders() {
    if (headers) {
        curl_slist_free_all(headers);
        headers = nullptr;
    }
}

CURLcode CurlWrapper::perform() {
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    return curl_easy_perform(curl);
}

long CurlWrapper::getResponseCode() {
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    return responseCode;
}

std::string CurlWrapper::getResponse() {
    char* response;
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &response);
    return response ? std::string(response) : "";
}

CurlWrapper::CurlWrapper(CurlWrapper&& other) noexcept : curl(other.curl), headers(other.headers) {
    other.curl = nullptr;
    other.headers = nullptr;
}

CurlWrapper& CurlWrapper::operator=(CurlWrapper&& other) noexcept {
    if (this != &other) {
        if (headers) {
            curl_slist_free_all(headers);
        }
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl = other.curl;
        headers = other.headers;
        other.curl = nullptr;
        other.headers = nullptr;
    }
    return *this;
}
