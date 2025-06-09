#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

// Khai báo enum cho vai trò người dùng
enum class UserRole {
    REGULAR,
    ADMIN
};

// Khai báo class User
class User {
private:
    std::string userId;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string email;
    std::string phoneNumber;
    UserRole role;
    bool isPasswordGenerated;
    bool isFirstLogin;
    std::string walletId;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point lastLogin;

public:
    User() : role(UserRole::REGULAR), isPasswordGenerated(false), isFirstLogin(true),
            createdAt(std::chrono::system_clock::now()), lastLogin(std::chrono::system_clock::now()) {}

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
    void setIsPasswordGenerated(bool generated) { isPasswordGenerated = generated; }    void setIsFirstLogin(bool firstLogin) { isFirstLogin = firstLogin; }
    void updateLastLogin() { lastLogin = std::chrono::system_clock::now(); }
    
    // Wallet management
    void setWalletId(const std::string& id) { walletId = id; }
    
    // Forward declaration can't access Wallet methods, implementation in .cpp
    void setWallet(std::shared_ptr<class Wallet> wallet);
    bool verifyPassword(const std::string& password) const;
    void changePassword(const std::string& newPassword);
    std::string toJson() const;
    static std::unique_ptr<User> fromJson(const std::string& json);

private:
    std::string generateUserId();
    std::string generateWalletId();
};

#endif
