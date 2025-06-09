#ifndef AUTH_SYSTEM_H
#define AUTH_SYSTEM_H

#include "../models/User.h"
#include "../models/Wallet.h"
#include "../security/SecurityUtils.h"
#include "../security/OTPManager.h"  
#include "../storage/DatabaseManager.h"
#include "WalletManager.h"
#include <memory>
#include <unordered_map>
#include <string>

struct LoginResult {
    bool success;
    std::string message;
    std::shared_ptr<User> user;
    bool requirePasswordChange;
};

struct RegistrationResult {
    bool success;
    std::string message;
    std::string generatedPassword;
};

class AuthSystem {
private:
    std::shared_ptr<DatabaseManager> dataManager;
    std::shared_ptr<OTPManager> otpManager;
    std::shared_ptr<WalletManager> walletManager;
    std::shared_ptr<User> currentUser;
    std::unordered_map<std::string, std::shared_ptr<User>> userCache;
    
    bool isInitialized;

public:
    AuthSystem();
    ~AuthSystem();
    bool initialize();

    RegistrationResult registerUser(const std::string& username,
                                   const std::string& password,
                                   const std::string& fullName,
                                   const std::string& email,
                                   const std::string& phoneNumber);

    RegistrationResult createAccount(const std::string& username,
                                    const std::string& fullName,
                                    const std::string& email,
                                    const std::string& phoneNumber,
                                    UserRole role = UserRole::REGULAR,
                                    bool autoGeneratePassword = true);

    LoginResult login(const std::string& username, const std::string& password);
    void logout();
    
    bool changePassword(const std::string& userId,
                       const std::string& oldPassword,
                       const std::string& newPassword);

    bool updateProfile(const std::string& userId,
                      const std::string& newFullName,
                      const std::string& newEmail,
                      const std::string& newPhoneNumber,
                      const std::string& otpCode);

    std::string requestProfileUpdateOTP(const std::string& userId);
    std::string requestPasswordChangeOTP(const std::string& userId);

    bool changePasswordWithOTP(const std::string& userId,
                               const std::string& oldPassword,
                               const std::string& newPassword,
                               const std::string& otpCode);

    std::shared_ptr<User> getCurrentUser() const { return currentUser; }
    bool isLoggedIn() const { return currentUser != nullptr; }
    bool isCurrentUserAdmin() const;
    bool hasAnyAdmin() const;
    std::vector<std::shared_ptr<User>> getAllUsers();
    std::shared_ptr<DatabaseManager> getDataManager() const { return dataManager; }
    
    std::shared_ptr<User> findUserByUsername(const std::string& username);
    std::shared_ptr<User> findUserById(const std::string& userId);
    bool isUsernameExists(const std::string& username);
    bool saveUser(std::shared_ptr<User> user);

private:
    /**
     * @brief Load user into cache
     * @param username Login username
     * @return Shared pointer to user
     */
    std::shared_ptr<User> loadUserToCache(const std::string& username);
    void removeUserFromCache(const std::string& username);
};

#endif
