#include "MailTM.h"
#include <iostream>
#include <string>
#include "CurlWrapper.h"

using namespace MailTMAPI;

// Main function to register an email address using MailTM
int main() {
    MailTM mailTm; // Create an instance of the MailTM class

    // Fetch an available domain
    std::string domain = mailTm.getAvailableDomain();
    if (domain.empty()) { // Check if domain fetching failed
        std::cerr << "Failed to fetch available domain. Cannot proceed with registration." << std::endl;
        return 1; // Exit with an error code
    }

    std::cout << "Fetched domain: " << domain << std::endl; // Output the fetched domain

    std::string username, password; // Variables to hold user-provided username and password

    while (true) { // Loop to allow the user to retry registration
        std::cout << "Enter your desired username: "; // Prompt for username
        std::cin >> username; // Read username input
        std::cout << "Enter your desired password: "; // Prompt for password
        std::cin >> password; // Read password input

        std::string fullEmail = username + "@" + domain; // Construct the full email address
        std::cout << "Attempting to register email: " << fullEmail << std::endl;

        // Attempt to register the email with the provided credentials
        auto result = mailTm.registerEmail(fullEmail, password);
        if (result) { // Check if registration was successful
            std::cout << "Registered email: " << fullEmail << std::endl; // Output success message
            break; // Exit the loop
        } else { // Handle registration failure
            std::cerr << "Failed to register email. Please try again." << std::endl;
        }
    }

    return 0; // Exit the program successfully
}
