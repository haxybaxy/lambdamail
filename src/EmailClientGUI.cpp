#include "EmailClientGUI.h"
#include <thread>
#include <random>
#include <iostream>
#include <unordered_map>

EmailClientGUI::EmailClientGUI()
    : window(sf::VideoMode(800, 600), "Temporary Email Client")
    , isEmailGenerated(false)
    , scrollOffset(0)
    , isGenerating(false) {

    const std::vector<std::string> fontPaths = {
        "resources/fonts/Arial.ttf",
        "../resources/fonts/Arial.ttf",
        "./resources/fonts/Arial.ttf"
    };

    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        throw std::runtime_error("Could not load font from any path");
    }

    inputPrompt.setFont(font);
    inputPrompt.setCharacterSize(20);
    inputPrompt.setFillColor(sf::Color::White);
}

void EmailClientGUI::generateEmail() {
    std::cout << "Starting email generation..." << std::endl;  // Debug output

    // Get available domain
    std::string domain = mailTm.getAvailableDomain();
    if (domain.empty()) {
        std::cerr << "Failed to get domain" << std::endl;
        return;
    }
    std::cout << "Got domain: " << domain << std::endl;  // Debug output

    // Generate random username
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    std::string username = "user" + std::to_string(dis(gen));
    password = "pass" + std::to_string(dis(gen));
    email = username + "@" + domain;

    std::cout << "Attempting to register: " << email << std::endl;  // Debug output

    // Register email
    auto result = mailTm.registerEmail(email, password);
    if (result) {
        accountId = *result;
        isEmailGenerated = true;
        std::cout << "Email registered successfully!" << std::endl;  // Debug output

        // Start checking inbox in a separate thread
        std::thread([this]() {
            std::cout << "Starting inbox check thread..." << std::endl;  // Debug output
            checkInboxThread();
        }).detach();
    } else {
        std::cerr << "Failed to register email" << std::endl;
    }
}

void EmailClientGUI::checkInboxThread() {
    std::cout << "Authenticating with email: " << email << std::endl;

    auto tokenOpt = mailTm.authenticate(email, password);
    if (!tokenOpt) {
        std::cerr << "Authentication failed" << std::endl;
        return;
    }

    std::string token = *tokenOpt;
    std::cout << "Authentication successful" << std::endl;

    std::unordered_map<std::string, bool> seenMessageIds;

    while (isEmailGenerated) {
        std::cout << "Checking for new messages..." << std::endl;
        auto newMessages = mailTm.checkInbox(token);

        {
            std::lock_guard<std::mutex> lock(messagesMutex);
            // Only add messages we haven't seen before
            for (const auto& message : newMessages) {
                std::string messageId = message["id"].asString();
                if (seenMessageIds.find(messageId) == seenMessageIds.end()) {
                    messages.push_back(message);
                    seenMessageIds[messageId] = true;
                }
            }
        }

        std::cout << "Total messages: " << messages.size() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void EmailClientGUI::drawMainInterface() {
    // Draw header
    sf::RectangleShape header(sf::Vector2f(800, 100));
    header.setFillColor(sf::Color(70, 70, 70));
    window.draw(header);

    if (!isEmailGenerated) {
        // Generate button
        sf::RectangleShape generateButton(sf::Vector2f(200, 50));
        generateButton.setPosition(300, 25);
        generateButton.setFillColor(sf::Color(100, 149, 237));

        sf::Text buttonText("Generate Email", font, 20);
        buttonText.setPosition(330, 40);
        buttonText.setFillColor(sf::Color::White);

        window.draw(generateButton);
        window.draw(buttonText);
    } else {
        // Show email address
        sf::Text emailText(email, font, 20);
        emailText.setPosition(20, 20);
        emailText.setFillColor(sf::Color::White);
        window.draw(emailText);

        // Delete account button
        sf::RectangleShape deleteButton(sf::Vector2f(150, 30));
        deleteButton.setPosition(620, 20);
        deleteButton.setFillColor(sf::Color(200, 70, 70));

        sf::Text deleteText("Delete Account", font, 16);
        deleteText.setPosition(635, 25);
        deleteText.setFillColor(sf::Color::White);

        window.draw(deleteButton);
        window.draw(deleteText);
    }
}

void EmailClientGUI::drawMessages() {
    std::lock_guard<std::mutex> lock(messagesMutex);
    float yPos = 110 - scrollOffset;

    for (const auto& message : messages) {
        if (yPos >= 110 && yPos <= 540) {
            sf::RectangleShape messageBox(sf::Vector2f(780, 80));
            messageBox.setPosition(10, yPos);
            messageBox.setFillColor(sf::Color(60, 60, 60));
            window.draw(messageBox);

            sf::Text fromText("From: " + message["from"]["address"].asString(), font, 16);
            fromText.setPosition(20, yPos + 10);
            fromText.setFillColor(sf::Color::White);
            window.draw(fromText);

            sf::Text subjectText("Subject: " + message["subject"].asString(), font, 16);
            subjectText.setPosition(20, yPos + 35);
            subjectText.setFillColor(sf::Color::White);
            window.draw(subjectText);
        }
        yPos += 90;
    }
}

void EmailClientGUI::handleMouseClick(int x, int y) {
    std::cout << "Click detected at x: " << x << ", y: " << y << std::endl;  // Debug output

    if (!isEmailGenerated) {
        // Generate button coordinates
        if (x >= 300 && x <= 500 && y >= 25 && y <= 75) {
            std::cout << "Generating email..." << std::endl;  // Debug output
            generateEmail();
        }
    } else if (isEmailGenerated) {
        // Delete button coordinates
        if (x >= 620 && x <= 770 && y >= 20 && y <= 50) {
            std::cout << "Deleting account..." << std::endl;  // Debug output
            deleteAccount();
        }
    }
}

void EmailClientGUI::handleScroll(float delta) {
    scrollOffset += delta * 30;
    if (scrollOffset < 0) scrollOffset = 0;

    float maxScroll = messages.size() * 90 - 430;
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
}

void EmailClientGUI::deleteAccount() {
    if (isEmailGenerated) {
        auto tokenOpt = mailTm.authenticate(email, password);
        if (tokenOpt) {
            mailTm.deleteAccount(*tokenOpt, accountId);
        }
        isEmailGenerated = false;
        messages.clear();
        email.clear();
        password.clear();
        accountId.clear();
    }
}

void EmailClientGUI::run() {
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        std::cout << "Mouse clicked!" << std::endl;  // Debug output
                        handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                    }
                    break;

                case sf::Event::MouseWheelScrolled:
                    handleScroll(event.mouseWheelScroll.delta);
                    break;

                default:
                    break;
            }
        }

        window.clear(sf::Color(50, 50, 50));
        drawMainInterface();
        if (isEmailGenerated) {
            drawMessages();
        }
        window.display();
    }
}
