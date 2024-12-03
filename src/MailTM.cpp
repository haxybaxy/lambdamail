#include "MailTM.h"
#include "CurlWrapper.h"
#include <sstream>
#include <iostream>
#include "CurlWrapper.h"

using namespace MailTMAPI;

// Callback function to write CURL response data into a string
size_t MailTM::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to send a network request using the specified URL, method, payload, and optional authentication token
std::string MailTM::sendRequest(const std::string& url, const std::string& method,
                                const std::string& payload, const std::string& authToken) {
    try {
        CurlWrapper curl; // Initialize CURL wrapper
        std::string response;

        // Set CURL options
        curl.setOption(CURLOPT_URL, url.c_str());
        curl.setOption(CURLOPT_WRITEFUNCTION, WriteCallback); // Set the write callback function
        curl.setOption(CURLOPT_WRITEDATA, &response);

        // Add headers if an authentication token is provided
        if (!authToken.empty()) {
            curl.addHeader("Authorization: Bearer " + authToken);
        }
        curl.addHeader("Content-Type: application/json");

        // Set method-specific options
        if (method == "POST") {
            curl.setOption(CURLOPT_POSTFIELDS, payload.c_str());
        } else if (method == "DELETE") {
            curl.setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
        }

        // Perform the CURL request
        CURLcode res = curl.perform();
        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        }

        return response; // Return the response as a string
    } catch (const std::exception& e) {
        std::cerr << "Error in sendRequest: " << e.what() << std::endl;
        return ""; // Return an empty string on error
    }
}

// Function to fetch an available domain for email creation
std::string MailTM::getAvailableDomain() {
    std::string response = sendRequest(baseUrl + "/domains", "GET");
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::istringstream stream(response);
    std::string errors;

    // Parse the response JSON
    if (Json::parseFromStream(reader, stream, &jsonData, &errors)) {
        if (!jsonData["hydra:member"].empty()) { // Check if domains are available
            return jsonData["hydra:member"][0]["domain"].asString(); // Return the first domain
        }
    }
    return ""; // Return an empty string if no domain is found
}

// Function to register an email address with a password
std::optional<std::string> MailTM::registerEmail(const std::string& email, const std::string& password) {
    Json::Value root;
    root["address"] = email; // Set email address in JSON payload
    root["password"] = password; // Set password in JSON payload

    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, root);

    // Send a POST request to create the account
    std::string response = sendRequest(baseUrl + "/accounts", "POST", jsonStr);

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::string errors;

    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &jsonResponse, &errors)) { // Parse the response JSON
        std::cerr << "Failed to parse registerEmail response: " << errors << std::endl;
        return std::nullopt;
    }

    if (jsonResponse.isMember("id")) { // Check if the response contains an account ID
        return jsonResponse["id"].asString(); // Return the account ID
    }

    return std::nullopt; // Return nullopt if registration fails
}

// Function to authenticate a user and retrieve a token
std::optional<std::string> MailTM::authenticate(const std::string& email, const std::string& password) {
    Json::Value payload;
    payload["address"] = email; // Set email address in JSON payload
    payload["password"] = password; // Set password in JSON payload

    Json::StreamWriterBuilder writer;
    std::string payloadStr = Json::writeString(writer, payload);

    try {
        // Send a POST request to authenticate
        std::string response = sendRequest(baseUrl + "/token", "POST", payloadStr);
        Json::CharReaderBuilder reader;
        Json::Value jsonResponse;
        std::string errors;

        std::istringstream ss(response);
        if (!Json::parseFromStream(reader, ss, &jsonResponse, &errors)) { // Parse the response JSON
            throw std::runtime_error("Failed to parse authentication response: " + errors);
        }

        if (jsonResponse.isMember("token")) { // Check if the response contains a token
            return jsonResponse["token"].asString(); // Return the token
        }
        return std::nullopt; // Return nullopt if authentication fails
    } catch (const std::exception& e) {
        std::cerr << "Authentication error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// Function to fetch messages from the inbox
std::vector<Json::Value> MailTM::checkInbox(const std::string& token) {
    std::string response = sendRequest(baseUrl + "/messages", "GET", "", token);
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::istringstream stream(response);
    std::string errors;
    std::vector<Json::Value> messages;

    // Parse the response JSON and extract messages
    if (Json::parseFromStream(reader, stream, &jsonData, &errors)) {
        for (const auto& message : jsonData["hydra:member"]) {
            messages.push_back(message);
        }
    }
    return messages; // Return the list of messages
}

// Function to delete an account
std::optional<std::string> MailTM::deleteAccount(const std::string& token, const std::string& accountId) {
    std::string url = baseUrl + "/accounts/" + accountId; // Construct the URL for account deletion
    std::string response = sendRequest(url, "DELETE", "", token);

    if (response.empty()) { // Check if the response indicates success
        return "Account deleted successfully"; // Return success message
    }

    return std::nullopt; // Return nullopt if deletion fails
}

// Function to fetch the account ID
std::optional<std::string> MailTM::getAccountId(const std::string& token) {
    std::string response = sendRequest(baseUrl + "/me", "GET", "", token);

    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errors;

    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &root, &errors)) { // Parse the response JSON
        std::cerr << "Failed to parse getAccountId response: " << errors << std::endl;
        return std::nullopt;
    }

    if (root.isMember("id")) { // Check if the response contains an account ID
        return root["id"].asString(); // Return the account ID
    }

    return std::nullopt; // Return nullopt if account ID is not found
}

// Function to fetch a specific message by ID
Json::Value MailTM::getMessage(const std::string& token, const std::string& messageId) {
    std::string response = sendRequest(baseUrl + "/messages/" + messageId, "GET", "", token);

    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;

    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &jsonResponse, &errors)) { // Parse the response JSON
        return Json::Value(); // Return an empty JSON value on error
    }

    return jsonResponse; // Return the full message as a JSON value
}
