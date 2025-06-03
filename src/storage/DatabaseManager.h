/**
 * @file DatabaseManager.h
 * @brief SQLite-based data storage manager
 * @author Team 2C
 * 
 * This replaces the file-based JSON storage with SQLite to provide:
 * - ACID compliance for data integrity
 * - Concurrent access safety with built-in locking
 * - Atomic transactions to prevent data corruption
 * - Better performance and reliability
 */

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "../models/User.h"
#include "../models/Wallet.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <sqlite3.h>

/**
 * @enum BackupType
 * @brief Backup type enumeration
 */
enum class BackupType {
    MANUAL,     // Manual backup
    AUTO,       // Automatic backup
    EMERGENCY   // Emergency backup
};

/**
 * @struct BackupInfo
 * @brief Backup information structure
 */
struct BackupInfo {
    std::string backupId;
    std::string filename;
    BackupType type;
    std::chrono::system_clock::time_point timestamp;
    size_t fileSize;
    std::string checksum;
};

/**
 * @class DatabaseManager
 * @brief SQLite-based database manager for ACID-compliant storage
 * 
 * Features:
 * - Thread-safe operations with mutex locking
 * - ACID transactions for data integrity
 * - Atomic operations to prevent race conditions
 * - Built-in SQLite file locking for concurrent access
 * - Backup and recovery functionality
 */
class DatabaseManager {
private:
    sqlite3* db;                        // SQLite database connection
    std::string dbPath;                 // Database file path
    std::string backupDirectory;        // Backup directory path
    mutable std::mutex dbMutex;         // Thread-safety mutex
    std::vector<BackupInfo> backupHistory;
    
    static const int MAX_BACKUP_COUNT = 10;
    static const int AUTO_BACKUP_INTERVAL_HOURS = 24;

    // Database initialization and schema
    bool createTables();
    bool enableWALMode();
    
    // Helper methods for prepared statements
    sqlite3_stmt* prepareStatement(const std::string& sql);
    bool executeStatement(sqlite3_stmt* stmt);
    void finalizeStatement(sqlite3_stmt* stmt);
    
    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

public:
    /**
     * @brief Constructor
     * @param dataDir Data directory (default: "data")
     */
    DatabaseManager(const std::string& dataDir = "data");
    
    /**
     * @brief Destructor - ensures proper database closure
     */
    ~DatabaseManager();

    /**
     * @brief Initialize database and create schema
     * @return true if successful
     */
    bool initialize();

    // ==================== USER DATA MANAGEMENT ====================
    
    /**
     * @brief Save user information (thread-safe, ACID)
     * @param user User to save
     * @return true if successful
     */
    bool saveUser(const User& user);
    bool saveUser(std::shared_ptr<User> user);

    /**
     * @brief Load user by username (thread-safe)
     * @param username Username to search
     * @return Unique pointer to User (nullptr if not found)
     */
    std::unique_ptr<User> loadUser(const std::string& username);
    std::unique_ptr<User> loadUserByUsername(const std::string& username);

    /**
     * @brief Load user by ID (thread-safe)
     * @param userId User ID to search
     * @return Unique pointer to User (nullptr if not found)
     */
    std::unique_ptr<User> loadUserById(const std::string& userId);

    /**
     * @brief Load all users (thread-safe)
     * @return Vector of all users
     */
    std::vector<std::shared_ptr<User>> loadAllUsers();

    /**
     * @brief Update user information (atomic transaction)
     * @param user Updated user information
     * @return true if successful
     */
    bool updateUser(const User& user);

    /**
     * @brief Delete user (atomic with cascade to wallet)
     * @param userId User ID to delete
     * @return true if successful
     */
    bool deleteUser(const std::string& userId);

    // ==================== WALLET DATA MANAGEMENT ====================
    
    /**
     * @brief Save wallet information (thread-safe, ACID)
     * @param wallet Wallet to save
     * @return true if successful
     */
    bool saveWallet(const Wallet& wallet);
    bool saveWallet(std::shared_ptr<Wallet> wallet);

    /**
     * @brief Load wallet by ID (thread-safe)
     * @param walletId Wallet ID
     * @return Shared pointer to Wallet (nullptr if not found)
     */
    std::shared_ptr<Wallet> loadWallet(const std::string& walletId);

    /**
     * @brief Load wallet by owner ID (thread-safe)
     * @param ownerId Owner's user ID
     * @return Shared pointer to Wallet (nullptr if not found)
     */
    std::shared_ptr<Wallet> loadWalletByOwnerId(const std::string& ownerId);

    /**
     * @brief Load all wallets (thread-safe)
     * @return Vector of all wallets
     */
    std::vector<std::shared_ptr<Wallet>> loadAllWallets();

    /**
     * @brief Update wallet information (atomic transaction)
     * @param wallet Updated wallet information
     * @return true if successful
     */
    bool updateWallet(const Wallet& wallet);

    /**
     * @brief Atomic transfer between wallets (ACID compliant)
     * @param fromWalletId Source wallet ID
     * @param toWalletId Destination wallet ID
     * @param amount Transfer amount
     * @param description Transaction description
     * @return true if successful
     */
    bool transferPoints(const std::string& fromWalletId, 
                       const std::string& toWalletId, 
                       double amount, 
                       const std::string& description);

    // ==================== TRANSACTION HISTORY ====================
    
    /**
     * @brief Save transaction record
     * @param transaction Transaction to save
     * @return true if successful
     */
    bool saveTransaction(const Transaction& transaction);

    /**
     * @brief Load transactions for a wallet
     * @param walletId Wallet ID
     * @return Vector of transactions
     */
    std::vector<Transaction> loadWalletTransactions(const std::string& walletId);

    // ==================== BACKUP MANAGEMENT ====================
    
    /**
     * @brief Create database backup
     * @param description Optional backup description
     * @param type Backup type
     * @return true if successful
     */
    bool createBackup(const std::string& description = "", BackupType type = BackupType::MANUAL);

    /**
     * @brief Restore database from backup
     * @param backupId Backup ID to restore from
     * @return true if successful
     */
    bool restoreFromBackup(const std::string& backupId);

    /**
     * @brief Get backup history
     * @return Vector of backup information
     */
    std::vector<BackupInfo> getBackupHistory() const;

    /**
     * @brief Clean old backups
     * @param keepCount Number of backups to keep
     * @return Number of backups deleted
     */
    int cleanupOldBackups(int keepCount = MAX_BACKUP_COUNT);

    // ==================== UTILITY METHODS ====================
    
    /**
     * @brief Check if database is initialized and ready
     * @return true if ready
     */
    bool isReady() const;

    /**
     * @brief Get database statistics
     * @return Statistics string
     */
    std::string getStatistics() const;
};

#endif // DATABASE_MANAGER_H
