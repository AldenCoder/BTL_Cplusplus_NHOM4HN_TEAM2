#include "system/AuthSystem.h"
#include "ui/UserInterface.h"
#include <iostream>
#include <locale>

int main() {
      try {
        // Initialize authentication system
        AuthSystem authSystem;
        
        // Initialize user interface
        UserInterface ui(authSystem);
        
        // Display startup information
        std::cout << "=================================================\n";
        std::cout << "    LOGIN AND WALLET POINT MANAGEMENT SYSTEM    \n";
        std::cout << "=================================================\n\n";
        
        // Run main interface
        ui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "System error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
