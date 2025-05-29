/**
 * @file DataManager.h
 * @brief Data storage and backup management
 * @author Team 2C
 */

#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "../models/User.h"
#include "../models/Wallet.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <chrono>
#include <sys/stat.h>  // For file operations instead of filesystem

/**
 * @enum BackupType
 * @brief Backup type
 */
enum class BackupType {
    MANUAL,     // Manual backup
    AUTO,       // Automatic backup
    EMERGENCY   // Emergency backup
};

/**
 * @struct BackupInfo
 * @brief Backup information
 */
struct BackupInfo {
    std::string backupId;       // Backup ID
    std::string filename;       // Backup filename
    BackupType type;           // Backup type
    std::chrono::system_clock::time_point timestamp; // Backup timestamp
    size_t fileSize;           // File size
    std::string checksum;      // Checksum for data integrity
};

/**
 * @class DataManager
 * @brief Data, storage and backup management
 * 
 * Features:
 * - Save/load user and wallet data
 * - Periodic data backup
 * - Data restoration from backup
 * - Data integrity validation
 */
class DataManager {
private:
    std::string dataDirectory;      // Data directory
    std::string backupDirectory;    // Backup directory
    std::string usersFile;          // Users list file
    std::string walletsFile;        // Wallets list file
    
    std::vector<BackupInfo> backupHistory;  // Backup history
    
    static const int MAX_BACKUP_COUNT = 10;  // Maximum number of backups
    static const int AUTO_BACKUP_INTERVAL_HOURS = 24; // Auto backup every 24h

public:
    /**
     * @brief Constructor
     * @param dataDir Data directory (default: "data")
     */
    DataManager(const std::string& dataDir = "data");

    /**
     * @brief Initialize storage system
     * @return true if successful
     */
    bool initialize();

    // ==================== USER DATA MANAGEMENT ====================
    
    /**
     * @brief Save user information
     * @param user User to save
     * @return true if successful
     */
    bool saveUser(const User& user);    /**
     * @brief Load user information by username
     * @param username Username
     * @return Unique pointer to User (nullptr if not found)
     */
    std::unique_ptr<User> loadUser(const std::string& username);

    /**
     * @brief Load user information by username (alias for loadUser)
     * @param username Username
     * @return Unique pointer to User (nullptr if not found)
     */
    std::unique_ptr<User> loadUserByUsername(const std::string& username);

    /**
     * @brief Load user information by ID
     * @param userId User ID
     * @return Unique pointer to User (nullptr if not found)
     */
    std::unique_ptr<User> loadUserById(const std::string& userId);

    /**
     * @brief Save user (overload for shared_ptr)
     * @param user Shared pointer to User to save
     * @return true if successful
     */
    bool saveUser(std::shared_ptr<User> user);

    /**
     * @brief Load all users
     * @return Vector containing all users
     */
    std::vector<std::unique_ptr<User>> loadAllUsers();

    /**
     * @brief Delete user
     * @param username Username
     * @return true if successful
     */
    bool deleteUser(const std::string& username);

    /**
     * @brief Check if user exists
     * @param username Username
     * @return true if exists
     */
    bool userExists(const std::string& username);

    // ==================== WALLET DATA MANAGEMENT ====================
      /**
     * @brief Save wallet information
     * @param wallet Wallet to save
     * @return true if successful
     */
    bool saveWallet(const Wallet& wallet);

    /**
     * @brief Save wallet information from shared_ptr
     * @param wallet Shared pointer to Wallet to save
     * @return true if successful
     */
    bool saveWallet(std::shared_ptr<Wallet> wallet);

    /**
     * @brief Load wallet by ID
     * @param walletId Wallet ID
     * @return Unique pointer to Wallet (nullptr if not found)
     */
    std::unique_ptr<Wallet> loadWallet(const std::string& walletId);

    /**
     * @brief Load wallet by ID (returns shared_ptr)
     * @param walletId Wallet ID
     * @return Shared pointer to Wallet
     */
    std::shared_ptr<Wallet> loadWalletById(const std::string& walletId);

    /**
     * @brief Load wallet by user ID
     * @param userId User ID
     * @return Shared pointer to Wallet
     */
    std::shared_ptr<Wallet> loadWalletByUserId(const std::string& userId);

    /**
     * @brief Load all wallets
     * @return Vector containing all wallets
     */
    std::vector<std::unique_ptr<Wallet>> loadAllWallets();

    /**
     * @brief Load wallet by owner ID
     * @param ownerId Owner ID
     * @return Unique pointer to Wallet
     */
    std::unique_ptr<Wallet> loadWalletByOwner(const std::string& ownerId);    /**
     * @brief Check if wallet exists
     * @param walletId Wallet ID
     * @return true if exists
     */
    bool walletExists(const std::string& walletId);

    // ==================== BACKUP & RECOVERY ====================
    
    /**
     * @brief Create backup
     * @param type Backup type
     * @param description Description (optional)
     * @return BackupInfo of the created backup
     */
    BackupInfo createBackup(BackupType type, const std::string& description = "");

    /**
     * @brief Restore from backup
     * @param backupId Backup ID
     * @return true if restoration successful
     */
    bool restoreFromBackup(const std::string& backupId);

    /**
     * @brief Get backup history list
     * @return Vector containing backup information
     */
    std::vector<BackupInfo> getBackupHistory() const { return backupHistory; }

    /**
     * @brief Delete old backups (keep maximum number)
     */
    void cleanupOldBackups();

    /**
     * @brief Check and create auto backup if needed
     */
    void checkAutoBackup();

private:
    // ==================== HELPER METHODS ====================
      /**
     * @brief Helper method to load wallet by owner with shared_ptr
     */
    std::shared_ptr<Wallet> loadWalletByOwner_shared(const std::string& ownerId);

    /**
     * @brief Verify data integrity
     * @return true if data is intact
     */
    bool verifyDataIntegrity();

    // ==================== UTILITY FUNCTIONS ====================
    
    /**
     * @brief Get data statistics
     * @return String containing statistics information
     */
    std::string getDataStatistics();

    /**
     * @brief Clean temporary files and unnecessary data
     */
    void cleanup();

    /**
     * @brief Export data to CSV file (for reporting)
     * @param outputPath Output file path
     * @return true if successful
     */
    bool exportToCSV(const std::string& outputPath);

private:
    /**
     * @brief Create directory if it doesn't exist
     * @param path Directory path
     * @return true if successful
     */
    bool createDirectoryIfNotExists(const std::string& path);

    /**
     * @brief Read JSON file
     * @param filepath File path
     * @return File content (empty if error)
     */
    std::string readJsonFile(const std::string& filepath);

    /**
     * @brief Write JSON file
     * @param filepath File path
     * @param content JSON content
     * @return true if successful
     */
    bool writeJsonFile(const std::string& filepath, const std::string& content);

    /**
     * @brief Calculate file checksum
     * @param filepath File path
     * @return Checksum string
     */
    std::string calculateChecksum(const std::string& filepath);

    /**
     * @brief Compress data directory
     * @param sourceDir Source directory
     * @param targetFile Target compressed file
     * @return true if successful
     */
    bool compressDirectory(const std::string& sourceDir, const std::string& targetFile);

    /**
     * @brief Extract backup file
     * @param backupFile Backup file
     * @param targetDir Target directory
     * @return true if successful
     */
    bool extractBackup(const std::string& backupFile, const std::string& targetDir);

    /**
     * @brief Load backup history from file
     */
    void loadBackupHistory();

    /**
     * @brief Save backup history to file
     */
    void saveBackupHistory();

    /**
     * @brief Generate unique backup ID
     * @return ID string
     */
    std::string generateBackupId();
};

#endif // DATA_MANAGER_H