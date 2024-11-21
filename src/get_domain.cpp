#include "MailTM.h"
#include <iostream>

// You can create an email without getting a domain, maybe we can remove this function

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
