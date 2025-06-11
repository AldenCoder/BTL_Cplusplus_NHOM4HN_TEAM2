#include "WalletManager.h"
#include "../security/OTPManager.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

const double WalletManager::INITIAL_USER_POINTS = 100.0; // 100 điểm ban đầu

WalletManager::WalletManager(std::shared_ptr<DatabaseManager> dataManager, 
                            std::shared_ptr<OTPManager> otpManager)
    : dataManager(dataManager), otpManager(otpManager) {
}

bool WalletManager::initialize() {
    try {
        masterWallet = std::unique_ptr<MasterWallet>(new MasterWallet(10000000.0)); // 10 million initial points

        auto wallets = dataManager->loadAllWallets();
        
        for (auto& wallet : wallets) {
            if (wallet) {
                std::string walletId = wallet->getId();
                walletCache[walletId] = wallet;
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] WalletManager initialization failed: " << e.what() << std::endl;
        return false;
    }
}

bool WalletManager::createUserWallet(const std::string& userId, const std::string& walletId) {
    try {
        if (walletExists(walletId)) {
            return false;
        }

        auto wallet = std::shared_ptr<Wallet>(new Wallet(walletId, userId, INITIAL_USER_POINTS));

        if (dataManager->saveWallet(wallet)) {
            walletCache[walletId] = wallet;

            std::string masterWalletId = dataManager->getMasterWalletId();
            if (!masterWalletId.empty()) {
                Transaction initTransaction(
                    SecurityUtils::generateUUID(),
                    masterWalletId,
                    walletId,
                    INITIAL_USER_POINTS,
                    TransactionType::TRANSFER,
                    TransactionStatus::COMPLETED,
                    "Điểm khởi tạo cho user mới"
                );
                wallet->addTransaction(initTransaction);
            }
            
            return true;
        } else {
            std::cerr << "[ERROR] dataManager->saveWallet returned false!" << std::endl;
        }
        
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in createUserWallet: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<Wallet> WalletManager::getWallet(const std::string& walletId) {
    auto it = walletCache.find(walletId);
    if (it != walletCache.end()) {
        return it->second;
    }

    return loadWalletToCache(walletId);
}

std::shared_ptr<Wallet> WalletManager::getWalletByUserId(const std::string& userId) {
    try {
        for (const auto& pair : walletCache) {
            if (pair.second->getOwnerId() == userId) {
                return pair.second;
            }
        }

        auto wallet = dataManager->loadWalletByOwnerId(userId);
        if (wallet) {
            walletCache[wallet->getId()] = wallet;
        }
        return wallet;
    }
    catch (const std::exception& e) {
        std::cerr << "Loi tim vi theo user ID: " << e.what() << std::endl;
        return nullptr;
    }
}

TransferResult WalletManager::transferPoints(const TransferRequest& request) {
    
    TransferResult result;
    result.success = false;

    std::string validationError = validateTransferRequest(request);
    if (!validationError.empty()) {
        result.message = validationError;
        return result;
    }

    try {
        auto fromWallet = getWallet(request.fromWalletId);
        auto toWallet = getWallet(request.toWalletId);

        if (!fromWallet || !toWallet) {
            result.message = "Wallet not found.!";
            return result;
        }
        if (fromWallet->getIsLocked() || toWallet->getIsLocked()) {
            result.message = "The wallet has been locked!";
            return result;
        }

        if (fromWallet->getBalance() < request.amount) {
            result.message = "Insufficient balance!";
            return result;
        }

        if (!otpManager->verifyOTP(fromWallet->getOwnerId(), request.otpCode, OTPType::TRANSFER)) {
            result.message = "The OTP code is incorrect or has expired!";
            return result;
        }

        std::string transactionId = executeAtomicTransfer(
            fromWallet, toWallet, request.amount, request.description);

        if (!transactionId.empty()) {
            result.success = true;
            result.message = "Points transferred successfully!";
            result.transactionId = transactionId;
            result.newBalance = fromWallet->getBalance();

            dataManager->saveWallet(fromWallet);
            dataManager->saveWallet(toWallet);
        } else {
            result.message = "Transaction execution error!";
        }
    }
    catch (const std::exception& e) {
        result.message = "System error: " + std::string(e.what());
    }

    return result;
}

std::string WalletManager::generateTransferOTP(const std::string& fromUserId,
                                              const std::string& toWalletId,
                                              double amount) {
    try {
        std::string otpData = fromUserId + "|" + toWalletId + "|" + std::to_string(amount);
        return otpManager->generateOTP(fromUserId, OTPType::TRANSFER);
    }
    catch (const std::exception& e) {
        std::cerr << "Error generating OTP for point transfer: " << e.what() << std::endl;
        return "";
    }
}

double WalletManager::getBalance(const std::string& walletId) {
    auto wallet = getWallet(walletId);
    return wallet ? wallet->getBalance() : -1.0;
}

std::vector<Transaction> WalletManager::getTransactionHistory(const std::string& walletId, 
                                                             int limit) {
    std::vector<Transaction> transactions;
    
    auto wallet = getWallet(walletId);
    if (!wallet) {
        return transactions;
    }

    auto allTransactions = wallet->getTransactionHistory();

    std::sort(allTransactions.begin(), allTransactions.end(),
              [](const Transaction& a, const Transaction& b) {
                  return a.getTimestamp() > b.getTimestamp();
              });

    if (limit > 0 && allTransactions.size() > static_cast<size_t>(limit)) {
        allTransactions.resize(limit);
    }

    return allTransactions;
}

std::vector<Transaction> WalletManager::getTransactionHistoryByDate(
    const std::string& walletId,
    const std::chrono::system_clock::time_point& fromDate,
    const std::chrono::system_clock::time_point& toDate) {
    
    std::vector<Transaction> filteredTransactions;
    auto allTransactions = getTransactionHistory(walletId);

    for (const auto& transaction : allTransactions) {
        auto transactionTime = transaction.getTimestamp();
        if (transactionTime >= fromDate && transactionTime <= toDate) {
            filteredTransactions.push_back(transaction);
        }
    }

    return filteredTransactions;
}

std::vector<std::string> WalletManager::findWalletsByOwner(const std::string& ownerId) {
    std::vector<std::string> walletIds;
    
    try {
        // Tìm trong cache
        for (const auto& pair : walletCache) {
            if (pair.second->getOwnerId() == ownerId) {
                walletIds.push_back(pair.first);
            }
        }

        if (walletIds.empty()) {
            auto wallet = dataManager->loadWalletByOwnerId(ownerId);
            if (wallet) {
                walletIds.push_back(wallet->getId());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Loi tim vi theo owner: " << e.what() << std::endl;
    }

    return walletIds;
}

bool WalletManager::walletExists(const std::string& walletId) {
    return getWallet(walletId) != nullptr;
}

bool WalletManager::setWalletLocked(const std::string& walletId, bool locked) {
    try {
        auto wallet = getWallet(walletId);
        if (!wallet) {
            return false;
        }

        wallet->setLocked(locked);
        return dataManager->saveWallet(wallet);
    }
    catch (const std::exception& e) {
        std::cerr << "Loi khoa/mo vi: " << e.what() << std::endl;
        return false;
    }
}

std::string WalletManager::issuePointsFromMaster(const std::string& adminUserId,
                                                const std::string& toWalletId,
                                                double amount,
                                                const std::string& description,
                                                const std::string& otpCode) {
    
    try {
        // Verify OTP first
        if (!otpManager->verifyOTP(adminUserId, otpCode, OTPType::TRANSFER)) {
            std::cerr << "[ERROR] Invalid or expired OTP code!" << std::endl;
            return "";
        }

        auto toWallet = getWallet(toWalletId);
        if (!toWallet || toWallet->getIsLocked()) {
            return "";
        }

        std::string masterWalletId = dataManager->getMasterWalletId();
        if (masterWalletId.empty()) {
            std::cerr << "[ERROR] Cannot find master wallet ID!" << std::endl;
            return "";
        }

        if (!masterWallet->hasEnoughPoints(amount)) {
            std::cerr << "[ERROR] Master wallet insufficient balance!" << std::endl;
            return "";
        }

        std::string transactionId = dataManager->transferPointsWithId(
            masterWalletId,
            toWalletId,
            amount,
            description
        );

        if (!transactionId.empty()) {
            masterWallet->transferOut(amount);

            toWallet->deposit(amount);

            Transaction transaction(
                transactionId,
                masterWalletId,
                toWalletId,
                amount,
                TransactionType::INITIAL,
                TransactionStatus::COMPLETED,
                description
            );
            
            toWallet->addTransaction(transaction);
            
            logTransaction(transaction, "COMPLETED", "Admin issued points successfully");
            return transactionId;
        } else {
            std::cerr << "[ERROR] Failed to execute atomic transfer for master wallet issuance!" << std::endl;
        }

        return "";
    }
    catch (const std::exception& e) {
        std::cerr << "Loi phat hanh diem: " << e.what() << std::endl;
        return "";
    }
}

std::string WalletManager::getSystemStatistics() {
    try {
        std::ostringstream stats;

        int totalWallets = walletCache.size();
        double totalPoints = 0.0;
        int activeWallets = 0;
        int lockedWallets = 0;

        for (const auto& pair : walletCache) {
            auto wallet = pair.second;
            totalPoints += wallet->getBalance();
            
            if (wallet->getIsLocked()) {
                lockedWallets++;
            } else {
                activeWallets++;
            }
        }        stats << "===== THONG KE HE THONG VI =====\n";
        stats << "Tong so vi: " << totalWallets << "\n";
        stats << "Vi dang hoat dong: " << activeWallets << "\n";
        stats << "Vi bi khoa: " << lockedWallets << "\n";
        stats << "Tong diem trong he thong: " << std::fixed << std::setprecision(2) << totalPoints << "\n";
        stats << "Diem con lai trong vi tong: " << masterWallet->getTotalPoints() << "\n";
        
        return stats.str();
    }
    catch (const std::exception& e) {
        return "Loi lay thong ke: " + std::string(e.what());
    }
}

bool WalletManager::confirmPendingTransaction(const std::string& transactionId,
                                            const std::string& otpCode) {
    (void)transactionId; // Suppress warning - reserved for future pending transaction feature
    (void)otpCode;       // Suppress warning - reserved for future OTP verification

    return false;
}

bool WalletManager::cancelPendingTransaction(const std::string& transactionId,
                                           const std::string& reason) {
    (void)transactionId; // Suppress warning - reserved for future pending transaction feature
    (void)reason;        // Suppress warning - reserved for future cancellation reason logging

    return false;
}

bool WalletManager::saveAllWallets() {
    try {
        for (const auto& pair : walletCache) {
            if (!dataManager->saveWallet(pair.second)) {
                return false;
            }
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Loi luu tat ca vi: " << e.what() << std::endl;
        return false;
    }
}

void WalletManager::clearWalletCache() {
    walletCache.clear();
}

// Private methods

std::shared_ptr<Wallet> WalletManager::loadWalletToCache(const std::string& walletId) {
    try {
        auto wallet = dataManager->loadWallet(walletId);
        if (wallet) {
            walletCache[walletId] = wallet;
        }
        return wallet;
    }
    catch (const std::exception& e) {
        std::cerr << "Loi tai vi vao cache: " << e.what() << std::endl;
        return nullptr;
    }
}

void WalletManager::removeWalletFromCache(const std::string& walletId) {
    walletCache.erase(walletId);
}

std::string WalletManager::validateTransferRequest(const TransferRequest& request) {
    if (request.amount <= 0) {
        return "So diem phai lon hon 0!";
    }

    if (request.amount > 1000000) { // Giới hạn 1 triệu điểm/giao dịch
        return "So diem vuot qua gioi han cho phep!";
    }

    if (request.fromWalletId == request.toWalletId) {
        return "Khong the chuyen diem cho chinh minh!";
    }

    if (request.otpCode.empty()) {
        return "Can ma OTP de xac thuc giao dich!";
    }

    if (request.description.empty()) {
        return "Can co mo ta cho giao dich!";
    }

    return ""; // Hợp lệ
}

std::string WalletManager::executeAtomicTransfer(std::shared_ptr<Wallet> fromWallet,
                                               std::shared_ptr<Wallet> toWallet,
                                               double amount,
                                               const std::string& description) {
    try {
        std::string transactionId = dataManager->transferPointsWithId(
            fromWallet->getId(), 
            toWallet->getId(), 
            amount, 
            description
        );
        
        if (!transactionId.empty()) {
            fromWallet->withdraw(amount);
            toWallet->deposit(amount);

            Transaction fromTransaction(
                transactionId,
                fromWallet->getId(),
                toWallet->getId(),
                amount,
                TransactionType::TRANSFER_OUT,
                TransactionStatus::COMPLETED,
                description
            );
            
            Transaction toTransaction(
                transactionId,
                fromWallet->getId(),
                toWallet->getId(),
                amount,
                TransactionType::TRANSFER_IN,
                TransactionStatus::COMPLETED,
                description
            );

            fromWallet->addTransaction(fromTransaction);
            toWallet->addTransaction(toTransaction);

            logTransaction(fromTransaction, "COMPLETED", "Transfer executed successfully");
            
            return transactionId;
        } else {
            return "";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Loi thuc hien giao dich atomic: " << e.what() << std::endl;
        return "";
    }
}

void WalletManager::rollbackTransfer(std::shared_ptr<Wallet> fromWallet,
                                   std::shared_ptr<Wallet> toWallet,
                                   double amount,
                                   const std::string& transactionId) {
    try {
        toWallet->withdraw(amount);
        fromWallet->deposit(amount);

        Transaction rollbackTransaction(
            SecurityUtils::generateUUID(),
            "SYSTEM",
            "SYSTEM",
            amount,
            TransactionType::ROLLBACK,
            TransactionStatus::COMPLETED,
            "Rollback transaction " + transactionId
        );

        fromWallet->addTransaction(rollbackTransaction);
        toWallet->addTransaction(rollbackTransaction);

        logTransaction(rollbackTransaction, "ROLLBACK", "Transaction rolled back");
    }
    catch (const std::exception& e) {
        std::cerr << "Loi rollback giao dich: " << e.what() << std::endl;
    }
}

void WalletManager::logTransaction(const Transaction& transaction,
                                 const std::string& status,
                                 const std::string& message) {
    try {
        std::cout << "[TRANSACTION LOG] " 
                  << "ID: " << transaction.getId()
                  << ", Status: " << status
                  << ", Amount: " << transaction.getAmount()
                  << ", Message: " << message << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Loi ghi log giao dich: " << e.what() << std::endl;
    }
}
