#include "SecurityUtils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "picosha2.h"
#include "../storage/OTPStorage.h"
#include <functional>  // For std::hash
// Removed OpenSSL includes for simple compilation

// Initialize static members
std::mt19937 SecurityUtils::rng;
// std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> SecurityUtils::otpStore;
const int SecurityUtils::OTP_VALIDITY_MINUTES;

void SecurityUtils::initialize() {
    // Initialize random seed
    std::random_device rd;
    rng.seed(rd());
}

std::string SecurityUtils::hashPassword(const std::string& password, const std::string& salt) {
    std::string actualSalt = salt.empty() ? generateSalt() : salt;
    
    // Combine password and salt
    std::string combined = password + actualSalt;
    
    // Calculate SHA256 hash
    std::string hash = sha256(combined);
    
    // Return format: salt$hash
    return actualSalt + "$" + hash;
}

bool SecurityUtils::verifyPassword(const std::string& password, const std::string& hashedPassword) {
    // Split salt and hash
    size_t dollarPos = hashedPassword.find('$');
    if (dollarPos == std::string::npos) {
        return false;
    }
    
    std::string salt = hashedPassword.substr(0, dollarPos);
    std::string storedHash = hashedPassword.substr(dollarPos + 1);
    
    // Hash password with stored salt
    std::string testHash = sha256(password + salt);
    
    return testHash == storedHash;
}

std::string SecurityUtils::generatePassword(int length, bool includeSpecialChars) {
    const std::string lowercase = "abcdefghijklmnopqrstuvwxyz";
    const std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digits = "0123456789";
    const std::string special = "!@#$%^&*";
    
    std::string charset = lowercase + uppercase + digits;
    if (includeSpecialChars) {
        charset += special;
    }
    
    std::string password;
    std::uniform_int_distribution<> dist(0, charset.length() - 1);
    
    for (int i = 0; i < length; ++i) {
        password += charset[dist(rng)];
    }
    
    return password;
}

std::string SecurityUtils::generateOTP(const std::string& userId, const std::string& purpose) {
    std::uniform_int_distribution<> dist(100000, 999999);
    std::string otp = std::to_string(dist(rng));
    OTPStorage::saveOTP(userId, purpose, otp, OTP_VALIDITY_MINUTES * 60);
    return otp;
}

bool SecurityUtils::verifyOTP(const std::string& userId, const std::string& otpCode, const std::string& purpose) {
    std::string correctOTP = OTPStorage::getOTP(userId, purpose);
    if (!correctOTP.empty() && correctOTP == otpCode) {
        OTPStorage::removeOTP(userId, purpose);
        return true;
    }
    return false;
}

void SecurityUtils::cleanupExpiredOTP() {
    OTPStorage::cleanupExpiredOTP();
}

std::string SecurityUtils::generateUUID() {
    // Simple UUID version 4
    std::uniform_int_distribution<> dist(0, 15);
    std::uniform_int_distribution<> dist2(8, 11);
    
    std::ostringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) {
        ss << dist(rng);
    }
    ss << "-";
    
    for (int i = 0; i < 4; i++) {
        ss << dist(rng);
    }
    ss << "-";
    
    ss << "4"; // Version 4
    for (int i = 0; i < 3; i++) {
        ss << dist(rng);
    }
    ss << "-";
    
    ss << std::hex << dist2(rng); // Variant bits
    for (int i = 0; i < 3; i++) {
        ss << dist(rng);
    }
    ss << "-";
    
    for (int i = 0; i < 12; i++) {
        ss << dist(rng);
    }
    
    return ss.str();
}

std::string SecurityUtils::generateSalt(int length) {
    const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string salt;
    std::uniform_int_distribution<> dist(0, charset.length() - 1);
    
    for (int i = 0; i < length; ++i) {
        salt += charset[dist(rng)];
    }
      return salt;
}

std::string SecurityUtils::generateRandomString(int length) {
    // Use the same implementation as generateSalt
    return generateSalt(length);
}

std::string SecurityUtils::encrypt(const std::string& data, const std::string& key) {
    std::string encrypted;
    
    for (size_t i = 0; i < data.length(); ++i) {
        char keyChar = key[i % key.length()];
        encrypted += data[i] ^ keyChar;
    }
      // Convert to hex
    std::ostringstream ss;
    for (unsigned char c : encrypted) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    
    return ss.str();
}

std::string SecurityUtils::decrypt(const std::string& encryptedData, const std::string& key) {
    // Convert hex to bytes
    std::vector<unsigned char> bytes = hexToBytes(encryptedData);
    
    std::string decrypted;
    for (size_t i = 0; i < bytes.size(); ++i) {
        char keyChar = key[i % key.length()];
        decrypted += bytes[i] ^ keyChar;
    }
    
    return decrypted;
}

std::string SecurityUtils::sha256(const std::string& input) {
    return picosha2::hash256_hex_string(input);
}

std::string SecurityUtils::bytesToHex(const unsigned char* bytes, size_t length) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    
    return ss.str();
}

std::vector<unsigned char> SecurityUtils::hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}
