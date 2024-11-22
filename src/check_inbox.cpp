#include "MailTM.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <set>
#include <chrono>

void monitorInput(std::atomic<bool>& running, std::atomic<bool>& deleteAccount) {
    std::string input;
    while (running) {
        std::cin >> input;
        if (input == "x" || input == "X") {
            running = false;
            break;
        } else if (input == "delete") {
            deleteAccount = true;
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

    auto [authSuccess, token] = mailTm.authenticate(email, password);
    if (!authSuccess) {
        std::cerr << "Authentication failed: " << token << std::endl;
        return 1;
    }

    std::cout << "Checking inbox (Type 'x' to stop, 'delete' to delete account)..." << std::endl;

    std::atomic<bool> running(true);
    std::atomic<bool> deleteAccount(false);
    std::set<std::string> printedMessages;

    std::thread inputThread(monitorInput, std::ref(running), std::ref(deleteAccount));

    while (running) {
        auto messages = mailTm.checkInbox(token);
        for (const auto& message : messages) {
            std::string messageId = message["id"].asString();
            if (printedMessages.find(messageId) == printedMessages.end()) {
                std::cout << "From: " << message["from"]["address"].asString() << std::endl;
                std::cout << "Subject: " << message["subject"].asString() << std::endl;
                std::cout << "-------------------" << std::endl;
                printedMessages.insert(messageId);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    if (inputThread.joinable()) {
        inputThread.join();
    }

    if (deleteAccount) {
        auto [deleteSuccess, message] = mailTm.deleteAccount(token);
        if (deleteSuccess) {
            std::cout << "Account deleted successfully: " << message << std::endl;
        } else {
            std::cerr << "Failed to delete email account." << std::endl;
        }
    }

    std::cout << "Exiting inbox check..." << std::endl;
    return 0;
}
