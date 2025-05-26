/**
 * @file User.h
 * @brief User class definition - user account information management
 * @author Team 2C
 */

#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

/**
 * @enum UserRole
 * @brief User role classification in the system
 */
enum class UserRole {
    REGULAR,    // Regular user
    ADMIN       // Administrator
};

/**
 * @class User
 * @brief User account information management class
 * 
 * Contains all necessary information of a user in the system:
 * - Personal information (ID, name, email, phone number)
 * - Login information (username, password hash)
 * - Security information (role, password status)
 * - Wallet information
 */
class User {
private:
    std::string userId;           // Unique user ID (UUID)
    std::string username;         // Login username (immutable)
    std::string passwordHash;     // Hashed password
    std::string fullName;         // Full name
    std::string email;           // Contact email
    std::string phoneNumber;     // Phone number
    UserRole role;               // Role in system
    bool isPasswordGenerated;    // Is password auto-generated
    bool isFirstLogin;          // First time login
    std::string walletId;       // Wallet ID
    std::chrono::system_clock::time_point createdAt;  // Account creation time
    std::chrono::system_clock::time_point lastLogin;  // Last login time

public:
    /**
     * @brief Default constructor (for JSON parsing)
     */
    User() : role(UserRole::REGULAR), isPasswordGenerated(false), isFirstLogin(true),
             createdAt(std::chrono::system_clock::now()), lastLogin(std::chrono::system_clock::now()) {}

    /**
     * @brief Constructor to create new user
     * @param id User ID (UUID)
     * @param username Login username
     * @param passwordHash Hashed password
     * @param fullName Full name
     * @param email Email
     * @param phoneNumber Phone number
     * @param role User role
     */
    User(const std::string& id, const std::string& username, const std::string& passwordHash,
         const std::string& fullName, const std::string& email,
         const std::string& phoneNumber, UserRole role = UserRole::REGULAR);// Getter methods
    const std::string& getId() const { return userId; }
    const std::string& getUserId() const { return userId; }
    const std::string& getUsername() const { return username; }
    const std::string& getPasswordHash() const { return passwordHash; }
    const std::string& getFullName() const { return fullName; }
    const std::string& getEmail() const { return email; }
    const std::string& getPhoneNumber() const { return phoneNumber; }    UserRole getRole() const { return role; }
    bool getIsPasswordGenerated() const { return isPasswordGenerated; }
    bool getIsFirstLogin() const { return isFirstLogin; }
    const std::string& getWalletId() const { return walletId; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return createdAt; }
    const std::chrono::system_clock::time_point& getLastLogin() const { return lastLogin; }    // Additional getter methods
    bool requirePasswordChange() const { return isPasswordGenerated || isFirstLogin; }
    bool isActive() const { return true; } // Placeholder - can be extended with account lock status

    // Setter methods (only allow changing certain fields)
    void setFullName(const std::string& name) { fullName = name; }
    void setEmail(const std::string& newEmail) { email = newEmail; }
    void setPhoneNumber(const std::string& phone) { phoneNumber = phone; }
    void setPasswordHash(const std::string& hash) { passwordHash = hash; }
    void setRequirePasswordChange(bool require) { isPasswordGenerated = require; isFirstLogin = require; }
    void setIsPasswordGenerated(bool generated) { isPasswordGenerated = generated; }
    void setIsFirstLogin(bool firstLogin) { isFirstLogin = firstLogin; }
    void updateLastLogin() { lastLogin = std::chrono::system_clock::now(); }

    // Wallet management (placeholder)
    void setWallet(std::shared_ptr<class Wallet> wallet) { /* Implementation if needed */ }

    /**
     * @brief Check if password is correct
     * @param password Password to check
     * @return true if password is correct
     */
    bool verifyPassword(const std::string& password) const;

    /**
     * @brief Change password
     * @param newPassword New password
     */
    void changePassword(const std::string& newPassword);

    /**
     * @brief Convert User object to JSON string for file storage
     * @return JSON string
     */
    std::string toJson() const;

    /**
     * @brief Create User object from JSON string
     * @param json JSON string
     * @return Unique pointer to User object
     */
    static std::unique_ptr<User> fromJson(const std::string& json);

private:
    /**
     * @brief Generate unique ID for user
     * @return UUID string
     */
    std::string generateUserId();

    /**
     * @brief Generate wallet ID for user
     * @return Wallet ID string
     */
    std::string generateWalletId();
};

#endif // USER_H
