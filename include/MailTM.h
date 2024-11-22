#pragma once
#include <string>
#include <vector>
#include <json/json.h>

class MailTM {
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string sendRequest(const std::string& url, const std::string& method,
                          const std::string& payload = "", const std::string& authToken = "");
    const std::string baseUrl = "https://api.mail.tm";

public:
    MailTM();
    ~MailTM();

    std::string getAvailableDomain();
    std::pair<bool, std::string> registerEmail(const std::string& username, const std::string& password);
    std::pair<bool, std::string> authenticate(const std::string& email, const std::string& password);
    std::vector<Json::Value> checkInbox(const std::string& token);
    std::pair<bool, std::string> getAccountId(const std::string& token);
    std::pair<bool, std::string> deleteAccount(const std::string& token, const std::string& accountId);
    Json::Value getMessage(const std::string& token, const std::string& messageId);
};
