#include "DatabaseManager.h"
#include "../security/SecurityUtils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime> // For localtime_s and localtime_r

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>  // Add stat support for Windows
#define mkdir _mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

DatabaseManager::DatabaseManager(const std::string& dataDir) 
    : db(nullptr), 
      dbPath(dataDir + "/wallet_system.db"),
      backupDirectory(dataDir + "/backup") {
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool DatabaseManager::initialize() {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    // Create data directory if it doesn't exist
    struct stat st;
    if (stat("data", &st) != 0) {
        #ifdef _WIN32
        mkdir("data");
        #else
        mkdir("data", 0755);
        #endif
    }
    
    // Create backup directory if it doesn't exist
    if (stat(backupDirectory.c_str(), &st) != 0) {
        #ifdef _WIN32
        mkdir(backupDirectory.c_str());
        #else
        mkdir(backupDirectory.c_str(), 0755);
        #endif
    }
    
    // Open SQLite database
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Enable WAL mode for better concurrency
    if (!enableWALMode()) {
        std::cerr << "Failed to enable WAL mode" << std::endl;
        return false;
    }
    
    // Create tables
    if (!createTables()) {
        std::cerr << "Failed to create database tables" << std::endl;
        return false;
    }
    
    std::cout << "Database initialized successfully: " << dbPath << std::endl;
    return true;
}

bool DatabaseManager::enableWALMode() {
    const char* sql = "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL; PRAGMA foreign_keys=ON;";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "WAL mode error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::createTables() {
    const char* userTableSQL = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            full_name TEXT NOT NULL,
            email TEXT NOT NULL,
            phone_number TEXT NOT NULL,
            role INTEGER NOT NULL,
            is_password_generated INTEGER DEFAULT 0,
            is_first_login INTEGER DEFAULT 0,
            wallet_id TEXT,
            created_at INTEGER NOT NULL,
            last_login INTEGER DEFAULT 0
        );
    )";
    
    const char* walletTableSQL = R"(
        CREATE TABLE IF NOT EXISTS wallets (
            wallet_id TEXT PRIMARY KEY,
            owner_id TEXT NOT NULL,
            balance REAL NOT NULL DEFAULT 0.0,
            created_at INTEGER NOT NULL,
            is_locked INTEGER DEFAULT 0,
            FOREIGN KEY (owner_id) REFERENCES users (user_id) ON DELETE CASCADE
        );
    )";
    
    const char* transactionTableSQL = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            transaction_id TEXT PRIMARY KEY,
            from_wallet_id TEXT,
            to_wallet_id TEXT,
            amount REAL NOT NULL,
            description TEXT,
            transaction_type INTEGER NOT NULL,
            timestamp INTEGER NOT NULL,
            FOREIGN KEY (from_wallet_id) REFERENCES wallets (wallet_id),
            FOREIGN KEY (to_wallet_id) REFERENCES wallets (wallet_id)
        );
    )";
    
    const char* indexSQL = R"(
        CREATE INDEX IF NOT EXISTS idx_username ON users(username);
        CREATE INDEX IF NOT EXISTS idx_wallet_owner ON wallets(owner_id);
        CREATE INDEX IF NOT EXISTS idx_transaction_from ON transactions(from_wallet_id);
        CREATE INDEX IF NOT EXISTS idx_transaction_to ON transactions(to_wallet_id);
    )";
    
    char* errMsg = nullptr;
    
    // Create users table
    int rc = sqlite3_exec(db, userTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create users table error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    // Create wallets table
    rc = sqlite3_exec(db, walletTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create wallets table error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    // Create transactions table
    rc = sqlite3_exec(db, transactionTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create transactions table error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    // Create indexes
    rc = sqlite3_exec(db, indexSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create indexes error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

sqlite3_stmt* DatabaseManager::prepareStatement(const std::string& sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Prepare statement error: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return stmt;
}

bool DatabaseManager::executeStatement(sqlite3_stmt* stmt) {
    int rc = sqlite3_step(stmt);
    
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "[ERROR] SQL execution failed: " << sqlite3_errmsg(db) 
                  << " (error code: " << rc << ")" << std::endl;
        return false;
    }
    return true;
}

void DatabaseManager::finalizeStatement(sqlite3_stmt* stmt) {
    if (stmt) {
        sqlite3_finalize(stmt);
    }
}

bool DatabaseManager::beginTransaction() {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Begin transaction error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::commitTransaction() {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Commit transaction error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::rollbackTransaction() {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Rollback transaction error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// ==================== USER MANAGEMENT ====================

bool DatabaseManager::saveUser(const User& user) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    // Start explicit transaction
    if (!beginTransaction()) {
        return false;
    }
    
    // Check if user exists first
    const char* checkSql = "SELECT COUNT(*) FROM users WHERE user_id = ?;";
    sqlite3_stmt* checkStmt = prepareStatement(checkSql);
    if (!checkStmt) {
        rollbackTransaction();
        return false;
    }
    
    sqlite3_bind_text(checkStmt, 1, user.getUserId().c_str(), -1, SQLITE_STATIC);
    bool userExists = false;
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        userExists = sqlite3_column_int(checkStmt, 0) > 0;
    }
    finalizeStatement(checkStmt);
    
    const char* sql;
    if (userExists) {
        // Update existing user - this preserves the user_id and won't trigger CASCADE DELETE
        sql = R"(
            UPDATE users SET 
                username = ?, password_hash = ?, full_name = ?, email = ?, phone_number = ?, 
                role = ?, is_password_generated = ?, is_first_login = ?, wallet_id = ?, 
                created_at = ?, last_login = ?
            WHERE user_id = ?;
        )";
    } else {
        // Insert new user
        sql = R"(
            INSERT INTO users 
            (user_id, username, password_hash, full_name, email, phone_number, 
             role, is_password_generated, is_first_login, wallet_id, created_at, last_login)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
        )";
    }
    
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) {
        rollbackTransaction();
        return false;
    }
    
    // Convert time_point to unix timestamp
    auto createdAt = std::chrono::duration_cast<std::chrono::seconds>(
        user.getCreatedAt().time_since_epoch()).count();
    auto lastLogin = std::chrono::duration_cast<std::chrono::seconds>(
        user.getLastLogin().time_since_epoch()).count();
    
    if (userExists) {
        // Bind parameters for UPDATE (user_id is in WHERE clause, so it's last)
        sqlite3_bind_text(stmt, 1, user.getUsername().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user.getPasswordHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user.getFullName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, user.getEmail().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, user.getPhoneNumber().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, static_cast<int>(user.getRole()));
        sqlite3_bind_int(stmt, 7, user.getIsPasswordGenerated() ? 1 : 0);
        sqlite3_bind_int(stmt, 8, user.getIsFirstLogin() ? 1 : 0);
        sqlite3_bind_text(stmt, 9, user.getWalletId().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 10, createdAt);
        sqlite3_bind_int64(stmt, 11, lastLogin);
        sqlite3_bind_text(stmt, 12, user.getUserId().c_str(), -1, SQLITE_STATIC);  // WHERE clause
    } else {
        // Bind parameters for INSERT
        sqlite3_bind_text(stmt, 1, user.getUserId().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user.getUsername().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user.getPasswordHash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, user.getFullName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, user.getEmail().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, user.getPhoneNumber().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 7, static_cast<int>(user.getRole()));
        sqlite3_bind_int(stmt, 8, user.getIsPasswordGenerated() ? 1 : 0);
        sqlite3_bind_int(stmt, 9, user.getIsFirstLogin() ? 1 : 0);
        sqlite3_bind_text(stmt, 10, user.getWalletId().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 11, createdAt);
        sqlite3_bind_int64(stmt, 12, lastLogin);
    }
    
    bool success = executeStatement(stmt);
    finalizeStatement(stmt);
    
    if (success) {
        commitTransaction();
    } else {
        rollbackTransaction();
    }
    
    return success;
}

bool DatabaseManager::saveUser(std::shared_ptr<User> user) {
    return user ? saveUser(*user) : false;
}

std::unique_ptr<User> DatabaseManager::loadUser(const std::string& username) {
    return loadUserByUsername(username);
}

std::unique_ptr<User> DatabaseManager::loadUserByUsername(const std::string& username) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    const char* sql = "SELECT * FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return nullptr;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), // user_id
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // username
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), // password_hash
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)), // full_name
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)), // email
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)), // phone_number
            static_cast<UserRole>(sqlite3_column_int(stmt, 6))           // role
        );
        
        user->setIsPasswordGenerated(sqlite3_column_int(stmt, 7) == 1);
        user->setIsFirstLogin(sqlite3_column_int(stmt, 8) == 1);
        user->setWalletId(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        // Skip setting timestamps for now - User class doesn't provide public setters
    }
    
    finalizeStatement(stmt);
    return user;
}

std::unique_ptr<User> DatabaseManager::loadUserById(const std::string& userId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    const char* sql = "SELECT * FROM users WHERE user_id = ?;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return nullptr;
    
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    
    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), // user_id
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // username
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), // password_hash
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)), // full_name
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)), // email
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)), // phone_number
            static_cast<UserRole>(sqlite3_column_int(stmt, 6))           // role
        );
        
        user->setIsPasswordGenerated(sqlite3_column_int(stmt, 7) == 1);
        user->setIsFirstLogin(sqlite3_column_int(stmt, 8) == 1);
        user->setWalletId(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        // Skip setting timestamps for now - User class doesn't provide public setters
    }
    
    finalizeStatement(stmt);
    return user;
}

std::vector<std::shared_ptr<User>> DatabaseManager::loadAllUsers() {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::vector<std::shared_ptr<User>> users;
    
    const char* sql = "SELECT * FROM users ORDER BY username;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return users;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_shared<User>(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), // user_id
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // username
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), // password_hash
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)), // full_name
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)), // email
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)), // phone_number
            static_cast<UserRole>(sqlite3_column_int(stmt, 6))           // role
        );
        
        user->setIsPasswordGenerated(sqlite3_column_int(stmt, 7) == 1);
        user->setIsFirstLogin(sqlite3_column_int(stmt, 8) == 1);
        user->setWalletId(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        // Skip setting timestamps for now - User class doesn't provide public setters
        
        users.push_back(user);
    }
    
    finalizeStatement(stmt);
    return users;
}

bool DatabaseManager::updateUser(const User& user) {
    return saveUser(user); // INSERT OR REPLACE handles updates
}

bool DatabaseManager::deleteUser(const std::string& userId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (!beginTransaction()) return false;
    
    // Delete user (wallet will be deleted by foreign key cascade)
    const char* sql = "DELETE FROM users WHERE user_id = ?;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) {
        rollbackTransaction();
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    bool success = executeStatement(stmt);
    finalizeStatement(stmt);
    
    if (success) {
        commitTransaction();
    } else {
        rollbackTransaction();
    }
    
    return success;
}

// ==================== WALLET MANAGEMENT ====================

bool DatabaseManager::saveWallet(const Wallet& wallet) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (!db) {
        std::cerr << "[ERROR] Database connection is null!" << std::endl;
        return false;
    }
    
    // Start explicit transaction
    if (!beginTransaction()) {
        std::cerr << "[ERROR] Failed to begin transaction!" << std::endl;
        return false;
    }
    
    const char* sql = R"(
        INSERT OR REPLACE INTO wallets 
        (wallet_id, owner_id, balance, created_at, is_locked)
        VALUES (?, ?, ?, ?, ?);
    )";
    
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) {
        std::cerr << "[ERROR] Failed to prepare statement!" << std::endl;
        rollbackTransaction();
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, wallet.getWalletId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, wallet.getOwnerId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, wallet.getBalance());
    // Set current timestamp for created_at
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    sqlite3_bind_int64(stmt, 4, now);
    sqlite3_bind_int(stmt, 5, wallet.getIsLocked() ? 1 : 0);
    
    bool success = executeStatement(stmt);
    finalizeStatement(stmt);
    
    if (success) {
        if (commitTransaction()) {
            // Force WAL checkpoint to ensure data is written to main database
            char* errMsg = nullptr;
            int rc = sqlite3_exec(db, "PRAGMA wal_checkpoint(FULL);", nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK) {
                std::cerr << "[WARNING] WAL checkpoint failed: " << errMsg << std::endl;
                sqlite3_free(errMsg);
            }
            
            // Verify wallet was actually saved by immediately querying it
            const char* verifySql = "SELECT COUNT(*) FROM wallets WHERE wallet_id = ?;";
            sqlite3_stmt* verifyStmt = prepareStatement(verifySql);
            if (verifyStmt) {
                sqlite3_bind_text(verifyStmt, 1, wallet.getWalletId().c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(verifyStmt) == SQLITE_ROW) {
                    int count = sqlite3_column_int(verifyStmt, 0);
                    if (count == 0) {
                        std::cerr << "[ERROR] CRITICAL: Wallet was not actually saved to database!" << std::endl;
                    }
                }
                finalizeStatement(verifyStmt);
            }
            
            // Also verify that owner_id exists in users table (foreign key check)
            const char* fkSql = "SELECT COUNT(*) FROM users WHERE user_id = ?;";
            sqlite3_stmt* fkStmt = prepareStatement(fkSql);
            if (fkStmt) {
                sqlite3_bind_text(fkStmt, 1, wallet.getOwnerId().c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(fkStmt) == SQLITE_ROW) {
                    int userCount = sqlite3_column_int(fkStmt, 0);
                    if (userCount == 0) {
                        std::cerr << "[ERROR] Foreign key constraint violation: User " << wallet.getOwnerId() << " does not exist!" << std::endl;
                    }
                }
                finalizeStatement(fkStmt);
            }
        } else {
            std::cerr << "[ERROR] Failed to commit transaction!" << std::endl;
            rollbackTransaction();
            return false;
        }
    } else {
        std::cerr << "[ERROR] Failed to save wallet to database!" << std::endl;
        rollbackTransaction();
        return false;
    }
    
    return success;
}

bool DatabaseManager::saveWallet(std::shared_ptr<Wallet> wallet) {
    return wallet ? saveWallet(*wallet) : false;
}

std::shared_ptr<Wallet> DatabaseManager::loadWallet(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    const char* sql = "SELECT * FROM wallets WHERE wallet_id = ?;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return nullptr;
    
    sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);
    
    std::shared_ptr<Wallet> wallet = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        wallet = std::make_shared<Wallet>(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), // wallet_id
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // owner_id
            sqlite3_column_double(stmt, 2)                               // balance
        );
        
        // Skip setting created_at since Wallet doesn't have setter
        wallet->setLocked(sqlite3_column_int(stmt, 4) == 1);
        
        // Load transactions for this wallet
        auto transactions = loadWalletTransactions(walletId);
        for (const auto& tx : transactions) {
            wallet->addTransaction(tx);
        }
    }
    
    finalizeStatement(stmt);
    return wallet;
}

std::shared_ptr<Wallet> DatabaseManager::loadWalletByOwnerId(const std::string& ownerId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    const char* sql = "SELECT * FROM wallets WHERE owner_id = ?;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) {
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, ownerId.c_str(), -1, SQLITE_STATIC);
    
    std::shared_ptr<Wallet> wallet = nullptr;
    int result = sqlite3_step(stmt);
    
    if (result == SQLITE_ROW) {
        std::string walletId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        
        wallet = std::make_shared<Wallet>(
            walletId,                                                    // wallet_id
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // owner_id
            sqlite3_column_double(stmt, 2)                               // balance
        );
        
        // Skip setting created_at since Wallet doesn't have setter
        wallet->setLocked(sqlite3_column_int(stmt, 4) == 1);
        
        // Load transactions for this wallet
        auto transactions = loadWalletTransactions(walletId);
        for (const auto& tx : transactions) {
            wallet->addTransaction(tx);
        }
    }
    
    finalizeStatement(stmt);
    return wallet;
}

std::vector<std::shared_ptr<Wallet>> DatabaseManager::loadAllWallets() {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::vector<std::shared_ptr<Wallet>> wallets;
    
    const char* sql = "SELECT * FROM wallets ORDER BY wallet_id;";
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return wallets;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string walletId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        auto wallet = std::make_shared<Wallet>(
            walletId,                                                    // wallet_id
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // owner_id
            sqlite3_column_double(stmt, 2)                               // balance
        );
        
        // Skip setting created_at since Wallet doesn't have setter
        wallet->setLocked(sqlite3_column_int(stmt, 4) == 1);
        
        wallets.push_back(wallet);
    }
    
    finalizeStatement(stmt);
    return wallets;
}

bool DatabaseManager::updateWallet(const Wallet& wallet) {
    return saveWallet(wallet); // INSERT OR REPLACE handles updates
}

bool DatabaseManager::transferPoints(const std::string& fromWalletId, 
                                    const std::string& toWalletId, 
                                    double amount, 
                                    const std::string& description) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (!beginTransaction()) return false;
    
    try {
        // Get source wallet balance
        const char* checkSql = "SELECT balance FROM wallets WHERE wallet_id = ?;";
        sqlite3_stmt* checkStmt = prepareStatement(checkSql);
        if (!checkStmt) {
            rollbackTransaction();
            return false;
        }
        
        sqlite3_bind_text(checkStmt, 1, fromWalletId.c_str(), -1, SQLITE_STATIC);
        
        double fromBalance = 0.0;
        if (sqlite3_step(checkStmt) == SQLITE_ROW) {
            fromBalance = sqlite3_column_double(checkStmt, 0);
        } else {
            finalizeStatement(checkStmt);
            rollbackTransaction();
            return false;
        }
        finalizeStatement(checkStmt);
        
        if (fromBalance < amount) {
            rollbackTransaction();
            return false; // Insufficient funds
        }
        
        // Update source wallet
        const char* debitSql = "UPDATE wallets SET balance = balance - ? WHERE wallet_id = ?;";
        sqlite3_stmt* debitStmt = prepareStatement(debitSql);
        if (!debitStmt) {
            rollbackTransaction();
            return false;
        }
        
        sqlite3_bind_double(debitStmt, 1, amount);
        sqlite3_bind_text(debitStmt, 2, fromWalletId.c_str(), -1, SQLITE_STATIC);
        
        if (!executeStatement(debitStmt)) {
            finalizeStatement(debitStmt);
            rollbackTransaction();
            return false;
        }
        finalizeStatement(debitStmt);
        
        // Update destination wallet
        const char* creditSql = "UPDATE wallets SET balance = balance + ? WHERE wallet_id = ?;";
        sqlite3_stmt* creditStmt = prepareStatement(creditSql);
        if (!creditStmt) {
            rollbackTransaction();
            return false;
        }
        
        sqlite3_bind_double(creditStmt, 1, amount);
        sqlite3_bind_text(creditStmt, 2, toWalletId.c_str(), -1, SQLITE_STATIC);
        
        if (!executeStatement(creditStmt)) {
            finalizeStatement(creditStmt);
            rollbackTransaction();
            return false;
        }
        finalizeStatement(creditStmt);
        
        // Record transaction
        std::string transactionId = SecurityUtils::generateUUID();
        Transaction transaction(transactionId, fromWalletId, toWalletId, amount, 
                              TransactionType::TRANSFER, TransactionStatus::COMPLETED, description);
        
        if (!saveTransaction(transaction)) {
            rollbackTransaction();
            return false;
        }
        
        commitTransaction();
        return true;
        
    } catch (const std::exception& e) {
        rollbackTransaction();
        return false;
    }
}

// ==================== TRANSACTION MANAGEMENT ====================

bool DatabaseManager::saveTransaction(const Transaction& transaction) {
    const char* sql = R"(
        INSERT INTO transactions 
        (transaction_id, from_wallet_id, to_wallet_id, amount, description, transaction_type, timestamp)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )";
    
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return false;
    
    sqlite3_bind_text(stmt, 1, transaction.getId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, transaction.getFromWalletId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, transaction.getToWalletId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, transaction.getAmount());
    sqlite3_bind_text(stmt, 5, transaction.getDescription().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, static_cast<int>(transaction.getType()));
    sqlite3_bind_int64(stmt, 7, std::chrono::duration_cast<std::chrono::seconds>(
        transaction.getTimestamp().time_since_epoch()).count());
    
    bool success = executeStatement(stmt);
    finalizeStatement(stmt);
    
    return success;
}

std::vector<Transaction> DatabaseManager::loadWalletTransactions(const std::string& walletId) {
    std::vector<Transaction> transactions;
    
    const char* sql = R"(
        SELECT * FROM transactions 
        WHERE from_wallet_id = ? OR to_wallet_id = ? 
        ORDER BY timestamp DESC;
    )";
    
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) return transactions;
    
    sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, walletId.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string transactionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string fromWalletId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string toWalletId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        double amount = sqlite3_column_double(stmt, 3);
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        TransactionType type = static_cast<TransactionType>(sqlite3_column_int(stmt, 5));
        
        // auto timestamp = std::chrono::system_clock::from_time_t(sqlite3_column_int64(stmt, 6));  // Unused for now
        
        transactions.emplace_back(transactionId, fromWalletId, toWalletId, amount, 
                                type, TransactionStatus::COMPLETED, description);
    }
    
    finalizeStatement(stmt);
    return transactions;
}

// ==================== BACKUP MANAGEMENT ====================

bool DatabaseManager::createBackup(const std::string& description, BackupType type) {
    (void)description; // Suppress warning - parameter reserved for future use
    (void)type;        // Suppress warning - parameter reserved for future use
    
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (!db) return false;
      // Generate backup filename
    auto now = std::chrono::system_clock::now();
    auto time_t_val = std::chrono::system_clock::to_time_t(now);
    
    std::tm* time_info = std::localtime(&time_t_val);
    std::stringstream ss;
    ss << backupDirectory << "/backup_";
    if (time_info) {
        ss << std::put_time(time_info, "%Y%m%d_%H%M%S");
    } else {
        ss << "backup_" << time_t_val;
    }
    ss << ".db";
    std::string backupPath = ss.str();
    
    // Use SQLite backup API
    sqlite3* backupDb;
    int rc = sqlite3_open(backupPath.c_str(), &backupDb);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot create backup database: " << sqlite3_errmsg(backupDb) << std::endl;
        return false;
    }
    
    sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", db, "main");
    if (!backup) {
        std::cerr << "Cannot initialize backup: " << sqlite3_errmsg(backupDb) << std::endl;
        sqlite3_close(backupDb);
        return false;
    }
    
    rc = sqlite3_backup_step(backup, -1);
    if (rc != SQLITE_DONE) {
        std::cerr << "Backup failed: " << sqlite3_errmsg(backupDb) << std::endl;
        sqlite3_backup_finish(backup);
        sqlite3_close(backupDb);
        return false;
    }
    
    sqlite3_backup_finish(backup);
    sqlite3_close(backupDb);
    
    // Create backup info
    BackupInfo info;
    info.backupId = SecurityUtils::generateUUID();
    info.filename = backupPath;
    info.type = type;
    info.timestamp = now;
    info.fileSize = 0; // You could calculate actual file size here
    info.checksum = ""; // You could calculate checksum here
    
    backupHistory.push_back(info);
    
    std::cout << "Database backup created: " << backupPath << std::endl;
    return true;
}

bool DatabaseManager::restoreFromBackup(const std::string& backupId) {
    // Find backup
    auto it = std::find_if(backupHistory.begin(), backupHistory.end(),
        [&backupId](const BackupInfo& info) {
            return info.backupId == backupId;
        });
    
    if (it == backupHistory.end()) {
        std::cerr << "Backup not found: " << backupId << std::endl;
        return false;
    }
    
    std::lock_guard<std::mutex> lock(dbMutex);
    
    // Close current database
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    
    // Copy backup file to main database
    std::ifstream src(it->filename, std::ios::binary);
    std::ofstream dst(dbPath, std::ios::binary);
    
    if (!src || !dst) {
        std::cerr << "Cannot copy backup file" << std::endl;
        return false;
    }
    
    dst << src.rdbuf();
    src.close();
    dst.close();
    
    // Reopen database
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot reopen database after restore: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    enableWALMode();
    
    std::cout << "Database restored from backup: " << it->filename << std::endl;
    return true;
}

std::vector<BackupInfo> DatabaseManager::getBackupHistory() const {
    return backupHistory;
}

int DatabaseManager::cleanupOldBackups(int keepCount) {
    if (backupHistory.size() <= static_cast<size_t>(keepCount)) {
        return 0;
    }
    
    // Sort by timestamp (newest first)
    std::sort(backupHistory.begin(), backupHistory.end(),
        [](const BackupInfo& a, const BackupInfo& b) {
            return a.timestamp > b.timestamp;
        });
    
    int deletedCount = 0;
    for (size_t i = keepCount; i < backupHistory.size(); ++i) {
        if (std::remove(backupHistory[i].filename.c_str()) == 0) {
            deletedCount++;
        }
    }
    
    backupHistory.resize(keepCount);
    return deletedCount;
}

// ==================== UTILITY METHODS ====================

bool DatabaseManager::isReady() const {
    return db != nullptr;
}

std::string DatabaseManager::getStatistics() const {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (!db) return "Database not initialized";
    
    std::stringstream ss;
    
    // Count users
    const char* userCountSql = "SELECT COUNT(*) FROM users;";
    sqlite3_stmt* stmt = sqlite3_prepare_v2(db, userCountSql, -1, nullptr, nullptr) == SQLITE_OK ? 
                         sqlite3_prepare_v2(db, userCountSql, -1, &stmt, nullptr) == SQLITE_OK ? stmt : nullptr : nullptr;
    
    if (stmt && sqlite3_step(stmt) == SQLITE_ROW) {
        ss << "Users: " << sqlite3_column_int(stmt, 0) << "\n";
    }
    if (stmt) sqlite3_finalize(stmt);
    
    // Count wallets
    const char* walletCountSql = "SELECT COUNT(*) FROM wallets;";
    stmt = sqlite3_prepare_v2(db, walletCountSql, -1, &stmt, nullptr) == SQLITE_OK ? stmt : nullptr;
    if (stmt && sqlite3_step(stmt) == SQLITE_ROW) {
        ss << "Wallets: " << sqlite3_column_int(stmt, 0) << "\n";
    }
    if (stmt) sqlite3_finalize(stmt);
    
    // Count transactions
    const char* txCountSql = "SELECT COUNT(*) FROM transactions;";
    stmt = sqlite3_prepare_v2(db, txCountSql, -1, &stmt, nullptr) == SQLITE_OK ? stmt : nullptr;
    if (stmt && sqlite3_step(stmt) == SQLITE_ROW) {
        ss << "Transactions: " << sqlite3_column_int(stmt, 0) << "\n";
    }
    if (stmt) sqlite3_finalize(stmt);
    
    ss << "Database: " << dbPath;
    return ss.str();
}
