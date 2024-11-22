#ifndef MAIL_TM_H
#define MAIL_TM_H

#include <string>
#include <vector>
#include <json/json.h>

class MailTM {
public:
    MailTM();
    ~MailTM();

    std::string getAvailableDomain();
    std::pair<bool, std::string> registerEmail(const std::string& username, const std::string& password);
    std::pair<bool, std::string> authenticate(const std::string& email, const std::string& password);
    std::vector<Json::Value> checkInbox(const std::string& token);
    std::pair<bool, std::string> deleteAccount(const std::string& token);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string sendRequest(const std::string& url, const std::string& method = "GET",
                            const std::string& payload = "", const std::string& authToken = "");
};

#endif // MAIL_TM_H
