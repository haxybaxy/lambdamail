#include "EmailClientGUI.h"
#include <iostream>

// Main function to run the Email Client GUI application
int main() {
    try {
        EmailClientGUI gui; // Create an instance of the EmailClientGUI
        gui.run(); // Start the application
    } catch (const std::exception& e) { // Catch and handle any exceptions
        std::cerr << "Error: " << e.what() << std::endl; // Output the error message
        return 1; // Return an error code
    }
    return 0; // Return success code
}
