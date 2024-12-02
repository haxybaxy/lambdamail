#include "MailTM.h"
#include <iostream>
#include <string>
#include "CurlWrapper.h"

using namespace MailTMAPI;

int main() {
    MailTM mailTm;
    std::string domain = mailTm.getAvailableDomain();

    if (domain.empty()) {
        std::cerr << "Failed to fetch available domain. Cannot proceed with registration." << std::endl;
        return 1;
    }

    std::cout << "Fetched domain: " << domain << std::endl;

    std::string username, password;
    while (true) {
        std::cout << "Enter your desired username: ";
        std::cin >> username;
        std::cout << "Enter your desired password: ";
        std::cin >> password;

        std::string fullEmail = username + "@" + domain;
        std::cout << "Attempting to register email: " << fullEmail << std::endl;

        auto result = mailTm.registerEmail(fullEmail, password);
        if (result) {
            std::cout << "Registered email: " << fullEmail << std::endl;
            break;
        } else {
            std::cerr << "Failed to register email. Please try again." << std::endl;
        }
    }

    return 0;
}
