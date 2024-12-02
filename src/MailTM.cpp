#include "MailTM.h"
#include "CurlWrapper.h"
#include <sstream>
#include <iostream>
#include "CurlWrapper.h"

using namespace MailTMAPI;

size_t MailTM::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string MailTM::sendRequest(const std::string& url, const std::string& method,
                                const std::string& payload, const std::string& authToken) {
    try {
        CurlWrapper curl;
        std::string response;

        curl.setOption(CURLOPT_URL, url.c_str());
        curl.setOption(CURLOPT_WRITEFUNCTION, WriteCallback); // Use the overloaded function
        curl.setOption(CURLOPT_WRITEDATA, &response);

        if (!authToken.empty()) {
            curl.addHeader("Authorization: Bearer " + authToken);
        }
        curl.addHeader("Content-Type: application/json");

        if (method == "POST") {
            curl.setOption(CURLOPT_POSTFIELDS, payload.c_str());
        } else if (method == "DELETE") {
            curl.setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
        }

        CURLcode res = curl.perform();
        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        }

        return response;
    } catch (const std::exception& e) {
        std::cerr << "Error in sendRequest: " << e.what() << std::endl;
        return "";
    }
}
std::string MailTM::getAvailableDomain() {
    std::string response = sendRequest(baseUrl + "/domains", "GET");
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::istringstream stream(response);
    std::string errors;

    if (Json::parseFromStream(reader, stream, &jsonData, &errors)) {
        if (!jsonData["hydra:member"].empty()) {
            return jsonData["hydra:member"][0]["domain"].asString();
        }
    }
    return "";
}

std::optional<std::string> MailTM::registerEmail(const std::string& email, const std::string& password) {
    Json::Value root;
    root["address"] = email;
    root["password"] = password;

    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, root);

    std::string response = sendRequest(baseUrl + "/accounts", "POST", jsonStr);

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::string errors;

    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &jsonResponse, &errors)) {
        std::cerr << "Failed to parse registerEmail response: " << errors << std::endl;
        return std::nullopt;
    }

    if (jsonResponse.isMember("id")) {
        return jsonResponse["id"].asString();
    }

    return std::nullopt;
}

std::optional<std::string> MailTM::authenticate(const std::string& email, const std::string& password) {
    Json::Value payload;
    payload["address"] = email;
    payload["password"] = password;

    Json::StreamWriterBuilder writer;
    std::string payloadStr = Json::writeString(writer, payload);

    try {
        std::string response = sendRequest(baseUrl + "/token", "POST", payloadStr);
        Json::CharReaderBuilder reader;
        Json::Value jsonResponse;
        std::string errors;

        std::istringstream ss(response);
        if (!Json::parseFromStream(reader, ss, &jsonResponse, &errors)) {
            throw std::runtime_error("Failed to parse authentication response: " + errors);
        }

        if (jsonResponse.isMember("token")) {
            return jsonResponse["token"].asString();
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Authentication error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::vector<Json::Value> MailTM::checkInbox(const std::string& token) {
    std::string response = sendRequest(baseUrl + "/messages", "GET", "", token);
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::istringstream stream(response);
    std::string errors;
    std::vector<Json::Value> messages;

    if (Json::parseFromStream(reader, stream, &jsonData, &errors)) {
        for (const auto& message : jsonData["hydra:member"]) {
            messages.push_back(message);
        }
    }
    return messages;
}

std::optional<std::string> MailTM::deleteAccount(const std::string& token, const std::string& accountId) {
    std::string url = baseUrl + "/accounts/" + accountId;
    std::string response = sendRequest(url, "DELETE", "", token);

    if (response.empty()) {
        return "Account deleted successfully";
    }

    return std::nullopt;
}

std::optional<std::string> MailTM::getAccountId(const std::string& token) {
    std::string response = sendRequest(baseUrl + "/me", "GET", "", token);

    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errors;

    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &root, &errors)) {
        std::cerr << "Failed to parse getAccountId response: " << errors << std::endl;
        return std::nullopt;
    }

    if (root.isMember("id")) {
        return root["id"].asString();
    }

    return std::nullopt;
}

Json::Value MailTM::getMessage(const std::string& token, const std::string& messageId) {
    std::string response = sendRequest(baseUrl + "/messages/" + messageId, "GET", "", token);

    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;

    std::istringstream stream(response);
    if (!Json::parseFromStream(reader, stream, &jsonResponse, &errors)) {
        return Json::Value();
    }

    return jsonResponse;
}
