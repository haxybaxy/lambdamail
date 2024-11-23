#include "MailTM.h"
#include <iostream>
#include <string>


//TODO: Improve the error handling here, make it print if the user name is already taken


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

        // Combine username and domain
        std::string fullEmail = username + "@" + domain;
        std::cout << "Attempting to register email: " << fullEmail << std::endl;

        // Attempt to register the email
        auto [success, message] = mailTm.registerEmail(fullEmail, password);
        if (success) {
            std::cout << "Registered email: " << fullEmail << std::endl;
            break; // Exit the loop if the email is successfully created
        } else {
            std::cerr << "Failed to register email: " << message << std::endl;
            std::cout << "Please try again.\n";
        }
    }

    return 0;
}
