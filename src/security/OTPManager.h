/**
 * @file OTPManager.h
 * @brief Header file cho OTPManager class
 * @author Team 2C
 */

#ifndef OTPMANAGER_H
#define OTPMANAGER_H

#include <string>

// Forward declaration
class SecurityUtils;

/**
 * @enum OTPType
 * @brief Loại OTP
 */
enum class OTPType {
    PROFILE_UPDATE,  // OTP cho cập nhật thông tin cá nhân
    TRANSFER        // OTP cho giao dịch chuyển điểm
};

/**
 * @class OTPManager  
 * @brief Quản lý OTP cho các mục đích khác nhau
 */
class OTPManager {
private:
    // SecurityUtils* securityUtils;  // Commented out until needed

public:
    /**
     * @brief Constructor
     */
    OTPManager();

    /**
     * @brief Destructor
     */
    ~OTPManager() = default;

    /**
     * @brief Tạo OTP chung
     * @param userId ID người dùng
     * @param type Loại OTP
     * @return Mã OTP
     */
    std::string generateOTP(const std::string& userId, OTPType type);

    /**
     * @brief Xác thực OTP chung
     * @param userId ID người dùng
     * @param otpCode Mã OTP
     * @param type Loại OTP
     * @return true nếu hợp lệ
     */
    bool verifyOTP(const std::string& userId, 
                   const std::string& otpCode, 
                   OTPType type);

    /**
     * @brief Tạo OTP cho thay đổi thông tin cá nhân
     * @param userId ID người dùng
     * @return Mã OTP
     */
    std::string generateProfileUpdateOTP(const std::string& userId);

    /**
     * @brief Tạo OTP cho giao dịch chuyển điểm
     * @param userId ID người dùng
     * @param amount Số điểm
     * @param toWalletId ID ví đích
     * @return Mã OTP
     */
    std::string generateTransferOTP(const std::string& userId, 
                                   double amount, 
                                   const std::string& toWalletId);

    /**
     * @brief Xác thực OTP thay đổi thông tin
     * @param userId ID người dùng
     * @param otpCode Mã OTP
     * @return true nếu hợp lệ
     */
    bool verifyProfileUpdateOTP(const std::string& userId, 
                               const std::string& otpCode);

    /**
     * @brief Xác thực OTP giao dịch
     * @param userId ID người dùng
     * @param otpCode Mã OTP
     * @param toWalletId ID ví đích (để tạo purpose)
     * @return true nếu hợp lệ
     */
    bool verifyTransferOTP(const std::string& userId, 
                          const std::string& otpCode,
                          const std::string& toWalletId);

    /**
     * @brief Mô phỏng gửi OTP qua email/SMS
     * @param userId ID người dùng
     * @param otpCode Mã OTP
     * @param purpose Mục đích
     * @param contactInfo Thông tin liên lạc (email/phone)
     */
    void sendOTP(const std::string& userId, 
                const std::string& otpCode,
                const std::string& purpose,
                const std::string& contactInfo);
};

#endif // OTPMANAGER_H
