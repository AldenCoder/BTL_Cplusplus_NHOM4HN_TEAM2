#include "OTPStorage.h"
#include <sqlite3.h>
#include <iostream>
#include <chrono>

static const char* DB_PATH = "data/wallet_system.db";

bool OTPStorage::saveOTP(const std::string& userId, const std::string& purpose,
                        const std::string& otpCode, int expiresAfterSec) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) return false;
    sqlite3_stmt* stmt = nullptr;

    // Xoá OTP cũ (nếu có) cho userId + purpose
    const char* delSQL = "DELETE FROM otps WHERE user_id=? AND purpose=?;";
    sqlite3_prepare_v2(db, delSQL, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, purpose.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Thêm OTP mới
    int expires = static_cast<int>(std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + std::chrono::seconds(expiresAfterSec)));

    const char* insSQL = "INSERT INTO otps(user_id, purpose, otp_code, expires_at) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, insSQL, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, purpose.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, otpCode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, expires);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
}

std::string OTPStorage::getOTP(const std::string& userId, const std::string& purpose) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) return "";
    sqlite3_stmt* stmt = nullptr;
    std::string otp = "";
    int now = static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    const char* sql = "SELECT otp_code FROM otps WHERE user_id=? AND purpose=? AND expires_at>=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, purpose.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, now);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        otp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return otp;
}

void OTPStorage::removeOTP(const std::string& userId, const std::string& purpose) {
    sqlite3* db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) return;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM otps WHERE user_id=? AND purpose=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, purpose.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void OTPStorage::cleanupExpiredOTP() {
    sqlite3* db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) return;
    sqlite3_stmt* stmt = nullptr;
    int now = static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    const char* sql = "DELETE FROM otps WHERE expires_at < ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, now);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}