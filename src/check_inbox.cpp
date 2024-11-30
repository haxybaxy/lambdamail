#include "MailTM.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <chrono>
using namespace std::chrono_literals; // Enable chrono literals like 10s

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

    auto tokenOpt = mailTm.authenticate(email, password);
    if (!tokenOpt) {
        std::cerr << "Authentication failed." << std::endl;
        return 1;
    }
    std::string token = *tokenOpt;

    auto accountIdOpt = mailTm.getAccountId(token);
    if (!accountIdOpt) {
        std::cerr << "Failed to get account ID." << std::endl;
        return 1;
    }
    std::string accountId = *accountIdOpt;

    std::cout << "Checking inbox (Type 'x' to stop, 'delete' to delete account)..." << std::endl;

    std::atomic<bool> running(true);
    std::atomic<bool> deleteAccount(false);
    std::unordered_set<std::string> printedMessages;

    std::thread inputThread(monitorInput, std::ref(running), std::ref(deleteAccount));

    while (running) {
        auto messages = mailTm.checkInbox(token);
        for (const auto& message : messages) {
            std::string messageId = message["id"].asString();
            if (printedMessages.find(messageId) == printedMessages.end()) {
                Json::Value fullMessage = mailTm.getMessage(token, messageId);

                std::cout << "From: " << message["from"]["address"].asString() << std::endl;
                std::cout << "Subject: " << message["subject"].asString() << std::endl;

                if (message.isMember("intro")) {
                    std::cout << "Preview: " << message["intro"].asString() << std::endl;
                }

                if (fullMessage.isMember("text")) {
                    std::cout << "Body: " << fullMessage["text"].asString() << std::endl;
                } else if (fullMessage.isMember("html")) {
                    std::cout << "Body (HTML): " << fullMessage["html"].asString() << std::endl;
                }

                std::cout << "-------------------" << std::endl;
                printedMessages.insert(messageId);
            }
        }
        std::this_thread::sleep_for(10s);
    }

    if (inputThread.joinable()) {
        inputThread.join();
    }

    if (deleteAccount) {
        auto deleteResult = mailTm.deleteAccount(token, accountId);
        if (deleteResult) {
            std::cout << "Account deleted successfully." << std::endl;
        } else {
            std::cerr << "Failed to delete account." << std::endl;
        }
    }

    std::cout << "Exiting inbox check..." << std::endl;
    return 0;
}
