#include "src/storage/DatabaseManager.h"
#include "src/models/Wallet.h"
#include <iostream>

int main() {
    std::cout << "Testing transaction saving..." << std::endl;
    
    // Initialize database manager
    DatabaseManager dbManager("data");
    if (!dbManager.initialize()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return 1;
    }
    
    // Get current wallet IDs
    auto wallets = dbManager.loadAllWallets();
    if (wallets.size() < 2) {
        std::cerr << "Need at least 2 wallets for testing!" << std::endl;
        return 1;
    }
    
    std::string fromWalletId = wallets[0]->getId();
    std::string toWalletId = wallets[1]->getId();
    
    std::cout << "From wallet: " << fromWalletId << std::endl;
    std::cout << "To wallet: " << toWalletId << std::endl;
    
    // Create a test transaction
    Transaction testTransaction(
        "test-transaction-123",
        fromWalletId,
        toWalletId,
        5.0,
        TransactionType::TRANSFER,
        TransactionStatus::COMPLETED,
        "Test transaction"
    );
    
    std::cout << "Attempting to save transaction..." << std::endl;
    bool success = dbManager.saveTransaction(testTransaction);
    
    if (success) {
        std::cout << "✓ Transaction saved successfully!" << std::endl;
    } else {
        std::cout << "✗ Failed to save transaction!" << std::endl;
    }
    
    return success ? 0 : 1;
}
