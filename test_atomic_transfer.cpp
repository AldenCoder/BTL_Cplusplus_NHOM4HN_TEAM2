#include "src/storage/DatabaseManager.h"
#include <iostream>

int main() {
    std::cout << "=== Testing Transaction with DatabaseManager ===" << std::endl;
    
    // Initialize database manager
    DatabaseManager dbManager("data");
    if (!dbManager.initialize()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return 1;
    }
    
    // Test with transferPointsWithId instead
    std::string transactionId = dbManager.transferPointsWithId(
        "62873a28-81bd-4092-b0c6-2655a218cff1",  // from
        "9a058d71-ab56-489b-a8b4-1f1316a02845",  // to 
        10.0,                                     // amount
        "Test transfer with atomic method"        // description
    );
    
    if (!transactionId.empty()) {
        std::cout << "✓ Transfer successful! Transaction ID: " << transactionId << std::endl;
        return 0;
    } else {
        std::cout << "✗ Transfer failed!" << std::endl;
        return 1;
    }
}
