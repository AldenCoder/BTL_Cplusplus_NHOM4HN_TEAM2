#ifndef OTPMANAGER_H
#define OTPMANAGER_H

#include <string>

class SecurityUtils;

enum class OTPType {
    PROFILE_UPDATE,
    TRANSFER,
    PASSWORD_CHANGE
};

class OTPManager {
private:

public:
    OTPManager();
    ~OTPManager() = default;

    std::string generateOTP(const std::string& userId, OTPType type);

    bool verifyOTP(const std::string& userId, 
                   const std::string& otpCode, 
                   OTPType type);

    std::string generateProfileUpdateOTP(const std::string& userId);

    std::string generateTransferOTP(const std::string& userId, 
                                   double amount, 
                                   const std::string& toWalletId);

    std::string generatePasswordChangeOTP(const std::string& userId);

    bool verifyProfileUpdateOTP(const std::string& userId, 
                               const std::string& otpCode);

    bool verifyTransferOTP(const std::string& userId, 
                          const std::string& otpCode,
                          const std::string& toWalletId);

    bool verifyPasswordChangeOTP(const std::string& userId, 
                                const std::string& otpCode);

    void sendOTP(const std::string& userId, 
                const std::string& otpCode,
                const std::string& purpose,
                const std::string& contactInfo);
};

#endif
