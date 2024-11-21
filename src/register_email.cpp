#include "MailTM.h"
#include <iostream>


//TODO: Improve the error handling here, make it print if the user name is already taken


int main() {
    MailTM mailTm;
    std::string username, password;

    std::cout << "Enter your desired username: ";
    std::cin >> username;
    std::cout << "Enter your desired password: ";
    std::cin >> password;

    std::string email = mailTm.registerEmail(username, password);
    if (!email.empty()) {
        std::cout << "Registered email: " << email << std::endl;
    } else {
        std::cerr << "Failed to register email." << std::endl;
    }
    return 0;
}
