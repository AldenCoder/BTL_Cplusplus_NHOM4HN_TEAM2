/**
 * @file UserInterface.cpp
 * @brief Console user interface implementation
 * @author Team 2C
 */

#include "UserInterface.h"
#include "../security/OTPManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <conio.h>  // For Windows
#include <limits>

UserInterface::UserInterface(AuthSystem& authSys) 
    : authSystem(authSys), isRunning(false) {
    // Initialize WalletManager
    auto dataManager = std::make_shared<DataManager>();
    auto otpManager = std::make_shared<OTPManager>();
    walletManager = std::unique_ptr<WalletManager>(new WalletManager(dataManager, otpManager));
}

UserInterface::~UserInterface() {
    // Destructor
}

void UserInterface::run() {
    // Initialize system
    if (!authSystem.initialize()) {
        showError("Cannot initialize authentication system!");
        return;
    }

    if (!walletManager->initialize()) {
        showError("Cannot initialize wallet system!");
        return;
    }

    isRunning = true;
    clearScreen();
    showHeader();
    showSuccess("System is ready!");

    while (isRunning) {
        try {
            if (!authSystem.isLoggedIn()) {
                showMainMenu();
            } else {
                if (authSystem.isCurrentUserAdmin()) {
                    showAdminMenu();
                } else {
                    showUserMenu();
                }
            }
        }
        catch (const std::exception& e) {
            showError("System error: " + std::string(e.what()));
            pauseScreen();
        }
    }
}

// ==================== MENU FUNCTIONS ====================

void UserInterface::showMainMenu() {
    clearScreen();
    showHeader();    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                   MAIN MENU                      ║\n";
    std::cout << "╠══════════════════════════════════════════════════╣\n";
    std::cout << "║  1. Login                                        ║\n";
    std::cout << "║  2. Register new account                         ║\n";
    std::cout << "║  3. Exit program                                 ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    int choice = getIntInput("Choose function: ", 1, 3);
    handleMainMenu(choice);
}

void UserInterface::showUserMenu() {
    clearScreen();
    showHeader();
      auto user = authSystem.getCurrentUser();
    std::cout << "Hello, " << user->getFullName() << "!\n\n";

    // Check if password change is required
    if (user->requirePasswordChange()) {
        showInfo("⚠️  You need to change your password before using the system!");
        changePassword();
        return;
    }

    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                   USER MENU                      ║\n";
    std::cout << "╠══════════════════════════════════════════════════╣\n";
    std::cout << "║  1. View personal information                    ║\n";
    std::cout << "║  2. Change password                              ║\n";
    std::cout << "║  3. Update personal information                  ║\n";
    std::cout << "║  4. View wallet balance                          ║\n";
    std::cout << "║  5. Transfer points                              ║\n";
    std::cout << "║  6. View transaction history                     ║\n";
    std::cout << "║  7. Wallet report                                ║\n";
    std::cout << "║  8. Logout                                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    int choice = getIntInput("Choose function: ", 1, 8);
    handleUserMenu(choice);
}

void UserInterface::showAdminMenu() {
    clearScreen();
    showHeader();
      auto user = authSystem.getCurrentUser();
    std::cout << "Hello Administrator " << user->getFullName() << "!\n\n";

    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                 ADMIN MENU                       ║\n";
    std::cout << "╠══════════════════════════════════════════════════╣\n";
    std::cout << "║  1. View personal information                    ║\n";
    std::cout << "║  2. Change password                              ║\n";
    std::cout << "║  3. View wallet balance                          ║\n";
    std::cout << "║  4. Transfer points                              ║\n";
    std::cout << "║  5. View transaction history                     ║\n";
    std::cout << "║  6. View user list                               ║\n";
    std::cout << "║  7. Create new account                           ║\n";
    std::cout << "║  8. Manage user accounts                         ║\n";
    std::cout << "║  9. View system statistics                       ║\n";
    std::cout << "║ 10. Issue points from master wallet              ║\n";
    std::cout << "║ 11. Manage data backup                           ║\n";
    std::cout << "║ 12. Logout                                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    int choice = getIntInput("Choose function: ", 1, 12);
    handleAdminMenu(choice);
}

void UserInterface::handleMainMenu(int choice) {
    switch (choice) {
        case 1: loginScreen(); break;
        case 2: registerScreen(); break;        case 3: 
            showInfo("Thank you for using the system!");
            isRunning = false; 
            break;
        default:
            showError("Invalid choice!");
            pauseScreen();
    }
}

void UserInterface::handleUserMenu(int choice) {
    switch (choice) {
        case 1: viewProfile(); break;
        case 2: changePassword(); break;
        case 3: updateProfile(); break;
        case 4: viewWalletBalance(); break;
        case 5: transferPoints(); break;
        case 6: viewTransactionHistory(); break;
        case 7: viewWalletReport(); break;
        case 8: logout(); break;
        default:
            showError("Invalid choice!");
            pauseScreen();
    }
}

void UserInterface::handleAdminMenu(int choice) {
    switch (choice) {
        case 1: viewProfile(); break;
        case 2: changePassword(); break;
        case 3: viewWalletBalance(); break;
        case 4: transferPoints(); break;
        case 5: viewTransactionHistory(); break;
        case 6: viewAllUsers(); break;
        case 7: createNewAccount(); break;
        case 8: manageUserAccount(); break;
        case 9: viewSystemStatistics(); break;
        case 10: issuePointsFromMaster(); break;
        case 11: manageBackup(); break;
        case 12: logout(); break;
        default:
            showError("Invalid choice!");
            pauseScreen();
    }
}

// ==================== AUTH FUNCTIONS ====================

void UserInterface::loginScreen() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                     LOGIN                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    std::string username = getInput("Username: ");
    if (username.empty()) return;

    std::string password = getPassword("Password: ");
    if (password.empty()) return;

    showInfo("Authenticating...");
    
    auto result = authSystem.login(username, password);
    
    if (result.success) {
        showSuccess(result.message);
        if (result.requirePasswordChange) {
            showInfo("You need to change your password on first login!");
        }
        pauseScreen();
    } else {
        showError(result.message);
        pauseScreen();
    }
}

void UserInterface::registerScreen() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                  REGISTER                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    std::string username = getInput("Username (3-20 characters): ");
    if (username.empty()) return;

    std::string fullName = getInput("Full name: ");
    if (fullName.empty()) return;

    std::string email;
    do {
        email = getInput("Email: ");
        if (email.empty()) return;
        if (!isValidEmail(email)) {
            showError("Invalid email!");
            email = "";
        }
    } while (email.empty());

    std::string phoneNumber;
    do {
        phoneNumber = getInput("Phone number (10-11 digits): ");
        if (phoneNumber.empty()) return;
        if (!isValidPhoneNumber(phoneNumber)) {
            showError("Invalid phone number!");
            phoneNumber = "";
        }
    } while (phoneNumber.empty());

    std::string password = getPassword("Password (at least 8 characters): ");
    if (password.empty()) return;

    std::string confirmPassword = getPassword("Confirm password: ");
    if (confirmPassword != password) {
        showError("Password confirmation does not match!");
        pauseScreen();
        return;
    }

    showInfo("Creating account...");
    
    auto result = authSystem.registerUser(username, password, fullName, email, phoneNumber);
    
    if (result.success) {
        showSuccess(result.message);
        showInfo("You can login now!");
    } else {
        showError(result.message);
    }
    
    pauseScreen();
}

void UserInterface::logout() {
    if (confirmAction("Are you sure you want to logout?")) {
        authSystem.logout();
        showSuccess("Logged out successfully!");
        pauseScreen();
    }
}

// ==================== USER PROFILE FUNCTIONS ====================

void UserInterface::viewProfile() {
    clearScreen();
    showHeader();
      std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                PERSONAL INFORMATION              ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    displayUserInfo(*user);
    
    pauseScreen();
}

void UserInterface::changePassword() {
    clearScreen();
    showHeader();
      std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                CHANGE PASSWORD                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    
    std::string oldPassword;
    if (!user->requirePasswordChange()) {        oldPassword = getPassword("Old password: ");
        if (oldPassword.empty()) return;
    }

    std::string newPassword = getPassword("New password (at least 8 characters): ");
    if (newPassword.empty()) return;

    std::string confirmPassword = getPassword("Confirm new password: ");    if (confirmPassword != newPassword) {
        showError("Password confirmation does not match!");
        pauseScreen();
        return;
    }

    showInfo("Updating password...");
    
    bool success = authSystem.changePassword(user->getId(), oldPassword, newPassword);
    
    if (success) {
        showSuccess("Password changed successfully!");
    } else {
        showError("Password change failed! Please check your old password.");
    }
    
    pauseScreen();
}

void UserInterface::updateProfile() {
    clearScreen();
    showHeader();
      std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║              UPDATE PERSONAL INFO               ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    
    std::cout << "Current information:\n";
    displayUserInfo(*user);
    std::cout << "\n";

    std::string newFullName = getInput("New full name (Enter to keep current): ");
    if (newFullName.empty()) newFullName = user->getFullName();

    std::string newEmail = getInput("New email (Enter to keep current): ");
    if (newEmail.empty()) {
        newEmail = user->getEmail();
    } else if (!isValidEmail(newEmail)) {
        showError("Invalid email!");
        pauseScreen();
        return;
    }

    std::string newPhoneNumber = getInput("New phone number (Enter to keep current): ");
    if (newPhoneNumber.empty()) {
        newPhoneNumber = user->getPhoneNumber();
    } else if (!isValidPhoneNumber(newPhoneNumber)) {
        showError("Invalid phone number!");
        pauseScreen();
        return;
    }

    // Request OTP
    showInfo("Generating OTP code...");
    std::string otpCode = authSystem.requestProfileUpdateOTP(user->getId());
    if (otpCode.empty()) {
        showError("Cannot generate OTP code!");
        pauseScreen();
        return;
    }

    showInfo("Your OTP code is: " + otpCode);
    showInfo("(In reality, this code would be sent via email/SMS)");
    
    std::string inputOTP = getInput("Enter OTP code: ");
    if (inputOTP.empty()) return;

    showInfo("Updating information...");
    
    bool success = authSystem.updateProfile(user->getId(), newFullName, newEmail, newPhoneNumber, inputOTP);
    
    if (success) {
        showSuccess("Profile updated successfully!");
    } else {
        showError("Profile update failed! Please check your OTP code.");
    }
    
    pauseScreen();
}

// ==================== WALLET FUNCTIONS ====================

void UserInterface::viewWalletBalance() {
    clearScreen();
    showHeader();
      std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                 WALLET BALANCE                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    auto wallet = walletManager->getWalletByUserId(user->getId());
    
    if (!wallet) {
        showError("Cannot find your wallet!");
        pauseScreen();
        return;
    }

    std::cout << "┌─────────────────────────────────────────────────┐\n";
    std::cout << "│ Wallet owner: " << std::setw(33) << user->getFullName() << " │\n";
    std::cout << "│ Wallet ID: " << std::setw(36) << wallet->getId() << " │\n";
    std::cout << "│ Current balance: " << std::setw(30) << formatCurrency(wallet->getBalance()) << " │\n";
    std::cout << "│ Status: " << std::setw(39) << (wallet->getIsLocked() ? "Locked" : "Active") << " │\n";
    std::cout << "└─────────────────────────────────────────────────┘\n\n";
    
    pauseScreen();
}

void UserInterface::transferPoints() {
    clearScreen();
    showHeader();
      std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                TRANSFER POINTS                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    auto fromWallet = walletManager->getWalletByUserId(user->getId());
    
    if (!fromWallet || fromWallet->getIsLocked()) {
        showError("Your wallet is not available!");
        pauseScreen();
        return;
    }

    std::cout << "Current balance: " << formatCurrency(fromWallet->getBalance()) << "\n\n";

    // Enter recipient information
    std::string recipientUsername = getInput("Recipient username: ");
    if (recipientUsername.empty()) return;

    auto recipient = authSystem.findUserByUsername(recipientUsername);
    if (!recipient) {
        showError("User not found!");
        pauseScreen();
        return;
    }    auto toWallet = walletManager->getWalletByUserId(recipient->getId());
    if (!toWallet || toWallet->getIsLocked()) {
        showError("Recipient wallet is not available!");
        pauseScreen();
        return;
    }

    std::cout << "Recipient: " << recipient->getFullName() << "\n\n";

    // Enter amount
    double amount = getDoubleInput("Points to transfer: ", 0.01, fromWallet->getBalance());
    if (amount <= 0) return;

    // Enter description
    std::string description = getInput("Transaction description: ");
    if (description.empty()) description = "Transfer points";

    // Generate OTP
    showInfo("Generating OTP code...");
    std::string otpCode = walletManager->generateTransferOTP(user->getId(), toWallet->getId(), amount);
    if (otpCode.empty()) {
        showError("Cannot generate OTP code!");
        pauseScreen();
        return;
    }

    showInfo("Your OTP code is: " + otpCode);
    showInfo("(In reality, this code would be sent via email/SMS)");

    // Confirm transaction
    std::cout << "\n" << "═══ TRANSACTION CONFIRMATION ═══\n";
    std::cout << "From: " << user->getFullName() << "\n";
    std::cout << "To: " << recipient->getFullName() << "\n";
    std::cout << "Amount: " << formatCurrency(amount) << "\n";
    std::cout << "Description: " << description << "\n\n";

    if (!confirmAction("Are you sure you want to execute this transaction?")) {
        showInfo("Transaction cancelled!");
        pauseScreen();
        return;
    }

    std::string inputOTP = getInput("Enter OTP code: ");
    if (inputOTP.empty()) return;

    // Execute transaction
    TransferRequest request;
    request.fromWalletId = fromWallet->getId();
    request.toWalletId = toWallet->getId();
    request.amount = amount;
    request.description = description;
    request.otpCode = inputOTP;

    showInfo("Đang thực hiện giao dịch...");
    
    auto result = walletManager->transferPoints(request);
    
    if (result.success) {
        showSuccess(result.message);
        std::cout << "ID giao dịch: " << result.transactionId << "\n";
        std::cout << "Số dư mới: " << formatCurrency(result.newBalance) << "\n";
    } else {
        showError(result.message);
    }
    
    pauseScreen();
}

void UserInterface::viewTransactionHistory() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║               LỊCH SỬ GIAO DỊCH                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    auto wallet = walletManager->getWalletByUserId(user->getId());
    
    if (!wallet) {
        showError("Không tìm thấy ví của bạn!");
        pauseScreen();
        return;
    }

    int limit = getIntInput("Số giao dịch muốn xem (0 = tất cả): ", 0, 100);
    if (limit == 0) limit = -1;

    auto transactions = walletManager->getTransactionHistory(wallet->getId(), limit);
    
    if (transactions.empty()) {
        showInfo("Chưa có giao dịch nào!");
    } else {
        displayTransactionTable(transactions);
    }
    
    pauseScreen();
}

void UserInterface::viewWalletReport() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                  BÁO CÁO VÍ                      ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto user = authSystem.getCurrentUser();
    auto wallet = walletManager->getWalletByUserId(user->getId());
    
    if (!wallet) {
        showError("Không tìm thấy ví của bạn!");
        pauseScreen();
        return;
    }

    auto transactions = walletManager->getTransactionHistory(wallet->getId());
    
    double totalIn = 0.0, totalOut = 0.0;
    int countIn = 0, countOut = 0;
    
    for (const auto& transaction : transactions) {
        if (transaction.getToWalletId() == wallet->getId()) {
            totalIn += transaction.getAmount();
            countIn++;
        } else {
            totalOut += transaction.getAmount();
            countOut++;
        }
    }

    std::cout << "┌─────────────────────────────────────────────────┐\n";
    std::cout << "│                  BÁO CÁO VÍ                     │\n";
    std::cout << "├─────────────────────────────────────────────────┤\n";
    std::cout << "│ Số dư hiện tại: " << std::setw(31) << formatCurrency(wallet->getBalance()) << " │\n";
    std::cout << "│ Tổng tiền nhận: " << std::setw(31) << formatCurrency(totalIn) << " │\n";
    std::cout << "│ Tổng tiền chuyển: " << std::setw(29) << formatCurrency(totalOut) << " │\n";
    std::cout << "│ Số giao dịch nhận: " << std::setw(28) << countIn << " │\n";
    std::cout << "│ Số giao dịch chuyển: " << std::setw(26) << countOut << " │\n";
    std::cout << "│ Tổng giao dịch: " << std::setw(31) << (countIn + countOut) << " │\n";
    std::cout << "└─────────────────────────────────────────────────┘\n\n";
    
    pauseScreen();
}

// ==================== ADMIN FUNCTIONS ====================

void UserInterface::viewAllUsers() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║              DANH SÁCH NGƯỜI DÙNG                ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    auto users = authSystem.getAllUsers();
    
    if (users.empty()) {
        showInfo("Không có người dùng nào!");
        pauseScreen();
        return;
    }

    std::cout << std::setw(5) << "STT" 
              << std::setw(15) << "Username" 
              << std::setw(25) << "Họ tên"
              << std::setw(10) << "Vai trò"
              << std::setw(12) << "Trạng thái" << "\n";
    std::cout << std::string(67, '-') << "\n";

    for (size_t i = 0; i < users.size(); i++) {
        auto user = users[i];
        std::cout << std::setw(5) << (i + 1)
                  << std::setw(15) << user->getUsername()
                  << std::setw(25) << user->getFullName()
                  << std::setw(10) << (user->getRole() == UserRole::ADMIN ? "Admin" : "User")
                  << std::setw(12) << (user->isActive() ? "Hoạt động" : "Khóa") << "\n";
    }
    
    pauseScreen();
}

void UserInterface::createNewAccount() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                TẠO TÀI KHOẢN MỚI                 ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    std::string username = getInput("Tên đăng nhập: ");
    if (username.empty()) return;

    std::string fullName = getInput("Họ và tên: ");
    if (fullName.empty()) return;

    std::string email;
    do {
        email = getInput("Email: ");
        if (email.empty()) return;
        if (!isValidEmail(email)) {
            showError("Email không hợp lệ!");
            email = "";
        }
    } while (email.empty());

    std::string phoneNumber;
    do {
        phoneNumber = getInput("Số điện thoại: ");
        if (phoneNumber.empty()) return;
        if (!isValidPhoneNumber(phoneNumber)) {
            showError("Số điện thoại không hợp lệ!");
            phoneNumber = "";
        }
    } while (phoneNumber.empty());

    std::vector<std::string> roleOptions = {"User thường", "Admin"};
    int roleChoice = showMenuSelection("Chọn vai trò:", roleOptions);
    UserRole role = (roleChoice == 2) ? UserRole::ADMIN : UserRole::REGULAR;

    showInfo("Đang tạo tài khoản...");
    
    auto result = authSystem.createAccount(username, fullName, email, phoneNumber, role, true);
    
    if (result.success) {
        showSuccess(result.message);
        if (!result.generatedPassword.empty()) {
            showInfo("Mật khẩu được tạo: " + result.generatedPassword);
            showInfo("User cần đổi mật khẩu trong lần đăng nhập đầu tiên!");
        }
    } else {
        showError(result.message);
    }
    
    pauseScreen();
}

void UserInterface::manageUserAccount() {
    showInfo("Chức năng đang được phát triển!");
    pauseScreen();
}

void UserInterface::viewSystemStatistics() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                THỐNG KÊ HỆ THỐNG                 ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    std::string stats = walletManager->getSystemStatistics();
    std::cout << stats << "\n";
    
    pauseScreen();
}

void UserInterface::issuePointsFromMaster() {
    clearScreen();
    showHeader();
    
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║            PHÁT HÀNH ĐIỂM TỪ VÍ TỔNG             ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    std::string username = getInput("Tên đăng nhập người nhận: ");
    if (username.empty()) return;

    auto user = authSystem.findUserByUsername(username);
    if (!user) {
        showError("Không tìm thấy người dùng!");
        pauseScreen();
        return;
    }

    auto wallet = walletManager->getWalletByUserId(user->getId());
    if (!wallet) {
        showError("Không tìm thấy ví của người dùng!");
        pauseScreen();
        return;
    }

    std::cout << "Người nhận: " << user->getFullName() << "\n";
    std::cout << "Số dư hiện tại: " << formatCurrency(wallet->getBalance()) << "\n\n";

    double amount = getDoubleInput("Số điểm cần phát hành: ", 0.01, 1000000.0);
    if (amount <= 0) return;

    std::string description = getInput("Lý do phát hành: ");
    if (description.empty()) description = "Admin issued points";

    if (!confirmAction("Bạn có chắc muốn phát hành " + formatCurrency(amount) + " điểm?")) {
        showInfo("Đã hủy thao tác!");
        pauseScreen();
        return;
    }

    showInfo("Đang phát hành điểm...");
    
    std::string transactionId = walletManager->issuePointsFromMaster(wallet->getId(), amount, description);
    
    if (!transactionId.empty()) {
        showSuccess("Phát hành điểm thành công!");
        std::cout << "ID giao dịch: " << transactionId << "\n";
    } else {
        showError("Phát hành điểm thất bại!");
    }
    
    pauseScreen();
}

void UserInterface::manageBackup() {
    showInfo("Chức năng sao lưu dữ liệu đang được phát triển!");
    pauseScreen();
}

// ==================== UTILITY FUNCTIONS ====================

std::string UserInterface::getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

std::string UserInterface::getPassword(const std::string& prompt) {
    std::cout << prompt;
    std::string password;
    char ch;
    
    while ((ch = _getch()) != '\r') { // '\r' là Enter
        if (ch == '\b') { // Backspace
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password += ch;
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return password;
}

int UserInterface::getIntInput(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(); // Xóa newline
            return value;
        }
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showError("Vui lòng nhập số từ " + std::to_string(min) + " đến " + std::to_string(max) + "!");
    }
}

double UserInterface::getDoubleInput(const std::string& prompt, double min, double max) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(); // Xóa newline
            return value;
        }
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showError("Vui lòng nhập số từ " + std::to_string(min) + " đến " + std::to_string(max) + "!");
    }
}

bool UserInterface::confirmAction(const std::string& message) {
    std::cout << message << " (y/N): ";
    std::string input;
    std::getline(std::cin, input);
    return input == "y" || input == "Y" || input == "yes" || input == "Yes";
}

void UserInterface::showError(const std::string& message) {
    std::cout << "❌ " << message << std::endl;
}

void UserInterface::showSuccess(const std::string& message) {
    std::cout << "✅ " << message << std::endl;
}

void UserInterface::showInfo(const std::string& message) {
    std::cout << "ℹ️  " << message << std::endl;
}

void UserInterface::pauseScreen() {
    std::cout << "\nNhấn Enter để tiếp tục...";
    std::cin.get();
}

void UserInterface::clearScreen() {
    system("cls"); // Windows
    // system("clear"); // Linux/Mac
}

void UserInterface::showHeader() {
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║        WALLET POINT MANAGEMENT SYSTEM           ║\n";
    std::cout << "║                   Team 2C                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";
}

void UserInterface::showSeparator() {
    std::cout << std::string(54, '=') << std::endl;
}

bool UserInterface::isValidEmail(const std::string& email) {
    std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, emailRegex);
}

bool UserInterface::isValidPhoneNumber(const std::string& phone) {
    std::regex phoneRegex(R"(^[0-9]{10,11}$)");
    return std::regex_match(phone, phoneRegex);
}

std::string UserInterface::formatCurrency(double amount) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << amount << " điểm";
    return oss.str();
}

std::string UserInterface::formatDateTime(const std::chrono::system_clock::time_point& timePoint) {
    auto time_t = std::chrono::system_clock::to_time_t(timePoint);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%d/%m/%Y %H:%M:%S");
    return oss.str();
}

void UserInterface::displayUserInfo(const User& user) {
    std::cout << "┌─────────────────────────────────────────────────┐\n";
    std::cout << "│ ID: " << std::setw(44) << user.getId() << " │\n";
    std::cout << "│ Tên đăng nhập: " << std::setw(32) << user.getUsername() << " │\n";
    std::cout << "│ Họ tên: " << std::setw(40) << user.getFullName() << " │\n";
    std::cout << "│ Email: " << std::setw(41) << user.getEmail() << " │\n";
    std::cout << "│ Số điện thoại: " << std::setw(32) << user.getPhoneNumber() << " │\n";
    std::cout << "│ Vai trò: " << std::setw(38) << (user.getRole() == UserRole::ADMIN ? "Admin" : "User") << " │\n";
    std::cout << "│ Trạng thái: " << std::setw(35) << (user.isActive() ? "Hoạt động" : "Bị khóa") << " │\n";
    std::cout << "│ Lần đăng nhập cuối: " << std::setw(27) << formatDateTime(user.getLastLogin()) << " │\n";
    std::cout << "└─────────────────────────────────────────────────┘\n";
}

void UserInterface::displayTransactionTable(const std::vector<Transaction>& transactions) {
    std::cout << std::setw(8) << "STT" 
              << std::setw(15) << "Thời gian"
              << std::setw(12) << "Số tiền"
              << std::setw(20) << "Mô tả" << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (size_t i = 0; i < transactions.size(); i++) {
        const auto& transaction = transactions[i];
        std::cout << std::setw(8) << (i + 1)
                  << std::setw(15) << formatDateTime(transaction.getTimestamp())
                  << std::setw(12) << formatCurrency(transaction.getAmount())
                  << std::setw(20) << transaction.getDescription() << "\n";
    }
}

int UserInterface::showMenuSelection(const std::string& title, const std::vector<std::string>& options) {
    std::cout << title << "\n";
    for (size_t i = 0; i < options.size(); i++) {
        std::cout << (i + 1) << ". " << options[i] << "\n";
    }
    
    return getIntInput("Chọn: ", 1, static_cast<int>(options.size()));
}
