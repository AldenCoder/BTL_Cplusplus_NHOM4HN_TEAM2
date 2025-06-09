#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H
#include <string>
#include <random>
#include <chrono>
#include <unordered_map>
class SecurityUtils {
private:
    static std::mt19937 rng;
    static std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> otpStore;
    static const int OTP_VALIDITY_MINUTES = 5;
    static const int OTP_LENGTH = 6;

public:
    static void initialize();
    static std::string hashPassword(const std::string& password, 
                                   const std::string& salt = "");
    static bool verifyPassword(const std::string& password, 
                              const std::string& hashedPassword);

    static std::string generatePassword(int length = 12, 
                                       bool includeSpecialChars = true);
    static std::string generateOTP(const std::string& userId, 
                                  const std::string& purpose = "general");

    static bool verifyOTP(const std::string& userId, 
                         const std::string& otpCode,
                         const std::string& purpose = "general");
    static std::string generateUUID();
    static std::string generateSalt(int length = 16);

    static std::string generateRandomString(int length = 12);
    static std::string encrypt(const std::string& data, const std::string& key);
    static std::string decrypt(const std::string& encryptedData, const std::string& key);
    static void cleanupExpiredOTP();
    static std::string sha256(const std::string& input);

private:
    static std::string bytesToHex(const unsigned char* bytes, size_t length);
    static std::vector<unsigned char> hexToBytes(const std::string& hex);
};

#endif
