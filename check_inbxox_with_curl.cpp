#include <iostream>
#include <curl/curl.h>
#include <json/json.h>

// Helper function to write CURL response to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

// Function to authenticate and retrieve the token
std::string authenticate(const std::string& email, const std::string& password) {
    CURL* curl = curl_easy_init();
    std::string token;

    if(curl) {
        std::string authUrl = "https://api.mail.tm/token";
        std::string authData = "{\"address\":\"" + email + "\", \"password\":\"" + password + "\"}";
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, authUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, authData.c_str());

        // Set up headers for JSON
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Write response to a string
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            Json::Value jsonData;
            Json::CharReaderBuilder readerBuilder;
            std::string errs;

            std::istringstream iss(response);
            if (Json::parseFromStream(readerBuilder, iss, &jsonData, &errs)) {
                token = jsonData["token"].asString();
                std::cout << "Token:" << token << "\n" << std::endl;
                std::cout << "Mail Adress:" << email << "\n" << std::endl;
            } else {
                std::cerr << "Failed to parse token from response" << std::endl;
            }
        } else {
            std::cerr << "CURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return token;
}

// Function to check inbox using the token
void checkInbox(const std::string& token) {
    CURL* curl = curl_easy_init();

    if(curl) {
        std::string inboxUrl = "https://api.mail.tm/messages";
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, inboxUrl.c_str());

        // Set up headers for authorization
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Write response to a string
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            Json::Value messagesData;
            Json::CharReaderBuilder readerBuilder;
            std::string errs;

            std::istringstream iss(response);
            if (Json::parseFromStream(readerBuilder, iss, &messagesData, &errs)) {
                for (const auto& message : messagesData["hydra:member"]) {
                    std::cout << "From: " << message["from"]["address"].asString() << std::endl;
                    std::cout << "Subject: " << message["subject"].asString() << std::endl;
                    std::cout << "Preview: " << message["intro"].asString() << std::endl;
                    std::cout << "-------------------" << std::endl;
                }
            } else {
                std::cerr << "Failed to parse messages from response" << std::endl;
            }
        } else {
            std::cerr << "CURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main() {
    std::string email = "cchguqmz@livinitlarge.net";  // Your registered email
    std::string password = "temporary_password";      // Your registration password

    std::string token = authenticate(email, password);
    if (!token.empty()) {
        std::cout << "Checking inbox for new messages..." << std::endl;
        checkInbox(token);
    } else {
        std::cerr << "Authentication failed." << std::endl;
    }

    return 0;
}

/*g++ -o check_inbox_with_curl check_inbox_with_curl.cpp -lcurl -I/opt/homebrew/Cellar/jsoncpp/1.9.6/include -L/opt/homebrew/Cellar/jsoncpp/1.9.6/lib -ljsoncpp -std=c++11
./check_inbox_with_curl*/
