#include "MailTM.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

MailTM::MailTM() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

MailTM::~MailTM() {
    curl_global_cleanup();
}

size_t MailTM::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string MailTM::sendRequest(const std::string& url, const std::string& method,
                                const std::string& payload, const std::string& authToken) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (!authToken.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + authToken).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

std::string MailTM::getAvailableDomain() {
    std::string response = sendRequest("https://api.mail.tm/domains", "GET");
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

std::pair<bool, std::string> MailTM::registerEmail(const std::string& email, const std::string& password) {
    Json::Value root;
    root["address"] = email;
    root["password"] = password;

    Json::FastWriter writer;
    std::string jsonStr = writer.write(root);

    std::string response = sendRequest(baseUrl + "/accounts", "POST", jsonStr);

    Json::Value jsonResponse;
    Json::Reader reader;
    if (!reader.parse(response, jsonResponse)) {
        return {false, "Failed to parse response"};
    }

    // Check for errors in the response
    if (jsonResponse.isMember("violations")) {
        std::string errorMsg;
        for (const auto& violation : jsonResponse["violations"]) {
            errorMsg += violation["propertyPath"].asString() + ": " +
                       violation["message"].asString() + "\n";
        }
        return {false, errorMsg};
    }

    if (jsonResponse.isMember("id")) {
        return {true, "Success"};
    }

    return {false, "Unknown error occurred"};
}

std::pair<bool, std::string> MailTM::authenticate(const std::string& email, const std::string& password) {
    Json::Value requestData;
    requestData["address"] = email;
    requestData["password"] = password;

    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, requestData);

    std::string response = sendRequest("https://api.mail.tm/token", "POST", payload);
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::istringstream stream(response);
    std::string errors;

    if (Json::parseFromStream(reader, stream, &jsonData, &errors)) {
        if (jsonData.isMember("token")) {
            return {true, jsonData["token"].asString()};
        } else if (jsonData.isMember("detail")) {
            return {false, jsonData["detail"].asString()};
        }
    }
    return {false, "Authentication failed"};
}

std::vector<Json::Value> MailTM::checkInbox(const std::string& token) {
    std::string response = sendRequest("https://api.mail.tm/messages", "GET", "", token);
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

std::pair<bool, std::string> MailTM::deleteAccount(const std::string& token, const std::string& accountId) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {false, "Failed to initialize CURL"};
    }

    std::string url = this->baseUrl + "/accounts/" + accountId;
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return {false, curl_easy_strerror(res)};
    }

    if (http_code == 204) {
        return {true, "Account deleted successfully"};
    } else {
        return {false, "Unexpected response code: " + std::to_string(http_code)};
    }
}

std::pair<bool, std::string> MailTM::getAccountId(const std::string& token) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {false, "Failed to initialize CURL"};
    }

    std::string url = this->baseUrl + "/me";
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return {false, curl_easy_strerror(res)};
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(response, root)) {
        return {false, "Failed to parse response"};
    }

    return {true, root["id"].asString()};
}

Json::Value MailTM::getMessage(const std::string& token, const std::string& messageId) {
    std::string response = sendRequest(baseUrl + "/messages/" + messageId, "GET", "", token);

    Json::Value jsonResponse;
    Json::Reader reader;
    if (!reader.parse(response, jsonResponse)) {
        return Json::Value();
    }

    return jsonResponse;
}
