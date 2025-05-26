/**
 * @file OTPManager.cpp
 * @brief Implementation of OTPManager
 * @author Team 2C
 */

#include "OTPManager.h"
#include "SecurityUtils.h"
#include <iostream>
#include <sstream>

/**
 * @brief Constructor
 */
OTPManager::OTPManager() : securityUtils(nullptr) {
    SecurityUtils::initialize();
}

/**
 * @brief Generate general OTP
 * @param userId User ID
 * @param type OTP type
 * @return OTP code
 */
std::string OTPManager::generateOTP(const std::string& userId, OTPType type) {
    std::string purpose;
    switch (type) {
        case OTPType::PROFILE_UPDATE:
            purpose = "profile_update";
            break;
        case OTPType::TRANSFER:
            purpose = "transfer";
            break;
        default:
            purpose = "general";
            break;
    }
    
    std::string otp = SecurityUtils::generateOTP(userId, purpose);
      // Simulate sending OTP
    std::cout << "\n=== OTP CODE GENERATED ===\n";
    std::cout << "OTP Code: " << otp << "\n";
    std::cout << "Purpose: " << purpose << "\n";
    std::cout << "User ID: " << userId << "\n";
    std::cout << "This code is valid for 5 minutes.\n";
    std::cout << "=========================\n\n";
    
    return otp;
}

/**
 * @brief Xác thực OTP chung
 * @param userId ID người dùng
 * @param otpCode Mã OTP
 * @param type Loại OTP
 * @return true nếu hợp lệ
 */
bool OTPManager::verifyOTP(const std::string& userId, 
                          const std::string& otpCode, 
                          OTPType type) {
    std::string purpose;
    switch (type) {
        case OTPType::PROFILE_UPDATE:
            purpose = "profile_update";
            break;
        case OTPType::TRANSFER:
            purpose = "transfer";
            break;
        default:
            purpose = "general";
            break;
    }
    
    return SecurityUtils::verifyOTP(userId, otpCode, purpose);
}

/**
 * @brief Tạo OTP cho thay đổi thông tin cá nhân
 * @param userId ID người dùng
 * @return Mã OTP
 */
std::string OTPManager::generateProfileUpdateOTP(const std::string& userId) {
    std::string otp = SecurityUtils::generateOTP(userId, "profile_update");
      // Simulate sending OTP (in reality would be sent via email/SMS)
    std::cout << "\n=== OTP CODE SENT ===\n";
    std::cout << "OTP code for information update: " << otp << "\n";
    std::cout << "This code is valid for 5 minutes.\n";
    std::cout << "========================\n\n";
    
    return otp;
}

/**
 * @brief Tạo OTP cho giao dịch chuyển điểm
 * @param userId ID người dùng
 * @param amount Số điểm
 * @param toWalletId ID ví đích
 * @return Mã OTP
 */
std::string OTPManager::generateTransferOTP(const std::string& userId,
                                           double amount, 
                                           const std::string& toWalletId) {
    std::string purpose = "transfer_" + toWalletId;
    std::string otp = SecurityUtils::generateOTP(userId, purpose);
      // Simulate sending OTP
    std::cout << "\n=== OTP FOR TRANSACTION ===\n";
    std::cout << "Transaction: Transfer " << amount << " points to wallet " << toWalletId << "\n";
    std::cout << "OTP verification code: " << otp << "\n";
    std::cout << "This code is valid for 5 minutes.\n";
    std::cout << "===========================\n\n";
    
    return otp;
}

/**
 * @brief Xác thực OTP thay đổi thông tin
 * @param userId ID người dùng
 * @param otpCode Mã OTP
 * @return true nếu hợp lệ
 */
bool OTPManager::verifyProfileUpdateOTP(const std::string& userId,
                                       const std::string& otpCode) {
    return SecurityUtils::verifyOTP(userId, otpCode, "profile_update");
}

/**
 * @brief Xác thực OTP giao dịch
 * @param userId ID người dùng
 * @param otpCode Mã OTP
 * @param toWalletId ID ví đích (để tạo purpose)
 * @return true nếu hợp lệ
 */
bool OTPManager::verifyTransferOTP(const std::string& userId,
                                  const std::string& otpCode,
                                  const std::string& toWalletId) {
    std::string purpose = "transfer_" + toWalletId;
    return SecurityUtils::verifyOTP(userId, otpCode, purpose);
}

/**
 * @brief Mô phỏng gửi OTP qua email/SMS
 * @param userId ID người dùng
 * @param otpCode Mã OTP
 * @param purpose Mục đích
 * @param contactInfo Thông tin liên lạc (email/phone)
 */
void OTPManager::sendOTP(const std::string& userId,
                        const std::string& otpCode,
                        const std::string& purpose,
                        const std::string& contactInfo) {
      std::cout << "\n=== OTP SEND NOTIFICATION ===\n";
    std::cout << "OTP code sent to: " << contactInfo << "\n";
    std::cout << "Purpose: " << purpose << "\n";
    std::cout << "User ID: " << userId << "\n";
    std::cout << "------------------------\n";
    std::cout << "In reality, OTP code would be sent via:\n";
    std::cout << "- Email: " << contactInfo << "\n";
    std::cout << "- SMS: " << contactInfo << "\n";
    std::cout << "========================\n\n";
}
