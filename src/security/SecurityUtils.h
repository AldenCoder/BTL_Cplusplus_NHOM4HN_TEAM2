/**
 * @file SecurityUtils.h
 * @brief Các tiện ích bảo mật - băm mật khẩu, tạo OTP, mã hóa
 * @author Team 2C
 */

#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include <string>
#include <random>
#include <chrono>
#include <unordered_map>

/**
 * @class SecurityUtils
 * @brief Lớp tiện ích bảo mật
 * 
 * Cung cấp các chức năng:
 * - Băm mật khẩu bằng SHA256 + salt
 * - Tạo mật khẩu tự động
 * - Tạo và xác thực OTP
 * - Tạo UUID
 */
class SecurityUtils {
private:
    static std::mt19937 rng;  // Random number generator
    
    // Lưu trữ OTP tạm thời (trong thực tế nên dùng database)
    static std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> otpStore;
    
    static const int OTP_VALIDITY_MINUTES = 5;  // OTP có hiệu lực 5 phút
    static const int OTP_LENGTH = 6;            // Độ dài OTP

public:
    /**
     * @brief Khởi tạo random seed
     */
    static void initialize();

    /**
     * @brief Băm mật khẩu với salt
     * @param password Mật khẩu gốc
     * @param salt Salt (nếu rỗng sẽ tự tạo)
     * @return Chuỗi hash có format: salt$hash
     */
    static std::string hashPassword(const std::string& password, 
                                   const std::string& salt = "");

    /**
     * @brief Xác thực mật khẩu
     * @param password Mật khẩu cần kiểm tra
     * @param hashedPassword Mật khẩu đã băm (format: salt$hash)
     * @return true nếu mật khẩu đúng
     */
    static bool verifyPassword(const std::string& password, 
                              const std::string& hashedPassword);

    /**
     * @brief Tạo mật khẩu tự động
     * @param length Độ dài mật khẩu (mặc định 12)
     * @param includeSpecialChars Có bao gồm ký tự đặc biệt không
     * @return Mật khẩu được tạo
     */
    static std::string generatePassword(int length = 12, 
                                       bool includeSpecialChars = true);

    /**
     * @brief Tạo OTP cho user
     * @param userId ID người dùng
     * @param purpose Mục đích sử dụng OTP
     * @return Mã OTP 6 chữ số
     */
    static std::string generateOTP(const std::string& userId, 
                                  const std::string& purpose = "general");

    /**
     * @brief Xác thực OTP
     * @param userId ID người dùng
     * @param otpCode Mã OTP cần xác thực
     * @param purpose Mục đích sử dụng (phải trùng với lúc tạo)
     * @return true nếu OTP hợp lệ
     */
    static bool verifyOTP(const std::string& userId, 
                         const std::string& otpCode,
                         const std::string& purpose = "general");

    /**
     * @brief Tạo UUID version 4
     * @return Chuỗi UUID
     */
    static std::string generateUUID();    /**
     * @brief Tạo salt ngẫu nhiên
     * @param length Độ dài salt
     * @return Chuỗi salt
     */
    static std::string generateSalt(int length = 16);

    /**
     * @brief Tạo chuỗi ngẫu nhiên
     * @param length Độ dài chuỗi
     * @return Chuỗi ngẫu nhiên
     */
    static std::string generateRandomString(int length = 12);

    /**
     * @brief Mã hóa dữ liệu đơn giản (XOR cipher)
     * @param data Dữ liệu cần mã hóa
     * @param key Khóa mã hóa
     * @return Dữ liệu đã mã hóa (hex string)
     */
    static std::string encrypt(const std::string& data, const std::string& key);

    /**
     * @brief Giải mã dữ liệu
     * @param encryptedData Dữ liệu đã mã hóa (hex string)
     * @param key Khóa giải mã
     * @return Dữ liệu gốc
     */
    static std::string decrypt(const std::string& encryptedData, const std::string& key);    /**
     * @brief Dọn dẹp OTP hết hạn
     */
    static void cleanupExpiredOTP();

    /**
     * @brief Tính hash SHA256
     * @param input Chuỗi đầu vào
     * @return Hash hex string
     */
    static std::string sha256(const std::string& input);

private:

    /**
     * @brief Chuyển đổi bytes thành hex string
     * @param bytes Mảng bytes
     * @param length Độ dài mảng
     * @return Hex string
     */
    static std::string bytesToHex(const unsigned char* bytes, size_t length);

    /**
     * @brief Chuyển hex string thành bytes
     * @param hex Hex string     * @return Vector bytes     */
    static std::vector<unsigned char> hexToBytes(const std::string& hex);
};

#endif // SECURITY_UTILS_H
