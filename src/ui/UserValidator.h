// UserValidator.h
#pragma once
#include <string>
#include <functional>
#include "../models/User.h"

class UserValidator {
public:
    static bool isValidEmail(const std::string& email);
    static bool isValidPhoneNumber(const std::string& phone);
    static bool validateStrongPassword(const std::string& password);
    static bool isValidUsername(const std::string& username);
    static bool isValidFullName(const std::string& fullName);
};