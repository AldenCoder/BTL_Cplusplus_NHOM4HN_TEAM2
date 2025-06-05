#include <iostream>
#include <sqlite3.h>

int main() {
    std::cout << "=== DEBUG: Transaction Foreign Key Issue ===" << std::endl;
    
    // Use known wallet IDs
    std::string fromWalletId = "62873a28-81bd-4092-b0c6-2655a218cff1";
    std::string toWalletId = "9a058d71-ab56-489b-a8b4-1f1316a02845";
    
    std::cout << "From wallet: '" << fromWalletId << "'" << std::endl;
    std::cout << "To wallet: '" << toWalletId << "'" << std::endl;
    
    // Test direct database access
    sqlite3* db;
    int rc = sqlite3_open("data/wallet_system.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    
    // Enable foreign keys
    sqlite3_exec(db, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);
    
    // Check if wallets exist
    const char* checkSql = "SELECT COUNT(*) FROM wallets WHERE wallet_id = ?";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, fromWalletId.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            std::cout << "From wallet exists: " << count << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    
    rc = sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, toWalletId.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            std::cout << "To wallet exists: " << count << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    
    // Try direct insert
    const char* insertSql = "INSERT INTO transactions (transaction_id, from_wallet_id, to_wallet_id, amount, description, transaction_type, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?)";
    rc = sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, "debug-test-123", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, fromWalletId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, toWalletId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, 5.0);
        sqlite3_bind_text(stmt, 5, "Debug test", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, 1);
        sqlite3_bind_int64(stmt, 7, 1749141290);
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            std::cout << "✓ Direct insert successful!" << std::endl;
        } else {
            std::cout << "✗ Direct insert failed: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    return 0;
}
