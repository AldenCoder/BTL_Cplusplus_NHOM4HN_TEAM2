#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

enum class TransactionType {
    TRANSFER_IN,
    TRANSFER_OUT,
    TRANSFER,
    INITIAL,
    ROLLBACK
};

enum class TransactionStatus {
    PENDING,
    COMPLETED,
    FAILED,
    CANCELLED
};

struct Transaction {
    std::string transactionId;
    std::string fromWalletId;
    std::string toWalletId;
    double amount;
    TransactionType type;
    TransactionStatus status;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    std::string otpUsed;

    Transaction() = default;

    Transaction(const std::string& fromId, const std::string& toId, 
                double amt, TransactionType t, const std::string& desc = "");

    Transaction(const std::string& id, const std::string& fromId, const std::string& toId,
                double amt, TransactionType t, TransactionStatus stat, const std::string& desc);
                
    std::string getId() const { return transactionId; }
    double getAmount() const { return amount; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }
    std::string getFromWalletId() const { return fromWalletId; }
    std::string getToWalletId() const { return toWalletId; }
    std::string getDescription() const { return description; }
    TransactionType getType() const { return type; }
    TransactionStatus getStatus() const { return status; }

    std::string toJson() const;
    static Transaction fromJson(const std::string& json);
};

class Wallet {
protected:
    std::string walletId;
    std::string ownerId;
    double balance;
    std::vector<Transaction> transactions;
    std::chrono::system_clock::time_point createdAt;
    bool isLocked;

public:
    Wallet(const std::string& walletId, const std::string& ownerId, 
            double initialBalance = 0.0);

    const std::string& getWalletId() const { return walletId; }
    std::string getId() const { return walletId; }
    const std::string& getOwnerId() const { return ownerId; }
    double getBalance() const { return balance; }
    const std::vector<Transaction>& getTransactions() const { return transactions; }
    std::vector<Transaction> getTransactionHistory() const { return transactions; }
    bool getIsLocked() const { return isLocked; }
    bool isLockedStatus() const { return isLocked; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return createdAt; }

    bool hasSufficientBalance(double amount) const;
    bool deposit(double amount);
    bool withdraw(double amount);
    void setId(const std::string& newId) { walletId = newId; }

    std::string transferOut(double amount, const std::string& toWalletId, 
                            const std::string& description = "");

    void receiveTransfer(double amount, const std::string& fromWalletId,
                        const std::string& transactionId, 
                        const std::string& description = "");

    bool cancelTransaction(const std::string& transactionId);
    bool confirmTransaction(const std::string& transactionId, 
                            const std::string& otpCode);

    std::vector<Transaction> getTransactionHistory(
        const std::chrono::system_clock::time_point& fromDate,
        const std::chrono::system_clock::time_point& toDate) const;

    void setLocked(bool locked) { isLocked = locked; }
    void addTransaction(const Transaction& transaction);
    std::string toJson() const;
    static std::unique_ptr<Wallet> fromJson(const std::string& json);

private:
    std::string generateTransactionId();
};

class MasterWallet : public Wallet {
private:
    static const std::string MASTER_WALLET_ID;
    static const std::string MASTER_OWNER_ID;

public:
    MasterWallet(double initialSupply = 1000000.0);
    std::string issuePoints(const std::string& toWalletId, double amount,
                            const std::string& description = "Initial points");
    bool hasEnoughPoints(double amount) const;
    double getTotalPoints() const;
    bool transferOut(double amount);
    static MasterWallet& getInstance();
};

#endif
