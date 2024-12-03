#include "CurlWrapper.h"

using namespace MailTMAPI;

// Constructor for CurlWrapper that initializes a CURL handle
CurlWrapper::CurlWrapper() : curl(curl_easy_init()), headers(nullptr) {
    if (!curl) { // Check if CURL initialization failed
        throw std::runtime_error("Failed to initialize CURL");
    }
}

// Destructor for CurlWrapper that cleans up allocated resources
CurlWrapper::~CurlWrapper() {
    if (headers) { // Free the header list if it exists
        curl_slist_free_all(headers);
    }
    if (curl) { // Clean up the CURL handle if it was initialized
        curl_easy_cleanup(curl);
    }
}

// Method to set a CURL option with a specific value
void CurlWrapper::setOption(CURLoption option, const void* value) {
    if (curl_easy_setopt(curl, option, value) != CURLE_OK) { // Check if setting the option fails
        throw std::runtime_error("Failed to set CURL option");
    }
}

// Method to set a CURL function option (callback function)
void CurlWrapper::setOption(CURLoption option, size_t (*value)(void*, size_t, size_t, void*)) {
    if (curl_easy_setopt(curl, option, value) != CURLE_OK) { // Check if setting the function option fails
        throw std::runtime_error("Failed to set CURL function option");
    }
}

// Method to add a header to the CURL request
void CurlWrapper::addHeader(const std::string& header) {
    headers = curl_slist_append(headers, header.c_str()); // Append the header to the list
    if (!headers) { // Check if adding the header fails
        throw std::runtime_error("Failed to add CURL header");
    }
}

// Method to clear all headers from the CURL request
void CurlWrapper::clearHeaders() {
    if (headers) { // Free the header list if it exists
        curl_slist_free_all(headers);
        headers = nullptr; // Reset the headers pointer to nullptr
    }
}

// Method to perform the CURL request
CURLcode CurlWrapper::perform() {
    if (headers) { // Set the headers for the CURL request if they exist
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    return curl_easy_perform(curl); // Perform the CURL request
}

// Method to retrieve the HTTP response code from the CURL request
long CurlWrapper::getResponseCode() {
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode); // Get the response code
    return responseCode; // Return the response code
}

// Method to retrieve the response as a string
std::string CurlWrapper::getResponse() {
    char* response;
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &response); // Get the effective URL
    return response ? std::string(response) : ""; // Return the response as a string or an empty string if null
}

// Move constructor for CurlWrapper
CurlWrapper::CurlWrapper(CurlWrapper&& other) noexcept : curl(other.curl), headers(other.headers) {
    other.curl = nullptr; // Reset the moved object's CURL handle
    other.headers = nullptr; // Reset the moved object's headers
}

// Move assignment operator for CurlWrapper
CurlWrapper& CurlWrapper::operator=(CurlWrapper&& other) noexcept {
    if (this != &other) { // Check for self-assignment
        if (headers) { // Free the current header list if it exists
            curl_slist_free_all(headers);
        }
        if (curl) { // Clean up the current CURL handle if it exists
            curl_easy_cleanup(curl);
        }
        curl = other.curl; // Transfer the CURL handle from the other object
        headers = other.headers; // Transfer the headers from the other object
        other.curl = nullptr; // Reset the moved object's CURL handle
        other.headers = nullptr; // Reset the moved object's headers
    }
    return *this; // Return the current object
}
