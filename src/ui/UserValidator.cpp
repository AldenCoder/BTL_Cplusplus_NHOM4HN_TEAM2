// UserValidator.cpp
// Tách các hàm kiểm tra hợp lệ user ra file riêng
#include "UserValidator.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include "../security/SecurityUtils.h"

bool UserValidator::isValidUsername(const std::string& username) {
    if (username.empty() || username.length() > 20) return false;
    std::regex usernameRegex(R"(^[a-zA-Z0-9_]{3,20}$)");
    if (!std::regex_match(username, usernameRegex)) return false;
    if (username.find("..") != std::string::npos) return false;
    if (username[0] == '.' || username[username.length()-1] == '.') return false;
    return true;
}

bool UserValidator::isValidFullName(const std::string& fullName) {
    if (fullName.empty() || fullName.length() > 30) return false;
    std::regex fullNameRegex(R"(^[a-zA-Z_]{0,30}$)");
    if (!std::regex_match(fullName, fullNameRegex)) return false;
    if (fullName.find("..") != std::string::npos) return false;
    if (fullName[0] == '.' || fullName[fullName.length()-1] == '.') return false;
    return true;
}

bool UserValidator::isValidEmail(const std::string& email) {
    if (email.empty() || email.length() > 254) return false;
    std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!std::regex_match(email, emailRegex)) return false;
    if (email.find("..") != std::string::npos) return false;
    if (email[0] == '.' || email[email.length()-1] == '.') return false;
    if (email.find('@') == 0 || email.find('@') == email.length()-1) return false;
    return true;
}

bool UserValidator::isValidPhoneNumber(const std::string& phone) {
    if (phone.empty()) return false;
    std::string cleanPhone = phone;
    cleanPhone.erase(std::remove_if(cleanPhone.begin(), cleanPhone.end(), 
        [](char c) { return c == ' ' || c == '-' || c == '(' || c == ')' || c == '+'; }), 
        cleanPhone.end());
    if (!std::all_of(cleanPhone.begin(), cleanPhone.end(), ::isdigit)) return false;
    std::vector<std::regex> phonePatterns = {
        std::regex(R"(^84[0-9]{9,10}$)"),
        std::regex(R"(^0[0-9]{9,10}$)"),
        std::regex(R"(^[0-9]{10,11}$)")
    };
    for (const auto& pattern : phonePatterns) {
        if (std::regex_match(cleanPhone, pattern)) return true;
    }
    return false;
}

bool UserValidator::validateStrongPassword(const std::string& password) {
    if (password.length() < 8) return false;

    std::regex lowercase("[a-z]");
    std::regex uppercase("[A-Z]");
    std::regex digit("[0-9]");
    std::regex special("[^a-zA-Z0-9]");

    return std::regex_search(password, lowercase) &&
           std::regex_search(password, uppercase) &&
           std::regex_search(password, digit) &&
           std::regex_search(password, special);
}
