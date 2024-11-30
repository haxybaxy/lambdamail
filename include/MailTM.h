#pragma once
#include <string>
#include <vector>
#include <optional>
#include <json/json.h>

class MailTM {
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp); // Static callback function
    std::string sendRequest(const std::string& url, const std::string& method,
                            const std::string& payload = "", const std::string& authToken = "");
    const std::string baseUrl = "https://api.mail.tm";

public:
    MailTM() = default;
    ~MailTM() = default;

    std::string getAvailableDomain();
    std::optional<std::string> registerEmail(const std::string& username, const std::string& password);
    std::optional<std::string> authenticate(const std::string& email, const std::string& password);
    std::vector<Json::Value> checkInbox(const std::string& token);
    std::optional<std::string> getAccountId(const std::string& token);
    std::optional<std::string> deleteAccount(const std::string& token, const std::string& accountId);
    Json::Value getMessage(const std::string& token, const std::string& messageId);
};
