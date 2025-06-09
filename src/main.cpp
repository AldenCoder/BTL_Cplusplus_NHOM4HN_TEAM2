#include "system/AuthSystem.h"
#include "ui/UserInterface.h"
#include <iostream>
#include <locale>

int main() {
    try {
        AuthSystem authSystem;
        UserInterface ui(authSystem);
        
        std::cout << "=================================================\n";
        std::cout << "    LOGIN AND WALLET POINT MANAGEMENT SYSTEM     \n";
        std::cout << "=================================================\n\n";
        
        ui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "System error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
