#include "AuthSystem.h"
#include "../security/OTPManager.h"
#include "WalletManager.h"
#include <iostream>
#include <algorithm>
#include <regex>

AuthSystem::AuthSystem() 
    : currentUser(nullptr), isInitialized(false) {
    dataManager = std::make_shared<DatabaseManager>();
    otpManager = std::make_shared<OTPManager>();
    walletManager = std::make_shared<WalletManager>(dataManager, otpManager);
}
AuthSystem::~AuthSystem() {
    if (isLoggedIn()) {
        logout();
    }
}

bool AuthSystem::initialize() {
    try {
        if (!dataManager->initialize()) {
            std::cerr << "Error: Cannot initialize DatabaseManager" << std::endl;
            return false;
        }

        if (!walletManager->initialize()) {
            std::cerr << "Error: Cannot initialize WalletManager" << std::endl;
            return false;
        }
        isInitialized = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing AuthSystem: " << e.what() << std::endl;
        return false;
    }
}

RegistrationResult AuthSystem::registerUser(const std::string& username,
                                           const std::string& password,
                                           const std::string& fullName,
                                           const std::string& email,
                                           const std::string& phoneNumber) {
    RegistrationResult result;
    result.success = false;
    try {
        std::string userId = SecurityUtils::generateUUID();
        UserRole userRole = UserRole::REGULAR;
        if (!hasAnyAdmin()) {
            userRole = UserRole::ADMIN;
            std::cout << "No admin users found. Creating first admin account..." << std::endl;
        }
        
        auto user = std::make_shared<User>(
            userId,
            username,
            SecurityUtils::hashPassword(password),
            fullName,
            email,
            phoneNumber,
            userRole
        );

        std::string walletId = SecurityUtils::generateUUID();
        user->setWalletId(walletId);
        user->setRequirePasswordChange(false); 

        // FIRST
        if (!dataManager->saveUser(user)) {
            result.message = "Error saving user to database!";
            return result;
        }
        
        userCache[username] = user;

        // SECOND
        if (!walletManager->createUserWallet(userId, walletId)) {
            dataManager->deleteUser(userId);
            userCache.erase(username);
            result.message = "Error creating user wallet!";
            return result;
        }
        
        result.success = true;
        if (userRole == UserRole::ADMIN) {
            result.message = "First admin account created successfully!";
        } else {
            result.message = "Account registered successfully!";
        }
    }
    catch (const std::exception& e) {
        result.message = "System error: " + std::string(e.what());
    }

    return result;
}
// For admin
RegistrationResult AuthSystem::createAccount(const std::string& username,
                                           const std::string& fullName,
                                           const std::string& email,
                                           const std::string& phoneNumber,
                                           UserRole role,
                                           bool autoGeneratePassword) {
    RegistrationResult result;
    result.success = false;    
    if (!isCurrentUserAdmin() && hasAnyAdmin()) {
        result.message = "No permission to create accounts!";
        return result;
    }

    try {        
        std::string password;
        if (autoGeneratePassword) {
            password = SecurityUtils::generateRandomString(12);
            result.generatedPassword = password;
        } else {
            password = "123456789"; 
        }
        
        std::string userId = SecurityUtils::generateUUID();
        
        auto user = std::make_shared<User>(
            userId,
            username,
            SecurityUtils::hashPassword(password),
            fullName,
            email,
            phoneNumber,
            role
        );
        
        user->setRequirePasswordChange(true);

        std::string walletId = SecurityUtils::generateUUID();
        user->setWalletId(walletId);

        // FIRST
        if (!dataManager->saveUser(user)) {
            result.message = "Error saving user data!";
            return result;
        }

        userCache[username] = user;

        // SECOND
        if (!walletManager->createUserWallet(userId, walletId)) {
            dataManager->deleteUser(userId);
            userCache.erase(username);
            result.message = "Error creating user wallet!";
            return result;
        }
        
        result.success = true;
        result.message = "Account created successfully!";
    }
    catch (const std::exception& e) {
        result.message = "System error: " + std::string(e.what());
    }

    return result;
}

LoginResult AuthSystem::login(const std::string& username, const std::string& password) {
    LoginResult result;
    result.success = false;    if (username.empty() || password.empty()) {
        result.message = "Username and password cannot be empty!";
        return result;
    }

    try {
        auto user = findUserByUsername(username);
        if (!user) {
            result.message = "Username does not exist!";
            return result;
        }

        if (!SecurityUtils::verifyPassword(password, user->getPasswordHash())) {
            result.message = "Password is incorrect!";
            return result;
        }

        if (!user->isActive()) {
            result.message = "Account is locked!";
            return result;
        }

        currentUser = user;
        user->updateLastLogin();
        result.success = true;
        result.user = user;
        result.requirePasswordChange = user->requirePasswordChange();
        result.message = "Login successful!";

        dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        result.message = "System error: " + std::string(e.what());
    }

    return result;
}

void AuthSystem::logout() {
    if (currentUser) {
        currentUser = nullptr;
    }
}

bool AuthSystem::changePassword(const std::string& userId,
                               const std::string& oldPassword,
                               const std::string& newPassword) {
    try {
        auto user = findUserById(userId);
        if (!user) {
            return false;
        }

        bool isAdminReset = isCurrentUserAdmin() && currentUser->getId() != userId;
        if (!isAdminReset) {
            if (!SecurityUtils::verifyPassword(oldPassword, user->getPasswordHash())) {
                return false;
            }
        }

        if (newPassword.length() < 8) {
            return false;
        }

        user->setPasswordHash(SecurityUtils::hashPassword(newPassword));
        user->setRequirePasswordChange(false);

        return dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        std::cerr << "Error changing password: " << e.what() << std::endl;
        return false;
    }
}

bool AuthSystem::updateProfile(const std::string& userId,
                              const std::string& newFullName,
                              const std::string& newEmail,
                              const std::string& newPhoneNumber,
                              const std::string& otpCode) {
    try {
        auto user = findUserById(userId);
        if (!user) {
            return false;
        }

        if (!otpManager->verifyOTP(userId, otpCode, OTPType::PROFILE_UPDATE)) {
            return false;
        }

        user->setFullName(newFullName);
        user->setEmail(newEmail);
        user->setPhoneNumber(newPhoneNumber);

        return dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating profile: " << e.what() << std::endl;
        return false;
    }
}

std::string AuthSystem::requestProfileUpdateOTP(const std::string& userId) {
    try {
        auto user = findUserById(userId);
        if (!user) {
            return "";
        }

        return otpManager->generateOTP(userId, OTPType::PROFILE_UPDATE);
    }
    catch (const std::exception& e) {
        std::cerr << "Error generating profile update OTP: " << e.what() << std::endl;
        return "";
    }
}

std::string AuthSystem::requestPasswordChangeOTP(const std::string& userId) {
    try {
        auto user = findUserById(userId);
        if (!user) {
            return "";
        }

        return otpManager->generatePasswordChangeOTP(userId);
    }
    catch (const std::exception& e) {
        std::cerr << "Error generating password change OTP: " << e.what() << std::endl;
        return "";
    }
}

bool AuthSystem::changePasswordWithOTP(const std::string& userId,
                                       const std::string& oldPassword,
                                       const std::string& newPassword,
                                       const std::string& otpCode) {
    try {
        auto user = findUserById(userId);
        if (!user) {
            return false;
        }

        if (!otpManager->verifyPasswordChangeOTP(userId, otpCode)) {
            return false;
        }

        bool isAdminReset = isCurrentUserAdmin() && currentUser->getId() != userId;
        if (!isAdminReset) {
            if (!SecurityUtils::verifyPassword(oldPassword, user->getPasswordHash())) {
                return false;
            }
        }

        if (newPassword.length() < 8) {
            return false;
        }

        user->setPasswordHash(SecurityUtils::hashPassword(newPassword));
        user->setRequirePasswordChange(false);

        return dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        std::cerr << "Error changing password with OTP: " << e.what() << std::endl;
        return false;
    }
}

bool AuthSystem::isCurrentUserAdmin() const {
    return currentUser && currentUser->getRole() == UserRole::ADMIN;
}

bool AuthSystem::hasAnyAdmin() const {
    try {
        auto users = dataManager->loadAllUsers();
        for (const auto& user : users) {
            if (user->getRole() == UserRole::ADMIN) {
                return true;
            }
        }
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error checking for admin users: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::shared_ptr<User>> AuthSystem::getAllUsers() {
    std::vector<std::shared_ptr<User>> users;
    
    if (!isCurrentUserAdmin()) {
        return users; 
    }
    
    try {
        users = dataManager->loadAllUsers();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading user list: " << e.what() << std::endl;
    }

    return users;
}

std::shared_ptr<User> AuthSystem::findUserByUsername(const std::string& username) {
    auto it = userCache.find(username);
    if (it != userCache.end()) {
        return it->second;
    }

    return loadUserToCache(username);
}

std::shared_ptr<User> AuthSystem::findUserById(const std::string& userId) {
    try {
        for (const auto& pair : userCache) {
            if (pair.second->getId() == userId) {
                return pair.second;
            }
        }     
        auto uniqueUser = dataManager->loadUserById(userId);
        if (uniqueUser) {
            return std::shared_ptr<User>(uniqueUser.release());
        }
        return nullptr;
    }
    catch (const std::exception& e) {
        std::cerr << "Error finding user: " << e.what() << std::endl;
        return nullptr;
    }
}

bool AuthSystem::isUsernameExists(const std::string& username) {
    return findUserByUsername(username) != nullptr;
}

bool AuthSystem::saveUser(std::shared_ptr<User> user) {
    try {
        bool success = dataManager->saveUser(user);
        if (success) {
            userCache[user->getUsername()] = user;
        }
        return success;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving user: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<User> AuthSystem::loadUserToCache(const std::string& username) {
    try {
        auto uniqueUser = dataManager->loadUserByUsername(username);
        if (uniqueUser) {
            auto sharedUser = std::shared_ptr<User>(uniqueUser.release());
            userCache[username] = sharedUser;
            return sharedUser;
        }
        return nullptr;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading user into cache: " << e.what() << std::endl;
        return nullptr;
    }
}

void AuthSystem::removeUserFromCache(const std::string& username) {
    userCache.erase(username);
}
