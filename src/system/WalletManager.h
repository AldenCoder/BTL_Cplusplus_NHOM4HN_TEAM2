#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include "../models/Wallet.h"
#include "../security/SecurityUtils.h"
#include "../security/OTPManager.h"
#include "../storage/DatabaseManager.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include "../thread_compat.h"

struct TransferRequest {
    std::string fromWalletId;
    std::string toWalletId;
    double amount;
    std::string description;
    std::string otpCode;
};

struct TransferResult {
    bool success;
    std::string message;
    std::string transactionId;
    double newBalance;
};
class WalletManager {
private:
    std::shared_ptr<DatabaseManager> dataManager;
    std::shared_ptr<OTPManager> otpManager;
    std::unique_ptr<MasterWallet> masterWallet;
    
    std::unordered_map<std::string, std::shared_ptr<Wallet>> walletCache;
    // std::mutex transferMutex;
    
    static const double INITIAL_USER_POINTS;

public:
    WalletManager(std::shared_ptr<DatabaseManager> dataManager, 
                  std::shared_ptr<OTPManager> otpManager);

    bool initialize();
    bool createUserWallet(const std::string& userId, const std::string& walletId);
    std::shared_ptr<Wallet> getWallet(const std::string& walletId);
    std::shared_ptr<Wallet> getWalletByUserId(const std::string& userId);
    TransferResult transferPoints(const TransferRequest& request);
    std::string generateTransferOTP(const std::string& fromUserId,
                                   const std::string& toWalletId,
                                   double amount);
    double getBalance(const std::string& walletId);
    std::vector<Transaction> getTransactionHistory(const std::string& walletId, 
                                                  int limit = -1);
    std::vector<Transaction> getTransactionHistoryByDate(
        const std::string& walletId,
        const std::chrono::system_clock::time_point& fromDate,
        const std::chrono::system_clock::time_point& toDate);
    std::vector<std::string> findWalletsByOwner(const std::string& ownerId);
    bool walletExists(const std::string& walletId);
    bool setWalletLocked(const std::string& walletId, bool locked);
    std::string issuePointsFromMaster(const std::string& adminUserId,
                                     const std::string& toWalletId,
                                     double amount,
                                     const std::string& description,
                                     const std::string& otpCode);
    std::string getSystemStatistics();
    bool confirmPendingTransaction(const std::string& transactionId,
                                  const std::string& otpCode);
    bool cancelPendingTransaction(const std::string& transactionId,
                                 const std::string& reason = "User cancelled");
    bool saveAllWallets();
    void clearWalletCache();

private:
    std::shared_ptr<Wallet> loadWalletToCache(const std::string& walletId);
    void removeWalletFromCache(const std::string& walletId);
    std::string validateTransferRequest(const TransferRequest& request);
    std::string executeAtomicTransfer(std::shared_ptr<Wallet> fromWallet,
                                     std::shared_ptr<Wallet> toWallet,
                                     double amount,
                                     const std::string& description);
    void rollbackTransfer(std::shared_ptr<Wallet> fromWallet,
                         std::shared_ptr<Wallet> toWallet,
                         double amount,
                         const std::string& transactionId);
    void logTransaction(const Transaction& transaction,
                       const std::string& status,
                       const std::string& message);
};

#endif

