#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

enum class TransactionType {
    TRANSFER_IN,    // Receive points 0
    TRANSFER_OUT,   // Send points 1
    TRANSFER,       // General transfer transaction 2
    INITIAL,        // Initial points from master wallet 3
    ROLLBACK        // Transaction rollback 4
};

enum class TransactionStatus {
    PENDING,        // Waiting for confirmation
    COMPLETED,      // Completed
    FAILED,         // Failed
    CANCELLED       // Cancelled
};
struct Transaction {
    std::string transactionId;          // Transaction ID (UUID)
    std::string fromWalletId;          // Sender wallet ID
    std::string toWalletId;            // Receiver wallet ID
    double amount;                     // Points amount
    TransactionType type;              // Transaction type
    TransactionStatus status;          // Status
    std::string description;           // Transaction description
    std::chrono::system_clock::time_point timestamp;  // Transaction time
    std::string otpUsed;              // Used OTP (if any)

    /**
     * Default constructor
     */
    Transaction() = default;

    /**
     * Constructor
     */
    Transaction(const std::string& fromId, const std::string& toId, 
                double amt, TransactionType t, const std::string& desc = "");

    /**
     * Constructor with full parameters
     */
    Transaction(const std::string& id, const std::string& fromId, const std::string& toId,
                double amt, TransactionType t, TransactionStatus stat, const std::string& desc);    // Getter methods
    std::string getId() const { return transactionId; }
    double getAmount() const { return amount; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }
    std::string getFromWalletId() const { return fromWalletId; }
    std::string getToWalletId() const { return toWalletId; }
    std::string getDescription() const { return description; }
    TransactionType getType() const { return type; }
    TransactionStatus getStatus() const { return status; }

    /**
     * @brief Convert transaction to JSON
     */
    std::string toJson() const;

    /**
     * @brief Create transaction from JSON
     */
    static Transaction fromJson(const std::string& json);
};

/**
 * @class Wallet
 * @brief Reward points wallet management class
 * 
 * Each user has a wallet with the following features:
 * - Store points balance
 * - Execute point transfer transactions
 * - Store transaction history
 * - Verify transactions with OTP
 */
class Wallet {
protected:  // Changed from private to protected for inheritance
    std::string walletId;              // Unique wallet ID
    std::string ownerId;               // Owner ID
    double balance;                    // Current balance
    std::vector<Transaction> transactions;  // Transaction history
    std::chrono::system_clock::time_point createdAt;  // Creation time
    bool isLocked;                     // Is wallet locked

public:
    /**
     * @brief Constructor to create new wallet
     * @param walletId Wallet ID
     * @param ownerId Owner ID
     * @param initialBalance Initial balance (default 0)
     */
    Wallet(const std::string& walletId, const std::string& ownerId, 
            double initialBalance = 0.0);

    // Getter methods
    const std::string& getWalletId() const { return walletId; }
    std::string getId() const { return walletId; } // Alias for getWalletId
    const std::string& getOwnerId() const { return ownerId; }
    double getBalance() const { return balance; }
    const std::vector<Transaction>& getTransactions() const { return transactions; }
    std::vector<Transaction> getTransactionHistory() const { return transactions; } // Overload without parameters
    bool getIsLocked() const { return isLocked; }
    bool isLockedStatus() const { return isLocked; } // Alias for getIsLocked
    const std::chrono::system_clock::time_point& getCreatedAt() const { return createdAt; }

    /**
     * @brief Check if balance is sufficient for transaction
     * @param amount Points amount to check
     * @return true if sufficient balance
     */
    bool hasSufficientBalance(double amount) const;

    /**
     * @brief Deposit points to wallet
     * @param amount Points to deposit
     * @return true if successful
     */
    bool deposit(double amount);

    /**
     * @brief Withdraw points from wallet
     * @param amount Points to withdraw
     * @return true if successful
     */
    bool withdraw(double amount);

    /**
     * @brief Set wallet ID (needed for some cases)
     * @param newId New ID
     */
    void setId(const std::string& newId) { walletId = newId; }

    /**
     * @brief Execute outbound point transfer transaction
     * @param amount Points to transfer
     * @param toWalletId Destination wallet ID
     * @param description Transaction description
     * @return Transaction ID if successful, empty string if failed
     */
    std::string transferOut(double amount, const std::string& toWalletId, 
                            const std::string& description = "");

    /**
     * @brief Receive points from transaction
     * @param amount Points to receive
     * @param fromWalletId Sender wallet ID
     * @param transactionId Transaction ID
     * @param description Description
     */
    void receiveTransfer(double amount, const std::string& fromWalletId,
                        const std::string& transactionId, 
                        const std::string& description = "");

    /**
     * @brief Cancel transaction (rollback)
     * @param transactionId Transaction ID to cancel
     * @return true if successfully cancelled
     */
    bool cancelTransaction(const std::string& transactionId);

    /**
     * @brief Confirm transaction
     * @param transactionId Transaction ID
     * @param otpCode OTP verification code
     * @return true if successfully confirmed
     */
    bool confirmTransaction(const std::string& transactionId, 
                            const std::string& otpCode);

    /**
     * @brief Get transaction history within time range
     * @param fromDate From date
     * @param toDate To date
     * @return Vector of transactions within time range
     */
    std::vector<Transaction> getTransactionHistory(
        const std::chrono::system_clock::time_point& fromDate,
        const std::chrono::system_clock::time_point& toDate) const;

    /**
     * @brief Lock/unlock wallet
     * @param locked true to lock, false to unlock
     */
    void setLocked(bool locked) { isLocked = locked; }

    /**
     * @brief Add transaction to history (public for WalletManager access)
     * @param transaction Transaction to add
     */
    void addTransaction(const Transaction& transaction);

    /**
     * @brief Convert wallet to JSON for file storage
     * @return JSON string
     */
    std::string toJson() const;

    /**
     * @brief Create wallet from JSON string
     * @param json JSON string
     * @return Unique pointer to Wallet object
     */
    static std::unique_ptr<Wallet> fromJson(const std::string& json);

private:
    /**
     * @brief Generate unique transaction ID
     * @return UUID string
     */
    std::string generateTransactionId();
};

/**
 * @class MasterWallet
 * @brief Master wallet - source of points generation in the system
 * 
 * Special wallet capable of creating new points and distributing to other wallets
 */
class MasterWallet : public Wallet {
private:
    static const std::string MASTER_WALLET_ID;
    static const std::string MASTER_OWNER_ID;

public:
    /**
     * @brief Constructor to create master wallet
     * @param initialSupply Total initial points in the system
     */
    MasterWallet(double initialSupply = 1000000.0);

    /**
     * @brief Issue new points to wallet
     * @param toWalletId Recipient wallet ID
     * @param amount Points to issue
     * @param description Reason for issuance
     * @return Transaction ID
     */
    std::string issuePoints(const std::string& toWalletId, double amount,
                            const std::string& description = "Initial points");

    /**
     * @brief Check if master wallet has enough points
     * @param amount Points to check
     * @return true if sufficient
     */
    bool hasEnoughPoints(double amount) const;

    /**
     * @brief Get total points in master wallet
     * @return Current points available
     */
    double getTotalPoints() const;

    /**
     * @brief Transfer points out of master wallet (simple)
     * @param amount Points to transfer
     * @return true if successful
     */
    bool transferOut(double amount);

    /**
     * @brief Get unique instance of master wallet (Singleton)
     * @return Reference to master wallet
     */
    static MasterWallet& getInstance();
};

#endif