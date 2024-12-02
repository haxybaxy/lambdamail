#include "EmailClientGUI.h"
#include <thread>
#include <random>
#include <iostream>
#include <unordered_map>

EmailClientGUI::EmailClientGUI()
    : window(sf::VideoMode(800, 600), "Temporary Email Client")
    , isEmailGenerated(false)
    , scrollOffset(0)
    , isGenerating(false)
    , isCustomUsername(false)
    , isInputActive(false) {

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

bool EmailClientGUI::isValidUsername(const std::string& username) {
    if (username.empty() || username.length() < 3) return false;

    // Check if username contains only allowed characters
    for (char c : username) {
        if (!std::isalnum(c) && c != '.' && c != '_' && c != '-') {
            return false;
        }
    }

    // Username should start with a letter
    if (!std::isalpha(username[0])) return false;

    return true;
}

void EmailClientGUI::generateEmail() {
    std::cout << "Starting email generation..." << std::endl;

    std::string domain = mailTm.getAvailableDomain();
    if (domain.empty()) {
        std::cerr << "Failed to get domain" << std::endl;
        return;
    }
    std::cout << "Got domain: " << domain << std::endl;

    std::string username;
    if (isCustomUsername) {
        // Validate custom username
        if (!isValidUsername(customUsername)) {
            std::cerr << "Invalid username. Username must:\n"
                     << "- Start with a letter\n"
                     << "- Be at least 3 characters long\n"
                     << "- Contain only letters, numbers, dots, underscores, or hyphens"
                     << std::endl;
            return;
        }
        username = customUsername;
    } else {
        // Generate random username (ensure it starts with a letter)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        username = "user" + std::to_string(dis(gen));
    }

    password = "pass" + std::to_string(std::random_device{}());
    email = username + "@" + domain;

    std::cout << "Attempting to register: " << email << std::endl;

    auto result = mailTm.registerEmail(email, password);
    if (result) {
        accountId = *result;
        isEmailGenerated = true;
        std::cout << "Email registered successfully!" << std::endl;

        std::thread([this]() {
            std::cout << "Starting inbox check thread..." << std::endl;
            checkInboxThread();
        }).detach();
    } else {
        std::cerr << "Failed to register email. Please try a different username." << std::endl;
        if (isCustomUsername) {
            customUsername.clear(); // Clear the invalid username
        }
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
            for (const auto& message : newMessages) {
                std::string messageId = message["id"].asString();
                if (seenMessageIds.find(messageId) == seenMessageIds.end()) {
                    // Get full message content
                    Json::Value fullMessage = mailTm.getMessage(token, messageId);
                    messages.push_back(fullMessage);  // Store full message instead
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
        // Toggle button for custom/random username
        sf::RectangleShape toggleButton(sf::Vector2f(200, 30));
        toggleButton.setPosition(300, 15);
        toggleButton.setFillColor(sf::Color(100, 149, 237));

        sf::Text toggleText(isCustomUsername ? "Use Random Username" : "Use Custom Username", font, 16);
        toggleText.setPosition(320, 20);
        toggleText.setFillColor(sf::Color::White);

        window.draw(toggleButton);
        window.draw(toggleText);

        if (isCustomUsername) {
            // Username input box
            sf::RectangleShape inputBox(sf::Vector2f(200, 30));
            inputBox.setPosition(300, 55);
            inputBox.setFillColor(isInputActive ? sf::Color::White : sf::Color(200, 200, 200));
            window.draw(inputBox);

            // Show username input
            sf::Text usernameText(customUsername + (isInputActive ? "_" : ""), font, 16);
            usernameText.setPosition(310, 60);
            usernameText.setFillColor(sf::Color::Black);
            window.draw(usernameText);

            // Helper text with more detailed requirements
            std::vector<std::string> helperLines = {
                "Username requirements:",
                "- Start with a letter",
                "- At least 3 characters",
                "- Letters, numbers, dots, _, -"
            };

            float yPos = 90;
            for (const auto& line : helperLines) {
                sf::Text helperText(line, font, 12);
                helperText.setPosition(300, yPos);
                helperText.setFillColor(sf::Color(200, 200, 200));
                window.draw(helperText);
                yPos += 15;
            }
        }

        // Generate button
        sf::RectangleShape generateButton(sf::Vector2f(200, 30));
        generateButton.setPosition(520, 55);
        generateButton.setFillColor(sf::Color(100, 149, 237));

        sf::Text buttonText("Generate Email", font, 16);
        buttonText.setPosition(570, 60);
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
    activeLinks.clear(); // Clear previous links

    for (const auto& message : messages) {
        if (yPos >= 110 && yPos <= 540) {
            // Message container
            sf::RectangleShape messageBox(sf::Vector2f(780, 120));
            messageBox.setPosition(10, yPos);
            messageBox.setFillColor(sf::Color(60, 60, 60));
            window.draw(messageBox);

            // From and Subject remain the same
            sf::Text fromText("From: " + message["from"]["address"].asString(), font, 16);
            fromText.setPosition(20, yPos + 10);
            fromText.setFillColor(sf::Color::White);
            window.draw(fromText);

            sf::Text subjectText("Subject: " + message["subject"].asString(), font, 16);
            subjectText.setPosition(20, yPos + 35);
            subjectText.setFillColor(sf::Color::White);
            window.draw(subjectText);

            // Body text processing
            std::string bodyText;
            if (message.isMember("text")) {
                bodyText = message["text"].asString();
            } else if (message.isMember("html")) {
                bodyText = stripHtmlExceptLinks(message["html"].asString());
            } else if (message.isMember("intro")) {
                bodyText = message["intro"].asString();
            }

            // Draw body text with clickable links
            float textX = 20;
            float textY = yPos + 60;
            float maxWidth = 740;

            std::vector<std::string> words = splitIntoWords(bodyText);
            std::string currentLine;
            float lineX = textX;

            for (const auto& word : words) {
                bool isLink = word.find("http://") == 0 || word.find("https://") == 0;

                sf::Text testText(currentLine + " " + word, font, 14);
                if (testText.getLocalBounds().width > maxWidth && !currentLine.empty()) {
                    // Draw current line
                    sf::Text lineText(currentLine, font, 14);
                    lineText.setPosition(lineX, textY);
                    lineText.setFillColor(sf::Color(200, 200, 200));
                    window.draw(lineText);

                    textY += 20; // Move to next line
                    currentLine = word;
                    lineX = textX;
                } else {
                    if (!currentLine.empty()) currentLine += " ";
                    currentLine += word;
                }

                if (isLink) {
                    sf::Text linkText(word, font, 14);
                    linkText.setPosition(lineX, textY);
                    linkText.setFillColor(sf::Color::Cyan);
                    linkText.setStyle(sf::Text::Underlined);

                    // Store clickable area
                    ClickableLink link;
                    link.bounds = linkText.getGlobalBounds();
                    link.url = word;
                    activeLinks.push_back(link);

                    window.draw(linkText);

                    lineX += linkText.getGlobalBounds().width + 5;
                } else {
                    sf::Text wordText(word + " ", font, 14);
                    wordText.setPosition(lineX, textY);
                    wordText.setFillColor(sf::Color(200, 200, 200));
                    window.draw(wordText);

                    lineX += wordText.getGlobalBounds().width;
                }
            }
        }
        yPos += 130;
    }
}

std::string EmailClientGUI::stripHtmlExceptLinks(const std::string& html) {
    std::string result;
    bool inTag = false;
    bool inLink = false;
    std::string currentLink;

    for (size_t i = 0; i < html.length(); ++i) {
        if (html[i] == '<') {
            inTag = true;
            // Check if it's a link
            if (html.substr(i, 9) == "<a href=\"") {
                inLink = true;
                i += 8; // Skip to the href value
                continue;
            }
        } else if (html[i] == '>') {
            inTag = false;
            if (inLink) {
                result += currentLink + " ";
                currentLink.clear();
                inLink = false;
            }
            continue;
        }

        if (!inTag) {
            if (html[i] == '&' && html.substr(i, 6) == "&nbsp;") {
                result += " ";
                i += 5;
            } else {
                result += html[i];
            }
        } else if (inLink && html[i] != '"') {
            currentLink += html[i];
        }
    }
    return result;
}

std::vector<std::string> EmailClientGUI::splitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

void EmailClientGUI::handleMouseClick(int x, int y) {
    // Check for link clicks
    for (const auto& link : activeLinks) {
        if (link.bounds.contains(x, y)) {
            openUrl(link.url);
            return;
        }
    }

    std::cout << "Click detected at x: " << x << ", y: " << y << std::endl;

    if (!isEmailGenerated) {
        // Toggle button
        if (x >= 300 && x <= 500 && y >= 15 && y <= 45) {
            isCustomUsername = !isCustomUsername;
            customUsername.clear();
            isInputActive = false;
        }
        // Input box
        else if (isCustomUsername && x >= 300 && x <= 500 && y >= 55 && y <= 85) {
            isInputActive = true;
        }
        // Generate button
        else if (x >= 520 && x <= 720 && y >= 55 && y <= 85) {
            if (!isCustomUsername || (isCustomUsername && !customUsername.empty())) {
                generateEmail();
            }
        } else {
            isInputActive = false;
        }
    } else if (x >= 620 && x <= 770 && y >= 20 && y <= 50) {
        deleteAccount();
    }
}

void EmailClientGUI::handleScroll(float delta) {
    scrollOffset += delta * 30;
    if (scrollOffset < 0) scrollOffset = 0;

    // Adjust max scroll to account for larger message boxes
    float maxScroll = messages.size() * 130 - 430;  // Changed from 90 to 130
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
                        handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                    }
                    break;

                case sf::Event::TextEntered:
                    if (isInputActive && !isEmailGenerated) {
                        if (event.text.unicode == '\b') {
                            if (!customUsername.empty()) {
                                customUsername.pop_back();
                            }
                        }
                        else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\n') {
                            customUsername += static_cast<char>(event.text.unicode);
                        }
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

void EmailClientGUI::openUrl(const std::string& url) {
    #ifdef _WIN32
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    #elif __APPLE__
        system(("open " + url).c_str());
    #else
        system(("xdg-open " + url).c_str());
    #endif
}
