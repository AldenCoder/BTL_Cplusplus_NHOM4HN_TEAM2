#include "AuthSystem.h"
#include "../security/OTPManager.h"
#include "WalletManager.h"
#include <iostream>
#include <algorithm>
#include <regex>

AuthSystem::AuthSystem() 
    : currentUser(nullptr), isInitialized(false) {
    // Initialize components
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

        // Note: No longer auto-creating default admin
        // Database starts empty - admin must be created manually
        
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

    // Kiểm tra đầu vào
    std::string validationError = validateRegistrationData(username, email, phoneNumber);
    if (!validationError.empty()) {
        result.message = validationError;
        return result;
    }    // Check if username exists
    if (isUsernameExists(username)) {
        result.message = "Username already exists!";
        return result;
    }

    // Check password
    if (password.length() < 8) {
        result.message = "Password must be at least 8 characters!";
        return result;
    }    try {
        // Generate a unique user ID
        std::string userId = SecurityUtils::generateUUID();
        
        // Determine user role: if no admin exists, make this user an admin
        UserRole userRole = UserRole::REGULAR;
        if (!hasAnyAdmin()) {
            userRole = UserRole::ADMIN;
            std::cout << "No admin users found. Creating first admin account..." << std::endl;
        }
        
        // Create new user with this ID
        auto user = std::make_shared<User>(
            userId,
            username,
            SecurityUtils::hashPassword(password),
            fullName,
            email,
            phoneNumber,
            userRole
        );

        // Generate wallet ID
        std::string walletId = SecurityUtils::generateUUID();
        user->setWalletId(walletId);

        // FIRST: Save user to database (required for foreign key constraint)
        if (!dataManager->saveUser(user)) {
            result.message = "Error saving user to database!";
            return result;
        }
        
        // Add to cache
        userCache[username] = user;

        // SECOND: Create wallet using WalletManager (now that user exists in DB)
        if (!walletManager->createUserWallet(userId, walletId)) {
            // If wallet creation fails, we should remove the user from database
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

RegistrationResult AuthSystem::createAccount(const std::string& username,
                                           const std::string& fullName,
                                           const std::string& email,
                                           const std::string& phoneNumber,
                                           UserRole role,
                                           bool autoGeneratePassword) {
    RegistrationResult result;
    result.success = false;    // Only admin can create accounts, unless no admin exists yet
    if (!isCurrentUserAdmin() && hasAnyAdmin()) {
        result.message = "No permission to create accounts!";
        return result;
    }

    // Validate input
    std::string validationError = validateRegistrationData(username, email, phoneNumber);
    if (!validationError.empty()) {
        result.message = validationError;
        return result;
    }

    // Check if username exists
    if (isUsernameExists(username)) {
        result.message = "Username already exists!";
        return result;
    }

    try {        std::string password;
        if (autoGeneratePassword) {
            password = SecurityUtils::generateRandomString(12);
            result.generatedPassword = password;
        } else {
            password = "123456789"; // Default password
        }
        
        // Generate a unique user ID
        std::string userId = SecurityUtils::generateUUID();
        
        // Tạo user mới
        auto user = std::make_shared<User>(
            userId,
            username,
            SecurityUtils::hashPassword(password),
            fullName,
            email,
            phoneNumber,
            role
        );
        
        // Đánh dấu cần đổi mật khẩu
        user->setRequirePasswordChange(true);

        // Generate wallet ID
        std::string walletId = SecurityUtils::generateUUID();
        user->setWalletId(walletId);

        // FIRST: Save user to database (required for foreign key constraint)
        if (!dataManager->saveUser(user)) {
            result.message = "Error saving user data!";
            return result;
        }
        
        // Add to cache
        userCache[username] = user;

        // SECOND: Create wallet using WalletManager (now that user exists in DB)
        if (!walletManager->createUserWallet(userId, walletId)) {
            // If wallet creation fails, we should remove the user from database
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
        // Find user
        auto user = findUserByUsername(username);
        if (!user) {
            result.message = "Username does not exist!";
            return result;
        }

        // Check password
        if (!SecurityUtils::verifyPassword(password, user->getPasswordHash())) {
            result.message = "Password is incorrect!";
            return result;
        }

        // Check account status
        if (!user->isActive()) {
            result.message = "Account is locked!";
            return result;
        }

        // Đăng nhập thành công
        currentUser = user;
        user->updateLastLogin();
        result.success = true;
        result.user = user;
        result.requirePasswordChange = user->requirePasswordChange();
        result.message = "Login successful!";

        // Save last login information
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

        // Kiểm tra mật khẩu cũ (trừ trường hợp admin reset)
        bool isAdminReset = isCurrentUserAdmin() && currentUser->getId() != userId;
        if (!isAdminReset) {
            if (!SecurityUtils::verifyPassword(oldPassword, user->getPasswordHash())) {
                return false;
            }
        }

        // Kiểm tra mật khẩu mới
        if (newPassword.length() < 8) {
            return false;
        }

        // Cập nhật mật khẩu
        user->setPasswordHash(SecurityUtils::hashPassword(newPassword));
        user->setRequirePasswordChange(false);

        // Lưu thay đổi
        return dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        std::cerr << "Loi doi mat khau: " << e.what() << std::endl;
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

        // Xác thực OTP
        if (!otpManager->verifyOTP(userId, otpCode, OTPType::PROFILE_UPDATE)) {
            return false;
        }

        // Cập nhật thông tin
        user->setFullName(newFullName);
        user->setEmail(newEmail);
        user->setPhoneNumber(newPhoneNumber);

        // Lưu thay đổi
        return dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        std::cerr << "Loi cap nhat thong tin: " << e.what() << std::endl;
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
        std::cerr << "Loi tao OTP: " << e.what() << std::endl;
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
        std::cerr << "Loi tao OTP cho doi mat khau: " << e.what() << std::endl;
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

        // Verify OTP first
        if (!otpManager->verifyPasswordChangeOTP(userId, otpCode)) {
            return false;
        }

        // Check old password (except for admin reset)
        bool isAdminReset = isCurrentUserAdmin() && currentUser->getId() != userId;
        if (!isAdminReset) {
            if (!SecurityUtils::verifyPassword(oldPassword, user->getPasswordHash())) {
                return false;
            }
        }

        // Check new password length
        if (newPassword.length() < 8) {
            return false;
        }

        // Update password
        user->setPasswordHash(SecurityUtils::hashPassword(newPassword));
        user->setRequirePasswordChange(false);

        // Save changes
        return dataManager->saveUser(user);
    }
    catch (const std::exception& e) {
        std::cerr << "Loi doi mat khau voi OTP: " << e.what() << std::endl;
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
        return users; // Chỉ admin mới được xem danh sách
    }
    
    try {
        users = dataManager->loadAllUsers();
    }
    catch (const std::exception& e) {
        std::cerr << "Loi tai danh sach user: " << e.what() << std::endl;
    }

    return users;
}

std::shared_ptr<User> AuthSystem::findUserByUsername(const std::string& username) {
    // Kiểm tra cache trước
    auto it = userCache.find(username);
    if (it != userCache.end()) {
        return it->second;
    }

    // Tải từ storage
    return loadUserToCache(username);
}

std::shared_ptr<User> AuthSystem::findUserById(const std::string& userId) {
    try {
        // Tìm trong cache trước
        for (const auto& pair : userCache) {
            if (pair.second->getId() == userId) {
                return pair.second;
            }
        }        // Tải từ storage
        auto uniqueUser = dataManager->loadUserById(userId);
        if (uniqueUser) {
            return std::shared_ptr<User>(uniqueUser.release());
        }
        return nullptr;
    }
    catch (const std::exception& e) {
        std::cerr << "Loi tim user: " << e.what() << std::endl;
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
            // Cập nhật cache
            userCache[user->getUsername()] = user;
        }
        return success;
    }
    catch (const std::exception& e) {
        std::cerr << "Loi luu user: " << e.what() << std::endl;
        return false;
    }
}

void AuthSystem::createDefaultAdmin() {
    try {
        // Kiểm tra đã có admin chưa
        auto users = dataManager->loadAllUsers();
        for (const auto& user : users) {
            if (user->getRole() == UserRole::ADMIN) {
                return; // Đã có admin
            }
        }        // Generate unique user ID for admin
        std::string adminId = SecurityUtils::generateUUID();
        
        // Tạo admin mặc định
        auto admin = std::make_shared<User>(
            adminId,
            "admin",
            SecurityUtils::hashPassword("admin123"),
            "Quan tri vien he thong",
            "admin@system.com",
            "0000000000",
            UserRole::ADMIN        
        );

        // Tạo ví cho admin with unique wallet ID
        std::string walletId = SecurityUtils::generateUUID();
        auto wallet = std::make_shared<Wallet>(walletId, adminId, 1000000.0); // 1 triệu điểm ban đầu
        admin->setWalletId(walletId);
        
        // Save wallet first, then admin
        dataManager->saveWallet(wallet);
        dataManager->saveUser(admin);
        userCache["admin"] = admin;

        std::cout << "Default admin account created:" << std::endl;
        std::cout << "Username: admin" << std::endl;
        std::cout << "Password: admin123" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating default admin: " << e.what() << std::endl;
    }
}

std::string AuthSystem::validateRegistrationData(const std::string& username,
                                                const std::string& email,
                                                const std::string& phoneNumber) {
    // Kiểm tra username
    if (username.length() < 3 || username.length() > 20) {
        return "Username must be 3-20 characters long!";
    }

    std::regex usernameRegex("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(username, usernameRegex)) {
        return "Username can only contain letters, numbers, and underscores!";
    }

    // Check email
    std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!std::regex_match(email, emailRegex)) {
        return "Invalid email format!";
    }

    // Check phone number
    std::regex phoneRegex(R"(^[0-9]{10,11}$)");
    if (!std::regex_match(phoneNumber, phoneRegex)) {
        return "Phone number must be 10-11 digits!";
    }

    return ""; // Hợp lệ
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
        std::cerr << "Loi tai user vao cache: " << e.what() << std::endl;
        return nullptr;
    }
}

void AuthSystem::removeUserFromCache(const std::string& username) {
    userCache.erase(username);
}
