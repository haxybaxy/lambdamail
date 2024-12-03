#pragma once
#include <curl/curl.h>
#include <string>
#include <stdexcept>

/**
 * @file CurlWrapper.h
 * @brief Provides a wrapper around the CURL library for HTTP requests.
 */

namespace MailTMAPI {

/**
 * @class CurlWrapper
 * @brief A wrapper for CURL to simplify HTTP requests.
 *
 * This class encapsulates CURL initialization, option setting, and cleanup.
 * It provides methods to set headers, perform HTTP requests, and retrieve responses.
 */
class CurlWrapper {
private:
    CURL* curl; /**< Pointer to the CURL instance. */
    struct curl_slist* headers; /**< Linked list of custom headers. */

public:
    /**
     * @brief Constructs a CurlWrapper instance and initializes CURL.
     * @throws std::runtime_error if CURL initialization fails.
     */
    CurlWrapper();

    /**
     * @brief Destructor to clean up CURL resources.
     */
    ~CurlWrapper();

    /**
     * @brief Sets a CURL option.
     * @param option The CURL option to set.
     * @param value The value for the option.
     * @throws std::runtime_error if the option cannot be set.
     */
    void setOption(CURLoption option, const void* value);

    /**
     * @brief Sets a CURL option for function pointers.
     * @param option The CURL option to set.
     * @param value The function pointer for the option.
     * @throws std::runtime_error if the option cannot be set.
     */
    void setOption(CURLoption option, size_t (*value)(void*, size_t, size_t, void*));

    /**
     * @brief Adds a custom header to the HTTP request.
     * @param header The header string (e.g., "Content-Type: application/json").
     * @throws std::runtime_error if the header cannot be added.
     */
    void addHeader(const std::string& header);

    /**
     * @brief Clears all custom headers.
     */
    void clearHeaders();

    /**
     * @brief Executes the HTTP request.
     * @return CURLcode indicating the result of the request.
     */
    CURLcode perform();

    /**
     * @brief Gets the HTTP response code.
     * @return The HTTP response code.
     */
    long getResponseCode();

    /**
     * @brief Gets the effective URL from the response.
     * @return The effective URL as a string.
     */
    std::string getResponse();

    // Disable copy semantics
    CurlWrapper(const CurlWrapper&) = delete;
    CurlWrapper& operator=(const CurlWrapper&) = delete;

    // Enable move semantics
    CurlWrapper(CurlWrapper&& other) noexcept;
    CurlWrapper& operator=(CurlWrapper&& other) noexcept;
};

} // namespace MailTMAPI
