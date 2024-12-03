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
    , isInputActive(false)
    , isPopupOpen(false)
    , selectedMessageIndex(-1)
    , popupScrollOffset(0) {

    // Try multiple font paths and use a more complete font
    const std::vector<std::string> fontPaths = {
        "resources/fonts/NotoSans-Regular.ttf",  // Noto Sans supports more characters
        "../resources/fonts/NotoSans-Regular.ttf",
        "./resources/fonts/NotoSans-Regular.ttf",
        "resources/fonts/Arial.ttf",
        "../resources/fonts/Arial.ttf",
        "./resources/fonts/Arial.ttf",
        "resources/fonts/Roboto-Regular.ttf",
        "../resources/fonts/Roboto-Regular.ttf",
        "./resources/fonts/Roboto-Regular.ttf"
    };

    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            std::cout << "Loaded font from: " << path << std::endl;
            break;
        }
    }

    if (!fontLoaded) {
        throw std::runtime_error("Failed to load font");
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
                     << "• Start with a letter\n"
                     << "• Be at least 3 characters long\n"
                     << "• Contain only letters, numbers, dots, underscores, or hyphens"
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
    // Draw header with gradient effect
    sf::RectangleShape header(sf::Vector2f(800, 100));
    sf::RectangleShape headerGradient(sf::Vector2f(800, 5));
    header.setFillColor(sf::Color(40, 44, 52));  // Darker, more professional blue
    headerGradient.setFillColor(sf::Color(65, 105, 225));  // Royal Blue
    headerGradient.setPosition(0, 100);
    window.draw(header);
    window.draw(headerGradient);

    if (!isEmailGenerated) {
        // Center container for controls
        sf::RectangleShape controlContainer(sf::Vector2f(600, 180));
        controlContainer.setPosition(100, 120);
        controlContainer.setFillColor(sf::Color(50, 54, 62));
        window.draw(controlContainer);

        // Title
        sf::Text title("Temporary Email Generator", font, 24);
        title.setPosition(270, 130);
        title.setFillColor(sf::Color::White);
        window.draw(title);

        // Toggle button with better styling
        sf::RectangleShape toggleButton(sf::Vector2f(200, 40));
        toggleButton.setPosition(150, 180);
        toggleButton.setFillColor(sf::Color(65, 105, 225));

        sf::Text toggleText(isCustomUsername ? "Use Random Username" : "Use Custom Username", font, 16);
        toggleText.setPosition(170, 190);
        toggleText.setFillColor(sf::Color::White);

        window.draw(toggleButton);
        window.draw(toggleText);

        if (isCustomUsername) {
            // Username input box with better styling
            sf::RectangleShape inputBox(sf::Vector2f(300, 40));
            inputBox.setPosition(150, 230);
            inputBox.setFillColor(isInputActive ? sf::Color::White : sf::Color(200, 200, 200));
            inputBox.setOutlineThickness(2);
            inputBox.setOutlineColor(isInputActive ? sf::Color(65, 105, 225) : sf::Color::Transparent);
            window.draw(inputBox);

            // Show username input
            sf::Text usernameText(customUsername + (isInputActive ? "_" : ""), font, 16);
            usernameText.setPosition(160, 240);
            usernameText.setFillColor(sf::Color::Black);
            window.draw(usernameText);

            // Helper text with better formatting
            std::vector<std::string> helperLines = {
                "• Start with a letter",
                "• At least 3 characters",
                "• Letters, numbers, dots, _, -"
            };

            float yPos = 280;
            for (const auto& line : helperLines) {
                sf::Text helperText(line, font, 12);
                helperText.setPosition(150, yPos);
                helperText.setFillColor(sf::Color(150, 150, 150));
                window.draw(helperText);
                yPos += 15;
            }
        }

        // Generate button with better styling
        sf::RectangleShape generateButton(sf::Vector2f(200, 40));
        generateButton.setPosition(450, 230);
        generateButton.setFillColor(sf::Color(46, 204, 113));  // Green color

        sf::Text buttonText("Generate Email", font, 16);
        float textWidth = buttonText.getLocalBounds().width;
        buttonText.setPosition(450 + (200 - textWidth) / 2, 240);
        buttonText.setFillColor(sf::Color::White);

        window.draw(generateButton);
        window.draw(buttonText);
    } else {
        // Email info display
        sf::Text emailText("Your temporary email: " + email, font, 18);
        emailText.setPosition(20, 20);
        emailText.setFillColor(sf::Color::White);
        window.draw(emailText);

        // Delete button with better styling
        sf::RectangleShape deleteButton(sf::Vector2f(150, 30));
        deleteButton.setPosition(620, 20);
        deleteButton.setFillColor(sf::Color(231, 76, 60));  // Red color

        sf::Text deleteText("Delete Account", font, 14);
        float deleteTextWidth = deleteText.getLocalBounds().width;
        deleteText.setPosition(620 + (150 - deleteTextWidth) / 2, 25);
        deleteText.setFillColor(sf::Color::White);

        window.draw(deleteButton);
        window.draw(deleteText);
    }
}

void EmailClientGUI::drawMessages() {
    std::lock_guard<std::mutex> lock(messagesMutex);
    float yPos = 110 - scrollOffset;
    activeLinks.clear();

    // Message container background
    sf::RectangleShape messagesContainer(sf::Vector2f(780, 480));
    messagesContainer.setPosition(10, 110);
    messagesContainer.setFillColor(sf::Color(45, 49, 57));
    window.draw(messagesContainer);

    for (const auto& message : messages) {
        if (yPos >= 110 && yPos <= 540) {
            // Message box with better styling
            sf::RectangleShape messageBox(sf::Vector2f(760, 120));
            messageBox.setPosition(20, yPos);
            messageBox.setFillColor(sf::Color(50, 54, 62));
            messageBox.setOutlineThickness(1);
            messageBox.setOutlineColor(sf::Color(70, 74, 82));
            window.draw(messageBox);

            // From header
            sf::Text fromText("From: " + message["from"]["address"].asString(), font, 16);
            fromText.setPosition(30, yPos + 10);
            fromText.setFillColor(sf::Color(200, 200, 200));
            window.draw(fromText);

            // Subject with better contrast
            sf::Text subjectText("Subject: " + message["subject"].asString(), font, 16);
            subjectText.setPosition(30, yPos + 35);
            subjectText.setFillColor(sf::Color::White);
            window.draw(subjectText);

            // Body text processing (rest of the code remains the same)
            std::string bodyText;
            if (message.isMember("text")) {
                bodyText = message["text"].asString();
            } else if (message.isMember("html")) {
                bodyText = stripHtmlExceptLinks(message["html"].asString());
            } else if (message.isMember("intro")) {
                bodyText = message["intro"].asString();
            }

            // Draw body text with improved formatting
            float textX = 30;  // Increased margin
            float textY = yPos + 60;
            float maxWidth = 720;  // Adjusted for new margins

            std::vector<std::string> words = splitIntoWords(bodyText);
            std::string currentLine;
            float lineX = textX;

            for (const auto& word : words) {
                bool isLink = word.find("http://") == 0 || word.find("https://") == 0;

                sf::Text testText(currentLine + " " + word, font, 14);
                if (testText.getLocalBounds().width > maxWidth && !currentLine.empty()) {
                    sf::Text lineText(currentLine, font, 14);
                    lineText.setPosition(lineX, textY);
                    lineText.setFillColor(sf::Color(180, 180, 180));
                    window.draw(lineText);

                    textY += 20;
                    currentLine = word;
                    lineX = textX;
                } else {
                    if (!currentLine.empty()) currentLine += " ";
                    currentLine += word;
                }

                if (isLink) {
                    sf::Text linkText(word, font, 14);
                    linkText.setPosition(lineX, textY);
                    linkText.setFillColor(sf::Color(100, 149, 237));  // Softer blue for links
                    linkText.setStyle(sf::Text::Underlined);

                    ClickableLink link;
                    link.bounds = linkText.getGlobalBounds();
                    link.url = word;
                    activeLinks.push_back(link);

                    window.draw(linkText);
                    lineX += linkText.getGlobalBounds().width + 5;
                }
            }
        }
        yPos += 130;
    }

    // Scroll indicator using UTF-8 symbol
    if (messages.size() > 4) {
        sf::Text scrollIndicator(L"▼", font, 20);  // Using wide string for Unicode
        scrollIndicator.setPosition(770, 560);
        scrollIndicator.setFillColor(sf::Color(150, 150, 150));
        window.draw(scrollIndicator);
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
            if (i + 9 < html.length() && html.substr(i, 9) == "<a href=\"") {
                inLink = true;
                i += 8; // Skip to the href value
                continue;
            }
        } else if (html[i] == '>') {
            inTag = false;
            if (inLink) {
                // Add the complete link to result
                if (!currentLink.empty()) {
                    result += currentLink + " ";
                    currentLink.clear();
                }
                inLink = false;
            }
            continue;
        }

        if (!inTag) {
            if (i + 6 <= html.length() && html.substr(i, 6) == "&nbsp;") {
                result += " ";
                i += 5;
            } else if (i + 4 <= html.length() && html.substr(i, 4) == "&lt;") {
                result += "<";
                i += 3;
            } else if (i + 4 <= html.length() && html.substr(i, 4) == "&gt;") {
                result += ">";
                i += 3;
            } else if (i + 5 <= html.length() && html.substr(i, 5) == "&amp;") {
                result += "&";
                i += 4;
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
    std::cout << "Click detected at x: " << x << ", y: " << y << std::endl;

    // Check for link clicks first
    for (const auto& link : activeLinks) {
        if (link.bounds.contains(x, y)) {
            openUrl(link.url);
            return;
        }
    }

    // If popup is open, handle popup-specific clicks
    if (isPopupOpen) {
        // Close button (top-right of popup)
        if (x >= 700 && x <= 730 && y >= 60 && y <= 90) {
            std::cout << "Closing popup" << std::endl;
            isPopupOpen = false;
            selectedMessageIndex = -1;
            return;
        }
        return; // Ignore other clicks when popup is open
    }

    // Check for message clicks when emails are shown
    if (isEmailGenerated) {
        float clickY = y + scrollOffset; // Adjust for scrolling
        if (x >= 20 && x <= 780 && clickY >= 110) {
            int messageIndex = (clickY - 110) / 90; // 90 is the height of each message preview
            std::cout << "Calculated message index: " << messageIndex << std::endl;

            if (messageIndex >= 0 && messageIndex < messages.size()) {
                std::cout << "Opening popup for message " << messageIndex << std::endl;
                isPopupOpen = true;
                selectedMessageIndex = messageIndex;
                popupScrollOffset = 0;
                return;
            }
        }
    }

    if (!isEmailGenerated) {
        // Toggle button (150, 180, 350, 220)
        if (x >= 150 && x <= 350 && y >= 180 && y <= 220) {
            isCustomUsername = !isCustomUsername;
            customUsername.clear();
            isInputActive = false;
        }
        // Input box (150, 230, 450, 270)
        else if (isCustomUsername && x >= 150 && x <= 450 && y >= 230 && y <= 270) {
            isInputActive = true;
        }
        // Generate button (450, 230, 650, 270)
        else if (x >= 450 && x <= 650 && y >= 230 && y <= 270) {
            if (!isCustomUsername || (isCustomUsername && !customUsername.empty())) {
                generateEmail();
            }
        } else {
            isInputActive = false;
        }
    } else {
        // Delete button (620, 20, 770, 50)
        if (x >= 620 && x <= 770 && y >= 20 && y <= 50) {
            deleteAccount();
        }
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

                case sf::Event::MouseWheelScrolled:
                    handleScroll(event.mouseWheelScroll.delta);
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

                default:
                    break;
            }
        }

        window.clear(sf::Color(50, 50, 50));
        drawMainInterface();
        if (isEmailGenerated) {
            drawMessages();
            if (isPopupOpen) {
                drawMessagePopup();
            }
        }
        window.display();
    }
}

void EmailClientGUI::openUrl(const std::string& url) {
    // URL encode only the pipe character, leave ampersands as is
    std::string encodedUrl = url;

    // Handle pipe character only
    size_t pos = 0;
    while ((pos = encodedUrl.find("|", pos)) != std::string::npos) {
        encodedUrl.replace(pos, 1, "%7C");
        pos += 3;
    }

    // Properly quote the entire URL
    #ifdef _WIN32
        ShellExecuteA(NULL, "open", encodedUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
    #elif __APPLE__
        std::string command = "open \"" + encodedUrl + "\"";
        system(command.c_str());
    #else
        std::string command = "xdg-open \"" + encodedUrl + "\"";
        system(command.c_str());
    #endif

    std::cout << "Opening URL: " << encodedUrl << std::endl;
}

void EmailClientGUI::drawMessagePopup() {
    if (!isPopupOpen || selectedMessageIndex < 0 || selectedMessageIndex >= messages.size()) {
        return;
    }

    // Draw the popup UI
    sf::RectangleShape overlay(sf::Vector2f(800, 600));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    sf::RectangleShape popup(sf::Vector2f(600, 400));
    popup.setPosition(100, 100);
    popup.setFillColor(sf::Color(45, 49, 57));
    popup.setOutlineThickness(2);
    popup.setOutlineColor(sf::Color(70, 74, 82));
    window.draw(popup);

    const auto& message = messages[selectedMessageIndex];
    float yPos = 120 - popupScrollOffset;

    // From
    if (message.isMember("from") && message["from"].isObject() && message["from"].isMember("address")) {
        try {
            std::string fromAddress = message["from"]["address"].asString();
            sf::Text fromText("From: " + fromAddress, font, 16);
            fromText.setPosition(120, yPos);
            fromText.setFillColor(sf::Color::White);
            window.draw(fromText);
            yPos += 30;
        } catch (const std::exception& e) {
            std::cout << "Error converting from address: " << e.what() << std::endl;
        }
    }

    // Subject
    if (message.isMember("subject")) {
        try {
            std::string subject = message["subject"].asString();
            sf::Text subjectText("Subject: " + subject, font, 16);
            subjectText.setPosition(120, yPos);
            subjectText.setFillColor(sf::Color::White);
            window.draw(subjectText);
            yPos += 40;
        } catch (const std::exception& e) {
            std::cout << "Error converting subject: " << e.what() << std::endl;
        }
    }

    // Body text
    if (message.isMember("text") && !message["text"].isNull()) {
        try {
            sf::Text bodyHeader("Message:", font, 14);
            bodyHeader.setPosition(120, yPos);
            bodyHeader.setFillColor(sf::Color(150, 150, 150));
            window.draw(bodyHeader);
            yPos += 25;

            std::string bodyText = message["text"].asString();
            drawWrappedText(bodyText, 120, yPos, 540, 14, sf::Color(200, 200, 200));
            yPos += calculateTextHeight(bodyText, 540, 14) + 20;
        } catch (const std::exception& e) {
            std::cout << "Error converting text content: " << e.what() << std::endl;
        }
    }

    // HTML content
    if (message.isMember("html") && !message["html"].isNull()) {
        try {
            // Debug the HTML content type
            std::cout << "HTML content type: " << message["html"].type() << std::endl;

            // Handle array type HTML content
            std::string htmlContent;
            if (message["html"].isArray()) {
                for (const auto& part : message["html"]) {
                    if (part.isString()) {
                        htmlContent += part.asString();
                    }
                }
            } else if (message["html"].isString()) {
                htmlContent = message["html"].asString();
            } else {
                std::cout << "Unsupported HTML content type" << std::endl;
                return;
            }

            if (!htmlContent.empty()) {
                sf::Text htmlHeader("HTML Content:", font, 14);
                htmlHeader.setPosition(120, yPos);
                htmlHeader.setFillColor(sf::Color(150, 150, 150));
                window.draw(htmlHeader);
                yPos += 25;

                std::string strippedHtml = stripHtmlExceptLinks(htmlContent);
                drawWrappedText(strippedHtml, 120, yPos, 540, 14, sf::Color(200, 200, 200));
            }
        } catch (const std::exception& e) {
            std::cout << "Error converting HTML content: " << e.what() << std::endl;
            // Print more details about the HTML content
            if (message.isMember("html")) {
                std::cout << "HTML content exists but couldn't be converted. Type: "
                          << message["html"].type() << std::endl;
            }
        }
    }

    // Close button
    sf::RectangleShape closeButton(sf::Vector2f(30, 30));
    closeButton.setPosition(700, 60);
    closeButton.setFillColor(sf::Color(231, 76, 60));
    window.draw(closeButton);

    sf::Text closeText(L"×", font, 20);
    closeText.setPosition(710, 65);
    closeText.setFillColor(sf::Color::White);
    window.draw(closeText);
}

void EmailClientGUI::drawWrappedText(const std::string& text, float x, float y, float maxWidth, unsigned int fontSize, const sf::Color& color) {
    std::vector<std::string> words = splitIntoWords(text);
    std::string currentLine;
    float lineX = x;
    float currentY = y;
    float maxY = 460;

    for (size_t i = 0; i < words.size(); ++i) {
        if (currentY > maxY) {
            sf::Text ellipsis("...", font, fontSize);
            ellipsis.setPosition(lineX, currentY - fontSize - 6);
            ellipsis.setFillColor(color);
            window.draw(ellipsis);
            break;
        }

        const std::string& word = words[i];
        bool isLink = (word.find("http://") == 0 || word.find("https://") == 0);

        // Simplified URL reconstruction
        std::string fullWord = word;
        if (isLink) {
            while (i + 1 < words.size()) {
                const std::string& next = words[i + 1];
                if (next.empty()) break;

                char firstChar = next[0];
                if (firstChar == '?' || firstChar == '&' || firstChar == '=' ||
                    firstChar == '/' || firstChar == '|' || firstChar == '-' ||
                    firstChar == '_' || firstChar == '.') {
                    fullWord += next;
                    i++;
                } else {
                    bool isUrlPart = true;
                    for (char c : next) {
                        if (!isalnum(c) && c != '=' && c != '&' && c != '%' &&
                            c != '+' && c != '|' && c != '-' && c != '_' && c != '.') {
                            isUrlPart = false;
                            break;
                        }
                    }
                    if (!isUrlPart) break;
                    fullWord += next;
                    i++;
                }
            }
        }

        // Handle long words
        if (fullWord.length() > 50) {
            sf::Text testWord(fullWord, font, fontSize);
            if (testWord.getLocalBounds().width > maxWidth) {
                if (isLink) {
                    ClickableLink link;
                    link.url = fullWord;
                    sf::Text linkText(fullWord, font, fontSize);
                    linkText.setPosition(lineX, currentY);
                    linkText.setFillColor(sf::Color(100, 149, 237));
                    linkText.setStyle(sf::Text::Underlined);
                    link.bounds = linkText.getGlobalBounds();
                    activeLinks.push_back(link);
                    window.draw(linkText);
                    currentY += fontSize + 6;
                    lineX = x;
                    continue;
                }
            }
        }

        // Normal word wrapping
        sf::Text testText(currentLine + (currentLine.empty() ? "" : " ") + fullWord, font, fontSize);
        if (testText.getLocalBounds().width > maxWidth && !currentLine.empty()) {
            sf::Text lineText(currentLine, font, fontSize);
            lineText.setPosition(lineX, currentY);
            lineText.setFillColor(color);
            window.draw(lineText);
            currentY += fontSize + 6;
            currentLine = fullWord;
            lineX = x;
        } else {
            if (!currentLine.empty()) currentLine += " ";
            currentLine += fullWord;
        }

        if (isLink) {
            sf::Text linkText(fullWord, font, fontSize);
            linkText.setPosition(lineX, currentY);
            linkText.setFillColor(sf::Color(100, 149, 237));
            linkText.setStyle(sf::Text::Underlined);

            ClickableLink link;
            link.bounds = linkText.getGlobalBounds();
            link.url = fullWord;
            activeLinks.push_back(link);

            window.draw(linkText);
            lineX += linkText.getGlobalBounds().width + 5;
        }
    }

    if (!currentLine.empty() && currentY <= maxY) {
        sf::Text lineText(currentLine, font, fontSize);
        lineText.setPosition(lineX, currentY);
        lineText.setFillColor(color);
        window.draw(lineText);
    }
}

float EmailClientGUI::calculateTextHeight(const std::string& text, float maxWidth, unsigned int fontSize) {
    std::vector<std::string> words = splitIntoWords(text);
    std::string currentLine;
    int numLines = 1;

    for (const auto& word : words) {
        sf::Text testText(currentLine + " " + word, font, fontSize);
        if (testText.getLocalBounds().width > maxWidth && !currentLine.empty()) {
            numLines++;
            currentLine = word;
        } else {
            if (!currentLine.empty()) currentLine += " ";
            currentLine += word;
        }
    }

    return numLines * (fontSize + 6);  // Include line spacing
}
