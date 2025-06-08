#include "User.h"
#include "Wallet.h" 
#include "../security/SecurityUtils.h"
#include <sstream>
#include <iomanip>
#include <iostream>

// Khởi tạo người dùng mặc định
User::User(const std::string& id, const std::string& username, const std::string& passwordHash,
           const std::string& fullName, const std::string& email,
           const std::string& phoneNumber, UserRole role)
    : userId(id), username(username), passwordHash(passwordHash), fullName(fullName), 
      email(email), phoneNumber(phoneNumber), role(role), isPasswordGenerated(false),
      isFirstLogin(true), createdAt(std::chrono::system_clock::now()),
      lastLogin(std::chrono::system_clock::now()) {
    
    walletId = "";
}

// Kiểm tra mật khẩu
bool User::verifyPassword(const std::string& password) const {
    return SecurityUtils::verifyPassword(password, passwordHash);
}

void User::changePassword(const std::string& newPassword) {
    passwordHash = SecurityUtils::hashPassword(newPassword);
    isPasswordGenerated = false;
    isFirstLogin = false;
}

//Chuyển đổi đối tượng User thành chuỗi JSON
std::string User::toJson() const {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"userId\": \"" << userId << "\",\n";
    json << "  \"username\": \"" << username << "\",\n";
    json << "  \"passwordHash\": \"" << passwordHash << "\",\n";
    json << "  \"fullName\": \"" << fullName << "\",\n";
    json << "  \"email\": \"" << email << "\",\n";
    json << "  \"phoneNumber\": \"" << phoneNumber << "\",\n";
    json << "  \"role\": " << static_cast<int>(role) << ",\n";
    json << "  \"isPasswordGenerated\": " << (isPasswordGenerated ? "true" : "false") << ",\n";
    json << "  \"isFirstLogin\": " << (isFirstLogin ? "true" : "false") << ",\n";    json << "  \"walletId\": \"" << walletId << "\",\n";
    
    // Chuyển đổi thời gian thành giây 
    auto createdTime = std::chrono::duration_cast<std::chrono::seconds>(
        createdAt.time_since_epoch()).count();
    auto lastLoginTime = std::chrono::duration_cast<std::chrono::seconds>(
        lastLogin.time_since_epoch()).count();
        
    json << "  \"createdAt\": " << createdTime << ",\n";
    json << "  \"lastLogin\": " << lastLoginTime << "\n";
    json << "}";
    
    return json.str();
}

// Tạo đối tượng User từ chuỗi JSON
std::unique_ptr<User> User::fromJson(const std::string& json) {
    try {
        // Tìm kiếm key trong chuỗi JSON
        auto user = std::unique_ptr<User>(new User());
        
        auto extractString = [&json](const std::string& key) -> std::string {
            std::string searchKey = "\"" + key + "\": \"";
            size_t start = json.find(searchKey);
            if (start == std::string::npos) return "";
            start += searchKey.length();
            size_t end = json.find("\"", start);
            if (end == std::string::npos) return "";
            return json.substr(start, end - start);        };
        
        auto extractValue = [&json](const std::string& key) -> std::string {
            std::string searchKey = "\"" + key + "\": ";
            size_t start = json.find(searchKey);
            if (start == std::string::npos) return "";
            start += searchKey.length();
            size_t end = json.find_first_of(",}\n", start);
            if (end == std::string::npos) return "";
            return json.substr(start, end - start);        };
        
        // Trích xuất và gán giá trị cho các trường của đối tượng User
        // Extract fields
        user->userId = extractString("userId");
        user->username = extractString("username");
        user->passwordHash = extractString("passwordHash");
        user->fullName = extractString("fullName");
        user->email = extractString("email");
        user->phoneNumber = extractString("phoneNumber");
        user->walletId = extractString("walletId");
        
        // Extract role
        std::string roleStr = extractValue("role");
        if (!roleStr.empty()) {
            user->role = static_cast<UserRole>(std::stoi(roleStr));
        }
        
        // Extract boolean values
        std::string isPasswordGeneratedStr = extractValue("isPasswordGenerated");
        user->isPasswordGenerated = (isPasswordGeneratedStr == "true");
        
        std::string isFirstLoginStr = extractValue("isFirstLogin");
        user->isFirstLogin = (isFirstLoginStr == "true");
        
        // Extract timestamps
        std::string createdAtStr = extractValue("createdAt");
        if (!createdAtStr.empty()) {
            auto createdTime = std::chrono::seconds(std::stoll(createdAtStr));
            user->createdAt = std::chrono::system_clock::time_point(createdTime);
        }
        
        std::string lastLoginStr = extractValue("lastLogin");
        if (!lastLoginStr.empty()) {
            auto lastLoginTime = std::chrono::seconds(std::stoll(lastLoginStr));
            user->lastLogin = std::chrono::system_clock::time_point(lastLoginTime);
        }
          return user;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing User JSON: " << e.what() << std::endl;
        return nullptr;
    }
}

// Sinh ID người dùng
std::string User::generateUserId() {
    return SecurityUtils::generateUUID();
}
// Sinh ID ví người dùng
std::string User::generateWalletId() {
    return SecurityUtils::generateUUID();
}

// Gán ID ví cho người dùng
void User::setWallet(std::shared_ptr<class Wallet> wallet) {
    if (wallet) {
        walletId = wallet->getWalletId();
    }
}
