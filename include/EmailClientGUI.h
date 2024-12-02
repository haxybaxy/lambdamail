#pragma once
#include <SFML/Graphics.hpp>
#include "MailTM.h"
#include <queue>
#include <mutex>

class EmailClientGUI {
private:
    sf::RenderWindow window;
    MailTMAPI::MailTM mailTm;

    // Resources
    sf::Font font;

    // Email data
    std::string email;
    std::string password;
    std::string accountId;
    bool isEmailGenerated;

    // Message handling
    std::vector<Json::Value> messages;
    std::mutex messagesMutex;
    float scrollOffset;

    // GUI elements
    sf::Text inputPrompt;
    std::string inputBuffer;
    bool isGenerating;

    // Private methods
    void drawMainInterface();
    void drawMessages();
    void checkInboxThread();
    void generateEmail();
    void handleMouseClick(int x, int y);
    void handleScroll(float delta);
    void deleteAccount();

    // Add these member variables to the private section:
    bool isCustomUsername;
    std::string customUsername;
    bool isInputActive;

    // Add to private section:
    bool isValidUsername(const std::string& username);

    // Add these new structures to help manage clickable links
    struct ClickableLink {
        sf::FloatRect bounds;
        std::string url;
    };

    // Add to class private members in EmailClientGUI.h
    std::vector<ClickableLink> activeLinks;

    // Add these helper function declarations
    std::string stripHtmlExceptLinks(const std::string& html);
    std::vector<std::string> splitIntoWords(const std::string& text);
    void openUrl(const std::string& url);

public:
    EmailClientGUI();
    void run();
};
