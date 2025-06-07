#pragma once
#include <string>
#include <chrono>

class OTPStorage {
public:
    // Lưu OTP vào DB
    static bool saveOTP(const std::string& userId, const std::string& purpose,
                        const std::string& otpCode, int expiresAfterSec = 300);

    // Lấy OTP (nếu còn hạn)
    static std::string getOTP(const std::string& userId, const std::string& purpose);

    // Xóa OTP (sau khi dùng hoặc hết hạn)
    static void removeOTP(const std::string& userId, const std::string& purpose);

    // Xóa tất cả OTP đã hết hạn (chạy định kỳ)
    static void cleanupExpiredOTP();
};