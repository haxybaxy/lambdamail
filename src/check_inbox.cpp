#include "MailTM.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <set>
#include <chrono>

//TODO: make it delete the email when you press x (or give you the option)


// Function to monitor user input
void monitorInput(std::atomic<bool>& running) {
    std::string input;
    while (running) {
        std::cin >> input;
        if (input == "x" || input == "X") { // User types 'x' or 'X' to terminate
            running = false;
            break;
        }
    }
}

int main() {
    MailTM mailTm;
    std::string email, password;

    std::cout << "Enter your email: ";
    std::cin >> email;
    std::cout << "Enter your password: ";
    std::cin >> password;

    // Authenticate and get the token
    std::string token = mailTm.authenticate(email, password);
    if (token.empty()) {
        std::cerr << "Authentication failed." << std::endl;
        return 1;
    }

    std::cout << "Checking inbox (Type 'x' to stop)..." << std::endl;

    std::set<std::string> printedMessages; // Set to store IDs of already printed messages
    std::atomic<bool> running(true);       // Atomic flag to control the loop

    // Start the input monitoring thread
    std::thread inputThread(monitorInput, std::ref(running));

    // Main loop to check inbox
    while (running) {
        // Get messages from inbox
        auto messages = mailTm.checkInbox(token);

        // Iterate through messages and print only new ones
        for (const auto& message : messages) {
            std::string messageId = message["id"].asString();
            if (printedMessages.find(messageId) == printedMessages.end()) {
                // Print the email
                std::cout << "From: " << message["from"]["address"].asString() << std::endl;
                std::cout << "Subject: " << message["subject"].asString() << std::endl;
                std::cout << "Preview: " << message["intro"].asString() << std::endl;
                std::cout << "-------------------" << std::endl;

                // Add the message ID to the set
                printedMessages.insert(messageId);
            }
        }

        // Wait for 10 seconds before checking again
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    // Clean up the input thread
    if (inputThread.joinable()) {
        inputThread.join();
    }

    std::cout << "Exiting inbox check..." << std::endl;
    return 0;
}
