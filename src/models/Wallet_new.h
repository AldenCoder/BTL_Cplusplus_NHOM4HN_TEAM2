/**
 * @file Wallet.h
 * @brief Wallet and transaction management for points system
 * @author Team 2C
 */

#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

/**
 * @enum TransactionType
 * @brief Type of transaction in wallet
 */
enum class TransactionType {
    TRANSFER_IN,    // Receive points
    TRANSFER_OUT,   // Send points
    TRANSFER,       // General point transfer
    INITIAL,        // Initial points from master wallet
    ROLLBACK        // Transaction rollback
};

/**
 * @enum TransactionStatus
 * @brief Transaction status
 */
enum class TransactionStatus {
    PENDING,        // Waiting for confirmation
    COMPLETED,      // Completed
    FAILED,         // Failed
    CANCELLED       // Cancelled
};

/**
 * @struct Transaction
 * @brief Transaction data structure
 */
struct Transaction {
    std::string transactionId;          // Transaction ID (UUID)
    std::string fromWalletId;          // Sender wallet ID
    std::string toWalletId;            // Receiver wallet ID
    double amount;                     // Point amount
    TransactionType type;              // Transaction type
    TransactionStatus status;          // Status
    std::string description;           // Description
    std::chrono::system_clock::time_point timestamp;  // Timestamp
    std::string otpUsed;              // OTP used (if any)

    /**
     * Constructor
     */
    Transaction(const std::string& fromId, const std::string& toId, 
                double amt, TransactionType t, const std::string& desc = "");

    /**
     * Constructor with full parameters
     */
    Transaction(const std::string& id, const std::string& fromId, const std::string& toId,
                double amt, TransactionType t, TransactionStatus stat, const std::string& desc);

    // Getter methods
    std::string getId() const { return transactionId; }
    double getAmount() const { return amount; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }

    /**
     * Convert transaction to JSON
     */
    std::string toJson() const;

    /**
     * Create transaction from JSON
     */
    static Transaction fromJson(const std::string& json);
};

/**
 * @class Wallet
 * @brief Wallet management class for points system
 * 
 * Each user has one wallet with features:
 * - Store point balance
 * - Perform point transfers
 * - Store transaction history
 * - OTP verification for transactions
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
     * Constructor to create new wallet
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

    /**
     * Check if wallet has sufficient balance for transaction
     * @param amount Amount to check
     * @return true if sufficient balance
     */
    bool hasSufficientBalance(double amount) const;

    /**
     * Deposit points to wallet
     * @param amount Amount to deposit
     * @return true if successful
     */
    bool deposit(double amount);

    /**
     * Withdraw points from wallet
     * @param amount Amount to withdraw
     * @return true if successful
     */
    bool withdraw(double amount);

    /**
     * Set wallet ID (needed for some cases)
     * @param newId New ID
     */
    void setId(const std::string& newId) { walletId = newId; }

    /**
     * Perform outgoing point transfer
     * @param amount Points to transfer
     * @param toWalletId Destination wallet ID
     * @param description Transaction description
     * @return Transaction ID if successful, empty string if failed
     */
    std::string transferOut(double amount, const std::string& toWalletId, 
                           const std::string& description = "");

    /**
     * Receive points from transfer
     * @param amount Points received
     * @param fromWalletId Sender wallet ID
     * @param transactionId Transaction ID
     * @param description Description
     */
    void receiveTransfer(double amount, const std::string& fromWalletId,
                        const std::string& transactionId, 
                        const std::string& description = "");

    /**
     * Cancel transaction (rollback)
     * @param transactionId Transaction ID to cancel
     * @return true if cancelled successfully
     */
    bool cancelTransaction(const std::string& transactionId);

    /**
     * Confirm transaction
     * @param transactionId Transaction ID
     * @param otpCode OTP verification code
     * @return true if confirmed successfully
     */
    bool confirmTransaction(const std::string& transactionId, 
                           const std::string& otpCode);

    /**
     * Get transaction history in time range
     * @param fromDate Start date
     * @param toDate End date
     * @return Vector of transactions in time range
     */
    std::vector<Transaction> getTransactionHistory(
        const std::chrono::system_clock::time_point& fromDate,
        const std::chrono::system_clock::time_point& toDate) const;

    /**
     * Lock/unlock wallet
     * @param locked true to lock, false to unlock
     */
    void setLocked(bool locked) { isLocked = locked; }

    /**
     * Add transaction to history (public for WalletManager access)
     * @param transaction Transaction to add
     */
    void addTransaction(const Transaction& transaction);

    /**
     * Convert wallet to JSON for file storage
     * @return JSON string
     */
    std::string toJson() const;

    /**
     * Create wallet from JSON string
     * @param json JSON string
     * @return Unique pointer to Wallet object
     */
    static std::unique_ptr<Wallet> fromJson(const std::string& json);

private:
    /**
     * Generate unique transaction ID
     * @return UUID string
     */
    std::string generateTransactionId();
};

/**
 * @class MasterWallet
 * @brief Master wallet - source of points in the system
 * 
 * Special wallet that can create new points and distribute to other wallets
 */
class MasterWallet : public Wallet {
private:
    static const std::string MASTER_WALLET_ID;
    static const std::string MASTER_OWNER_ID;

public:
    /**
     * Constructor to create master wallet
     * @param initialSupply Total initial points in system
     */
    MasterWallet(double initialSupply = 1000000.0);

    /**
     * Issue new points to wallet
     * @param toWalletId Receiver wallet ID
     * @param amount Points to issue
     * @param description Reason for issuing
     * @return Transaction ID
     */
    std::string issuePoints(const std::string& toWalletId, double amount,
                           const std::string& description = "Initial points");

    /**
     * Check if master wallet has enough points
     * @param amount Amount to check
     * @return true if enough
     */
    bool hasEnoughPoints(double amount) const;

    /**
     * Get total points in master wallet
     * @return Current points available
     */
    double getTotalPoints() const;

    /**
     * Transfer points out of master wallet (simple)
     * @param amount Points to transfer
     * @return true if successful
     */
    bool transferOut(double amount);

    /**
     * Get unique instance of master wallet (Singleton)
     * @return Reference to master wallet
     */
    static MasterWallet& getInstance();
};

#endif // WALLET_H
