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

public:
    EmailClientGUI();
    void run();
};
