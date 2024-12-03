#include "MailTM.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <chrono>
#include "CurlWrapper.h"

using namespace std::chrono_literals; // Enable chrono literals like 10s
using namespace MailTMAPI;   

// Function to monitor user input and handle commands to stop or delete the account
void monitorInput(std::atomic<bool>& running, std::atomic<bool>& deleteAccount) {
    std::string input;
    while (running) { // Continuously check for user input while the program is running
        std::cin >> input; // Read user input
        if (input == "x" || input == "X") { // Check if the input is to exit
            running = false; // Set the running flag to false to stop the program
            break;
        } else if (input == "delete") { // Check if the input is to delete the account
            deleteAccount = true; // Set the deleteAccount flag to true
            running = false; // Stop the program after setting deleteAccount
            break;
        }
    }
}

int main() {
    MailTM mailTm; // Create an instance of the MailTM class
    std::string email, password; // Variables to store user credentials

    std::cout << "Enter your email: "; // Prompt the user for email
    std::cin >> email; // Read email input
    std::cout << "Enter your password: "; // Prompt the user for password
    std::cin >> password; // Read password input

    // Authenticate the user with the provided credentials
    auto tokenOpt = mailTm.authenticate(email, password);
    if (!tokenOpt) { // Check if authentication failed
        std::cerr << "Authentication failed." << std::endl;
        return 1; // Exit with an error code
    }
    std::string token = *tokenOpt; // Retrieve the authentication token

    // Get the account ID using the authentication token
    auto accountIdOpt = mailTm.getAccountId(token);
    if (!accountIdOpt) { // Check if retrieving account ID failed
        std::cerr << "Failed to get account ID." << std::endl;
        return 1; // Exit with an error code
    }
    std::string accountId = *accountIdOpt; // Retrieve the account ID

    std::cout << "Checking inbox (Type 'x' to stop, 'delete' to delete account)..." << std::endl;

    std::atomic<bool> running(true); // Flag to indicate if the program is running
    std::atomic<bool> deleteAccount(false); // Flag to indicate if the account should be deleted
    std::unordered_set<std::string> printedMessages; // Set to track printed messages

    // Start a thread to monitor user input
    std::thread inputThread(monitorInput, std::ref(running), std::ref(deleteAccount));

    // Main loop to check the inbox
    while (running) {
        auto messages = mailTm.checkInbox(token); // Fetch messages from the inbox
        for (const auto& message : messages) { // Iterate over the messages
            std::string messageId = message["id"].asString(); // Get the message ID
            if (printedMessages.find(messageId) == printedMessages.end()) { // Check if the message is new
                Json::Value fullMessage = mailTm.getMessage(token, messageId); // Fetch the full message

                // Print message details
                std::cout << "From: " << message["from"]["address"].asString() << std::endl;
                std::cout << "Subject: " << message["subject"].asString() << std::endl;

                // Print message preview if available
                if (message.isMember("intro")) {
                    std::cout << "Preview: " << message["intro"].asString() << std::endl;
                }

                // Print the full message body or HTML content
                if (fullMessage.isMember("text")) {
                    std::cout << "Body: " << fullMessage["text"].asString() << std::endl;
                } else if (fullMessage.isMember("html")) {
                    std::cout << "Body (HTML): " << fullMessage["html"].asString() << std::endl;
                }

                std::cout << "-------------------" << std::endl;
                printedMessages.insert(messageId); // Mark the message as printed
            }
        }
        std::this_thread::sleep_for(10s); // Wait for 10 seconds before checking the inbox again
    }

    // Join the input thread before exiting
    if (inputThread.joinable()) {
        inputThread.join();
    }

    // Handle account deletion if requested by the user
    if (deleteAccount) {
        auto deleteResult = mailTm.deleteAccount(token, accountId); // Attempt to delete the account
        if (deleteResult) { // Check if deletion was successful
            std::cout << "Account deleted successfully." << std::endl;
        } else { // Handle failure to delete the account
            std::cerr << "Failed to delete account." << std::endl;
        }
    }

    std::cout << "Exiting inbox check..." << std::endl; // Indicate program termination
    return 0; // Exit the program successfully
}
