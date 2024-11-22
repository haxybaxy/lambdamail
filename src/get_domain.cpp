#include "MailTM.h"
#include <iostream>


int main() {
    MailTM mailTm;
    std::string domain = mailTm.getAvailableDomain();
    if (!domain.empty()) {
        std::cout << "Available domain: " << domain << std::endl;
    } else {
        std::cerr << "Failed to fetch a domain." << std::endl;
    }
    return 0;
}