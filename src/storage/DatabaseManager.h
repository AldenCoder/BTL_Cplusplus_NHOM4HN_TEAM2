#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "../models/User.h"
#include "../models/Wallet.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <sqlite3.h>

#ifdef _WIN32
    #include "../thread_compat.h"
#else
    #include <mutex>
#endif

enum class BackupType {
    MANUAL,
    AUTO,
    EMERGENCY
};

struct BackupInfo {
    std::string backupId;
    std::string filename;
    BackupType type;
    std::chrono::system_clock::time_point timestamp;
    size_t fileSize;
    std::string checksum;
};

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    std::string backupDirectory;
    mutable std::mutex dbMutex;
    std::vector<BackupInfo> backupHistory;
    
    static const int MAX_BACKUP_COUNT = 10;
    static const int AUTO_BACKUP_INTERVAL_HOURS = 24;

    bool createTables();
    bool enableWALMode();
    
    sqlite3_stmt* prepareStatement(const std::string& sql);
    bool executeStatement(sqlite3_stmt* stmt);
    void finalizeStatement(sqlite3_stmt* stmt);
    
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

public:
    DatabaseManager(const std::string& dataDir = "data");
    ~DatabaseManager();
    bool initialize();

    bool saveUser(const User& user);
    bool saveUser(std::shared_ptr<User> user);
    std::unique_ptr<User> loadUser(const std::string& username);
    std::unique_ptr<User> loadUserByUsername(const std::string& username);
    std::unique_ptr<User> loadUserById(const std::string& userId);

    std::vector<std::shared_ptr<User>> loadAllUsers();
    bool updateUser(const User& user);
    bool deleteUser(const std::string& userId);

    bool saveWallet(const Wallet& wallet);
    bool saveWallet(std::shared_ptr<Wallet> wallet);
    std::shared_ptr<Wallet> loadWallet(const std::string& walletId);

    std::shared_ptr<Wallet> loadWalletByOwnerId(const std::string& ownerId);
    std::vector<std::shared_ptr<Wallet>> loadAllWallets();
    bool updateWallet(const Wallet& wallet);

    bool transferPoints(const std::string& fromWalletId, 
                       const std::string& toWalletId, 
                       double amount, 
                       const std::string& description);
    std::string transferPointsWithId(const std::string& fromWalletId, 
                                    const std::string& toWalletId, 
                                    double amount, 
                                    const std::string& description);

    std::string getMasterWalletId();
    bool saveTransaction(const Transaction& transaction);
    std::vector<Transaction> loadWalletTransactions(const std::string& walletId);

    bool createBackup(const std::string& description = "", BackupType type = BackupType::MANUAL);
    bool restoreFromBackup(const std::string& backupId);
    std::vector<BackupInfo> getBackupHistory() const;
    int cleanupOldBackups(int keepCount = MAX_BACKUP_COUNT);
    bool isReady() const;
    std::string getStatistics() const;
};

#endif
