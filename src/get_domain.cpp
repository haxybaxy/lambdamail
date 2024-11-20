#include <iostream>
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <string>

// Helper function to handle the response data from curl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to authenticate and get a token
std::string authenticate(const std::string& email, const std::string& password) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL for authentication." << std::endl;
        return "";
    }

    std::string url = "https://api.mail.tm/token";
    std::string readBuffer;

    Json::Value requestData;
    requestData["address"] = email;
    requestData["password"] = password;

    Json::StreamWriterBuilder writer;
    std::string requestBody = Json::writeString(writer, requestData);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);

    std::string token = "";
    if (res != CURLE_OK) {
        std::cerr << "CURL error during authentication: " << curl_easy_strerror(res) << std::endl;
    } else {
        // Parse JSON response
        Json::CharReaderBuilder readerBuilder;
        Json::Value jsonData;
        std::string errors;
        std::istringstream responseStream(readBuffer);

        if (Json::parseFromStream(readerBuilder, responseStream, &jsonData, &errors)) {
            token = jsonData["token"].asString();
        } else {
            std::cerr << "Error parsing authentication response: " << errors << std::endl;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return token;
}

// Function to check inbox messages
void checkInbox(const std::string& token) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL for inbox check." << std::endl;
        return;
    }

    std::string url = "https://api.mail.tm/messages";
    std::string readBuffer;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error during inbox check: " << curl_easy_strerror(res) << std::endl;
    } else {
        // Parse JSON response
        Json::CharReaderBuilder readerBuilder;
        Json::Value jsonData;
        std::string errors;
        std::istringstream responseStream(readBuffer);

        if (Json::parseFromStream(readerBuilder, responseStream, &jsonData, &errors)) {
            if (jsonData["hydra:member"].empty()) {
                std::cout << "No messages in inbox." << std::endl;
            } else {
                for (const auto& message : jsonData["hydra:member"]) {
                    std::cout << "From: " << message["from"]["address"].asString() << std::endl;
                    std::cout << "Subject: " << message["subject"].asString() << std::endl;
                    std::cout << "Preview: " << message["intro"].asString() << std::endl;
                    std::cout << "-------------------------" << std::endl;
                }
            }
        } else {
            std::cerr << "Error parsing inbox response: " << errors << std::endl;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

// Main function
int main() {
    std::string email, password;

    // Prompt the user for email and password
    std::cout << "Enter your email address: ";
    std::cin >> email;

    std::cout << "Enter your password: ";
    std::cin >> password;

    // Authenticate to get a token
    std::string token = authenticate(email, password);
    if (!token.empty()) {
        std::cout << "Authentication successful!" << std::endl;

        // Check the inbox
        checkInbox(token);
    } else {
        std::cerr << "Authentication failed. Exiting." << std::endl;
    }

    return 0;
}
