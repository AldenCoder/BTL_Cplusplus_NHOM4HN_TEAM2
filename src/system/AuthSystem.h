/**
 * @file AuthSystem.h
 * @brief Authentication system and user management
 * @author Team 2C
 */

#ifndef AUTH_SYSTEM_H
#define AUTH_SYSTEM_H

#include "../models/User.h"
#include "../models/Wallet.h"
#include "../security/SecurityUtils.h"
#include "../security/OTPManager.h"
#include "../storage/DataManager.h"
#include <memory>
#include <unordered_map>
#include <string>

/**
 * @struct LoginResult
 * @brief Login result
 */
struct LoginResult {
    bool success;              // Login successful or not
    std::string message;       // Result message
    std::shared_ptr<User> user; // User information (if successful)
    bool requirePasswordChange; // Need to change password or not
};

/**
 * @struct RegistrationResult
 * @brief Registration result
 */
struct RegistrationResult {
    bool success;              // Registration successful or not
    std::string message;       // Result message
    std::string generatedPassword; // Auto-generated password (if any)
};

/**
 * @class AuthSystem
 * @brief Main authentication system
 * 
 * Manages entire process:
 * - New account registration
 * - Login/logout
 * - Session management
 * - Access authentication
 */
class AuthSystem {
private:
    std::shared_ptr<DataManager> dataManager;    // Data manager
    std::shared_ptr<OTPManager> otpManager;      // OTP manager
    std::shared_ptr<User> currentUser;           // Currently logged in user
    std::unordered_map<std::string, std::shared_ptr<User>> userCache; // User cache
    
    bool isInitialized;                          // System initialized or not

public:
    /**
     * @brief Constructor
     */
    AuthSystem();

    /**
     * @brief Destructor
     */
    ~AuthSystem();    /**
     * @brief Initialize system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Register new account (by user)
     * @param username Login username
     * @param password Password
     * @param fullName Full name
     * @param email Email
     * @param phoneNumber Phone number
     * @return Registration result
     */
    RegistrationResult registerUser(const std::string& username,
                                   const std::string& password,
                                   const std::string& fullName,
                                   const std::string& email,
                                   const std::string& phoneNumber);

    /**
     * @brief Create new account (by admin)
     * @param username Login username
     * @param fullName Full name
     * @param email Email
     * @param phoneNumber Phone number
     * @param role User role
     * @param autoGeneratePassword Auto-generate password or not
     * @return Account creation result
     */
    RegistrationResult createAccount(const std::string& username,
                                    const std::string& fullName,
                                    const std::string& email,
                                    const std::string& phoneNumber,
                                    UserRole role = UserRole::REGULAR,
                                    bool autoGeneratePassword = true);

    /**
     * @brief Login
     * @param username Login username
     * @param password Password
     * @return Login result
     */
    LoginResult login(const std::string& username, const std::string& password);

    /**
     * @brief Logout
     */
    void logout();    /**
     * @brief Change password
     * @param userId User ID
     * @param oldPassword Old password
     * @param newPassword New password
     * @return true if successful
     */
    bool changePassword(const std::string& userId,
                       const std::string& oldPassword,
                       const std::string& newPassword);

    /**
     * @brief Update personal information (with OTP)
     * @param userId User ID
     * @param newFullName New full name
     * @param newEmail New email
     * @param newPhoneNumber New phone number
     * @param otpCode OTP verification code
     * @return true if successful
     */
    bool updateProfile(const std::string& userId,
                      const std::string& newFullName,
                      const std::string& newEmail,
                      const std::string& newPhoneNumber,
                      const std::string& otpCode);

    /**
     * @brief Request OTP for profile update
     * @param userId User ID
     * @return OTP code (will be sent via email/SMS in real implementation)
     */
    std::string requestProfileUpdateOTP(const std::string& userId);

    /**
     * @brief Get current user information
     * @return Shared pointer to user
     */
    std::shared_ptr<User> getCurrentUser() const { return currentUser; }

    /**
     * @brief Check if user is currently logged in
     * @return true if user is logged in
     */
    bool isLoggedIn() const { return currentUser != nullptr; }

    /**
     * @brief Check if current user is admin
     * @return true if admin
     */
    bool isCurrentUserAdmin() const;    /**
     * @brief Get list of all users (admin only)
     * @return Vector containing user information
     */
    std::vector<std::shared_ptr<User>> getAllUsers();

    /**
     * @brief Get data manager
     * @return Shared pointer to data manager
     */
    std::shared_ptr<DataManager> getDataManager() const { return dataManager; }    /**
     * @brief Find user by username
     * @param username Login username
     * @return Shared pointer to user (nullptr if not found)
     */
    std::shared_ptr<User> findUserByUsername(const std::string& username);

    /**
     * @brief Find user by ID
     * @param userId User ID
     * @return Shared pointer to user (nullptr if not found)
     */
    std::shared_ptr<User> findUserById(const std::string& userId);

    /**
     * @brief Check if username exists
     * @param username Login username
     * @return true if already exists
     */
    bool isUsernameExists(const std::string& username);

    /**
     * @brief Save user data
     * @param user User to save
     * @return true if save successful
     */
    bool saveUser(std::shared_ptr<User> user);

    /**
     * @brief Create default admin account (if not exists)
     */
    void createDefaultAdmin();

private:
    /**
     * @brief Validate registration data
     * @param username Login username
     * @param email Email
     * @param phoneNumber Phone number
     * @return Error string (empty if valid)
     */
    std::string validateRegistrationData(const std::string& username,
                                        const std::string& email,
                                        const std::string& phoneNumber);

    /**
     * @brief Load user into cache
     * @param username Login username
     * @return Shared pointer to user
     */
    std::shared_ptr<User> loadUserToCache(const std::string& username);

    /**
     * @brief Remove user from cache
     * @param username Login username
     */
    void removeUserFromCache(const std::string& username);
};

#endif // AUTH_SYSTEM_H
