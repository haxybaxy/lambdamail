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
    std::string response = sendRequest("https://api.mail.tm/domains");
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

std::string MailTM::registerEmail(const std::string& username, const std::string& password) {
    std::string domain = getAvailableDomain();
    if (domain.empty()) return "";

    std::string email = username + "@" + domain;
    Json::Value requestData;
    requestData["address"] = email;
    requestData["password"] = password;

    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, requestData);

    std::string response = sendRequest("https://api.mail.tm/accounts", "POST", payload);
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::istringstream stream(response);
    std::string errors;

    if (Json::parseFromStream(reader, stream, &jsonData, &errors)) {
        if (jsonData["id"].isString()) {
            std::cout << "Email registered successfully: " << email << std::endl;
            return email;
        }
    }
    return "";
}

std::string MailTM::authenticate(const std::string& email, const std::string& password) {
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
        return jsonData["token"].asString();
    }
    return "";
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
