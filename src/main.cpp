#include "EmailClientGUI.h"
#include <iostream>

int main() {
    try {
        EmailClientGUI gui;
        gui.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}