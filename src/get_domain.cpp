#include "MailTM.h"
#include <iostream>
#include "CurlWrapper.h"

using namespace MailTMAPI;

// Main function to demonstrate fetching an available domain
// Note: It's possible to create an email without explicitly fetching a domain,
// so this function might be redundant.

int main() {
    MailTM mailTm; // Create an instance of the MailTM class

    // Attempt to fetch an available domain
    std::string domain = mailTm.getAvailableDomain();
    if (!domain.empty()) { // Check if a domain was successfully fetched
        std::cout << "Available domain: " << domain << std::endl; // Output the domain
    } else { // Handle failure to fetch a domain
        std::cerr << "Failed to fetch a domain." << std::endl;
    }

    return 0; // Exit the program successfully
}
