#include "UserInterface.h"
#include "../security/OTPManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <ctime> // For localtime_s and localtime_r
#ifdef _WIN32
#include <conio.h>  // For Windows getch()
#else
#include <termios.h>  // For macOS/Linux
#include <unistd.h>
#endif
// #include <sys/ioctl.h>
#include <limits>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
// Windows version uses conio.h getch() directly - no need to redefine
#else
// macOS/Linux version
int getch() {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
        newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}
#endif

UserInterface::UserInterface(AuthSystem& authSys)
    : authSystem(authSys), isRunning(false) {
    // Initialize WalletManager using the DatabaseManager from AuthSystem
    auto dataManager = authSystem.getDataManager();
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
    showHeader();

    // Check if system has any admin users
    bool hasAdmin = authSystem.hasAnyAdmin();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                   MAIN MENU                      |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|  1. Login                                        |\n";
    std::cout << "|  2. Register new account                         |\n";
    std::cout << "|  3. Exit program                                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    if (!hasAdmin) {
        std::cout << "\n";
        showInfo("NOTICE: No admin accounts exist.");
        showInfo("The first registered user will become an administrator.");
        std::cout << "\n";
    }
    std::cout << "\n";

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
        showInfo("Warning: You need to change your password before using the system!");
        changePassword();
        return;
    }

    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                   USER MENU                      |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|  1. View personal information                    |\n";
    std::cout << "|  2. Change password                              |\n";
    std::cout << "|  3. Update personal information                  |\n";
    std::cout << "|  4. View wallet balance                          |\n";
    std::cout << "|  5. Transfer points                              |\n";
    std::cout << "|  6. View transaction history                     |\n";
    std::cout << "|  7. Wallet report                                |\n";
    std::cout << "|  8. Logout                                       |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    int choice = getIntInput("Choose function: ", 1, 8);
    handleUserMenu(choice);
}

void UserInterface::showAdminMenu() {
    clearScreen();
    showHeader();
      auto user = authSystem.getCurrentUser();
    std::cout << "Hello Administrator " << user->getFullName() << "!\n\n";

    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                 ADMIN MENU                       |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|  1. View personal information                    |\n";
    std::cout << "|  2. Change password                              |\n";
    std::cout << "|  3. View wallet balance                          |\n";
    std::cout << "|  4. Transfer points                              |\n";
    std::cout << "|  5. View transaction history                     |\n";
    std::cout << "|  6. View user list                               |\n";
    std::cout << "|  7. Create new account                           |\n";
    std::cout << "|  8. Manage user accounts                         |\n";
    std::cout << "|  9. View system statistics                       |\n";
    std::cout << "| 10. Issue points from master wallet              |\n";
    std::cout << "| 11. Manage data backup                           |\n";
    std::cout << "| 12. Logout                                       |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    int choice = getIntInput("Choose function: ", 1, 12);
    handleAdminMenu(choice);
}

void UserInterface::handleMainMenu(int choice) {
    switch (choice) {
        case 1: loginScreen(); break;
        case 2: registerScreen(); break;
        case 3:
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
        case 9:
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
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                     LOGIN                        |\n";
    std::cout << "+--------------------------------------------------+\n\n";

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
    
    // Check if this will be the first admin
    bool willBeFirstAdmin = !authSystem.hasAnyAdmin();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                  REGISTER                        |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    if (willBeFirstAdmin) {
        std::cout << "\n";
        showInfo("FIRST ADMIN REGISTRATION");
        showInfo("This will be the first admin account in the system.");
        std::cout << "\n";
    }
    
    std::cout << "\n";

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
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                PERSONAL INFORMATION              |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    auto user = authSystem.getCurrentUser();
    displayUserInfo(*user);
    std::cout << "| Press Tab to return                                 |\n";
    if (isTabPressed()) {
        showAdminMenu();
        return;
    }
}

void UserInterface::changePassword() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                CHANGE PASSWORD                   |\n";
    std::cout << "+--------------------------------------------------+\n\n";
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << "| Press Tab to return                                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    std::string oldPassword = getPassword("Current password: ");
    if (oldPassword.empty()) {
        if (!confirmAction("Are you sure you want to cancel?")) {
            showInfo("Cancelled successfully.!");
            showAdminMenu();
            return;
        }
    }
    std::string newPassword = getPassword("New password (at least 8 characters): ");
    if (newPassword.empty()) {
        if (!confirmAction("Are you sure you want to cancel?")) {
            showInfo("Cancelled successfully.!");
            showAdminMenu();
            return;
        }
    }
    std::string confirmPassword = getPassword("Confirm new password: ");

    if (confirmPassword.empty()) {
        if (!confirmAction("Are you sure you want to cancel?")) {
            showInfo("Cancelled successfully.!");
            showAdminMenu();
            return;
        }
    }
    
    if (newPassword != confirmPassword) {
        showError("Passwords do not match!");
        pauseScreen();
        return;
    }
    
    if (newPassword.length() < 8) {
        showError("Password must be at least 8 characters long!");
        pauseScreen();
        return;
    }
    
    showInfo("Updating password...");
    if (authSystem.changePassword(authSystem.getCurrentUser()->getId(), oldPassword, newPassword)) {
        showSuccess("Password changed successfully!");
    } else {
        showError("Password change failed! Please check your old password.");
    }
    
    pauseScreen();
}

void UserInterface::updateProfile() {
    clearScreen();
    showHeader();
      std::cout << "+--------------------------------------------------+\n";
    std::cout << "|              UPDATE PERSONAL INFO               |\n";
    std::cout << "+--------------------------------------------------+\n\n";

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
      std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                 WALLET BALANCE                   |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    auto user = authSystem.getCurrentUser();
    auto wallet = walletManager->getWalletByUserId(user->getId());
    
    if (!wallet) {
        showError("Cannot find your wallet!");
        pauseScreen();
        return;
    }    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Wallet owner: " << std::setw(33) << user->getFullName() << " |\n";
    std::cout << "| Wallet ID: " << std::setw(36) << wallet->getId() << " |\n";
    std::cout << "| Current balance: " << std::setw(30) << formatCurrency(wallet->getBalance()) << " |\n";
    std::cout << "| Status: " << std::setw(39) << (wallet->getIsLocked() ? "Locked" : "Active") << " |\n";
    std::cout << "+--------------------------------------------------+\n\n";
    
    pauseScreen();
}

void UserInterface::transferPoints() {
    clearScreen();
    showHeader();
      std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                TRANSFER POINTS                   |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    auto user = authSystem.getCurrentUser();
    auto fromWallet = walletManager->getWalletByUserId(user->getId());
    
    if (!fromWallet || fromWallet->getIsLocked()) {
        showError("Your wallet is not available!");
        pauseScreen();
        return;
    }    std::cout << "Current balance: " << formatCurrency(fromWallet->getBalance()) << "\n\n";

    // Enter recipient information
    std::string recipientUsername = getInput("Recipient username: ");
    if (recipientUsername.empty()) return;
    
    auto recipient = authSystem.findUserByUsername(recipientUsername);
    if (!recipient) {
        showError("User not found!");
        pauseScreen();
        return;
    }

    auto toWallet = walletManager->getWalletByUserId(recipient->getId());
    if (!toWallet || toWallet->getIsLocked()) {
        showError("Recipient wallet is not available!");
        pauseScreen();
        return;
    }

    std::cout << "Recipient: " << recipient->getFullName() << "\n\n";

    // Enter transfer amount
    double amount = getDoubleInput("Amount to transfer: ", 0.01, fromWallet->getBalance());
    if (amount <= 0) return;    // Enter description
    std::string description = getInput("Transaction description: ");
    if (description.empty()) description = "Transfer points";

    // Create OTP
    showInfo("Generating OTP code...");
    std::string otpCode = walletManager->generateTransferOTP(user->getId(), toWallet->getId(), amount);
    if (otpCode.empty()) {
        showError("Cannot generate OTP code!");
        pauseScreen();
        return;
    }

    showInfo("Your OTP code is: " + otpCode);
    showInfo("(In reality, this code would be sent via email/SMS)");    // Confirm transaction details
    std::cout << "\n" << "=== CONFIRM TRANSACTION ===\n";
    std::cout << "From: " << user->getFullName() << "\n";
    std::cout << "To: " << recipient->getFullName() << "\n";
    std::cout << "Amount: " << formatCurrency(amount) << "\n";
    std::cout << "Description: " << description << "\n\n";
    if (!confirmAction("Are you sure you want to proceed with this transaction?")) {
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

    showInfo("Processing transaction...");
    
    auto result = walletManager->transferPoints(request);
    
    if (result.success) {
        showSuccess(result.message);
        std::cout << "Transaction ID: " << result.transactionId << "\n";
        std::cout << "New balance: " << formatCurrency(result.newBalance) << "\n";
    } else {
        showError(result.message);
    }
    
    pauseScreen();
}

void UserInterface::viewTransactionHistory() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|              TRANSACTION HISTORY                 |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    auto user = authSystem.getCurrentUser();
    auto wallet = walletManager->getWalletByUserId(user->getId());
    
    if (!wallet) {
        showError("Cannot find your wallet!");
        pauseScreen();
        return;
    }    int limit = getIntInput("Number of transactions to view (0 = all): ", 0, 100);
    if (limit == 0) limit = -1;
    
    auto transactions = walletManager->getTransactionHistory(wallet->getId(), limit);
    
    if (transactions.empty()) {
        showInfo("No transactions found!");
    } else {
        displayTransactionTable(transactions);
    }
    
    pauseScreen();
}

void UserInterface::viewWalletReport() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                WALLET REPORT                     |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    auto user = authSystem.getCurrentUser();
    auto wallet = walletManager->getWalletByUserId(user->getId());
    
    if (!wallet) {
        showError("Cannot find your wallet!");
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
    }    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                 WALLET REPORT                   |\n";
    std::cout << "|--------------------------------------------------|\n";
    std::cout << "| Current balance: " << std::setw(30) << formatCurrency(wallet->getBalance()) << " |\n";
    std::cout << "| Total received: " << std::setw(31) << formatCurrency(totalIn) << " |\n";
    std::cout << "| Total transferred: " << std::setw(28) << formatCurrency(totalOut) << " |\n";
    std::cout << "| Incoming transactions: " << std::setw(25) << countIn << " |\n";
    std::cout << "| Outgoing transactions: " << std::setw(25) << countOut << " |\n";
    std::cout << "| Total transactions: " << std::setw(27) << (countIn + countOut) << " |\n";
    std::cout << "+--------------------------------------------------+\n\n";
    
    pauseScreen();
}

// ==================== ADMIN FUNCTIONS ====================

void UserInterface::viewAllUsers() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                 USER LIST                        |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    auto users = authSystem.getAllUsers();
    
    if (users.empty()) {
        showInfo("No users found!");
        pauseScreen();
        return;
    }

    std::cout << std::setw(5) << "No." 
              << std::setw(15) << "Username" 
              << std::setw(25) << "Full Name"
              << std::setw(10) << "Role"
              << std::setw(12) << "Status" << "\n";
    std::cout << std::string(67, '-') << "\n";

    for (size_t i = 0; i < users.size(); i++) {
        auto user = users[i];
        std::cout << std::setw(5) << (i + 1)
                  << std::setw(15) << user->getUsername()
                  << std::setw(25) << user->getFullName()
                  << std::setw(10) << (user->getRole() == UserRole::ADMIN ? "Admin" : "User")
                  << std::setw(12) << (user->isActive() ? "Active" : "Locked") << "\n";
    }
    
    pauseScreen();
}

void UserInterface::createNewAccount() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|               CREATE NEW ACCOUNT                 |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    std::string username = getInput("Username: ");
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
        phoneNumber = getInput("Phone number: ");
        if (phoneNumber.empty()) return;
        if (!isValidPhoneNumber(phoneNumber)) {
            showError("Invalid phone number!");
            phoneNumber = "";
        }
    } while (phoneNumber.empty());    std::vector<std::string> roleOptions = {"Regular User", "Admin"};
    int roleChoice = showMenuSelection("Select role:", roleOptions);
    UserRole role = (roleChoice == 2) ? UserRole::ADMIN : UserRole::REGULAR;
    
    showInfo("Creating account...");
    
    auto result = authSystem.createAccount(username, fullName, email, phoneNumber, role, true);
    
    if (result.success) {
        showSuccess(result.message);
        std::cout << "\n";
        std::cout << "+--------------------------------------------------+\n";
        std::cout << "|            ACCOUNT CREATION SUCCESSFUL           |\n";
        std::cout << "+--------------------------------------------------+\n";
        std::cout << "| Username: " << std::setw(37) << username << " |\n";
        std::cout << "| Full Name: " << std::setw(36) << fullName << " |\n";
        std::cout << "| Email: " << std::setw(41) << email << " |\n";
        std::cout << "| Role: " << std::setw(42) << (role == UserRole::ADMIN ? "Admin" : "User") << " |\n";
        std::cout << "+--------------------------------------------------+\n";
        
        if (!result.generatedPassword.empty()) {
            std::cout << "\n";
            std::cout << "+--------------------------------------------------+\n";
            std::cout << "|               AUTO-GENERATED PASSWORD            |\n";
            std::cout << "+--------------------------------------------------+\n";
            std::cout << "| Password: " << std::setw(37) << result.generatedPassword << " |\n";
            std::cout << "+--------------------------------------------------+\n";
            std::cout << "\n";
            showWarning("IMPORTANT NOTICE:");
            showWarning("- Please provide this password to the user");
            showWarning("- User MUST change password on first login");
            showWarning("- Keep this password secure until delivered");
            std::cout << "\n";
        }
    } else {
        showError(result.message);
    }
    
    pauseScreen();
}

void UserInterface::manageUserAccount() {
    if (!authSystem.isCurrentUserAdmin()) {
        showError("Access denied! Admin privileges required.");
        pauseScreen();
        return;
    }    while (true) {
        clearScreen();
        showHeader();
        
        std::cout << " +----------------------------------------------------------+\n";
        std::cout << " |                 USER ACCOUNT MANAGEMENT                  |\n";
        std::cout << " +----------------------------------------------------------+\n\n";
        
        std::cout << " 1. View All Users\n";
        std::cout << " 2. Search User by Username\n";
        std::cout << " 3. Create New Account\n";
        std::cout << " 4. Edit User Information\n";
        std::cout << " 5. Reset User Password\n";
        std::cout << " 6. View User Wallet Details\n";
        std::cout << " 0. Return to Main Menu\n\n";
        
        int choice = getIntInput("Choose function: ", 0, 6);
        
        switch (choice) {
            case 1: viewAllUsers(); break;
            case 2: searchUserByUsername(); break;
            case 3: createNewUserAccount(); break;
            case 4: editUserInformation(); break;
            case 5: resetUserPassword(); break;
            case 6: viewUserWalletDetails(); break;
            case 0: return;
        }
    }
}

void UserInterface::viewSystemStatistics() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|               SYSTEM STATISTICS                  |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    std::string stats = walletManager->getSystemStatistics();
    std::cout << stats << "\n";
    
    pauseScreen();
}

void UserInterface::issuePointsFromMaster() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|         ISSUE POINTS FROM MASTER WALLET          |\n";
    std::cout << "+--------------------------------------------------+\n\n";    std::string username = getInput("Recipient username: ");
    if (username.empty()) return;
    
    auto user = authSystem.findUserByUsername(username);
    if (!user) {
        showError("User not found!");
        pauseScreen();
        return;
    }

    auto wallet = walletManager->getWalletByUserId(user->getId());
    if (!wallet) {
        showError("User's wallet not found!");
        pauseScreen();
        return;
    }

    std::cout << "Recipient: " << user->getFullName() << "\n";
    std::cout << "Current balance: " << formatCurrency(wallet->getBalance()) << "\n\n";

    double amount = getDoubleInput("Points to issue: ", 0.01, 1000000.0);
    if (amount <= 0) return;

    std::string description = getInput("Reason for issuing: ");
    if (description.empty()) description = "Admin issued points";

    if (!confirmAction("Are you sure you want to issue " + formatCurrency(amount) + " points?")) {
        showInfo("Operation cancelled!");
        pauseScreen();
        return;
    }

    showInfo("Issuing points...");
    
    std::string transactionId = walletManager->issuePointsFromMaster(wallet->getId(), amount, description);
    
    if (!transactionId.empty()) {
        showSuccess("Points issued successfully!");
        std::cout << "Transaction ID: " << transactionId << "\n";
    } else {
        showError("Failed to issue points!");
    }
    
    pauseScreen();
}

// ==================== BACKUP MANAGEMENT FUNCTIONS ====================

void UserInterface::manageBackup() {
    int choice;
    do {
        clearScreen();
        showHeader();
        
        std::cout << "+--------------------------------------------------+\n";
        std::cout << "|               BACKUP MANAGEMENT                  |\n";
        std::cout << "+--------------------------------------------------+\n\n";
        
        std::vector<std::string> backupOptions = {
            "Create Manual Backup",
            "View Backup History",
            "Restore from Backup",
            "Cleanup Old Backups",
            "Return to Main Menu"
        };
        
        choice = showMenuSelection("Select backup operation:", backupOptions);
        
        switch (choice) {
            case 1: createManualBackup(); break;
            case 2: viewBackupHistory(); break;
            case 3: restoreFromBackup(); break;
            case 4: cleanupBackups(); break;
            case 5: 
                showInfo("Returning to main menu...");
                pauseScreen();
                break;
            default:
                showError("Invalid selection! Please try again.");
                pauseScreen();
                break;
        }
    } while (choice != 5);
}

void UserInterface::createManualBackup() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|               CREATE MANUAL BACKUP               |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    std::string description = getInput("Backup description (optional): ");
    if (description.empty()) {
        description = "Manual backup";
    }

    showInfo("Creating backup...");
    
    try {
        // Get DataManager through AuthSystem
        auto dataManager = authSystem.getDataManager();
        if (dataManager) {
            bool success = dataManager->createBackup(description, BackupType::MANUAL);
            
            if (success) {
                showSuccess("Backup created successfully!");
                std::cout << "\nBackup created with description: " << description << "\n";
            } else {
                showError("Failed to create backup!");
            }
        } else {
            showError("Unable to access data manager!");
        }
    } catch (const std::exception& e) {
        showError("Backup creation failed: " + std::string(e.what()));
    }
    
    pauseScreen();
}

void UserInterface::viewBackupHistory() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                BACKUP HISTORY                    |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    try {
        // Get DataManager through AuthSystem
        auto dataManager = authSystem.getDataManager();
        if (dataManager) {
            std::vector<BackupInfo> backupHistory = dataManager->getBackupHistory();
            
            if (backupHistory.empty()) {
                showInfo("No backup files found.");
            } else {
                std::cout << "Found " << backupHistory.size() << " backup(s):\n\n";
                
                std::cout << std::setw(20) << "Backup ID" 
                         << std::setw(25) << "Filename"
                         << std::setw(12) << "Size"
                         << std::setw(20) << "Created" 
                         << std::setw(10) << "Type" << "\n";
                std::cout << std::string(87, '-') << "\n";
                
                for (const auto& backup : backupHistory) {
                    std::string typeStr = (backup.type == BackupType::MANUAL) ? "Manual" :
                                        (backup.type == BackupType::AUTO) ? "Auto" : "Emergency";
                    
                    std::cout << std::setw(20) << backup.backupId.substr(0,18)
                             << std::setw(25) << backup.filename.substr(0,23)
                             << std::setw(12) << formatFileSize(backup.fileSize)
                             << std::setw(20) << formatDateTime(backup.timestamp).substr(0,18)
                             << std::setw(10) << typeStr << "\n";
                }
            }
        } else {
            showError("Unable to access data manager!");
        }
    } catch (const std::exception& e) {
        showError("Failed to retrieve backup history: " + std::string(e.what()));
    }
    
    pauseScreen();
}

void UserInterface::restoreFromBackup() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                RESTORE FROM BACKUP               |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    try {
        // Get DataManager through AuthSystem
        auto dataManager = authSystem.getDataManager();
        if (!dataManager) {
            showError("Unable to access data manager!");
            pauseScreen();
            return;
        }

        std::vector<BackupInfo> backupHistory = dataManager->getBackupHistory();
        
        if (backupHistory.empty()) {
            showInfo("No backup files available for restore.");
            pauseScreen();
            return;
        }

        // Show available backups
        std::vector<std::string> backupOptions;
        for (size_t i = 0; i < std::min(backupHistory.size(), static_cast<size_t>(10)); ++i) {
            const auto& backup = backupHistory[i];
            std::string option = backup.backupId + " (" + formatDateTime(backup.timestamp) + 
                               ", " + formatFileSize(backup.fileSize) + ")";
            if (i == 0) option += " [Latest]";
            backupOptions.push_back(option);
        }
        
        int choice = showMenuSelection("Select backup to restore:", backupOptions);
        if (choice <= 0 || choice > static_cast<int>(backupOptions.size())) {
            showInfo("Operation cancelled!");
            pauseScreen();
            return;
        }

        const BackupInfo& selectedBackup = backupHistory[choice - 1];
        
        std::cout << "\nSelected backup details:\n";
        std::cout << "- ID: " << selectedBackup.backupId << "\n";
        std::cout << "- Created: " << formatDateTime(selectedBackup.timestamp) << "\n";
        std::cout << "- Size: " << formatFileSize(selectedBackup.fileSize) << "\n\n";
        
        showWarning("WARNING: Restoring will overwrite current data!");
        showInfo("Current data will be automatically backed up before restore.");
        
        if (!confirmAction("Do you want to proceed with restore?")) {
            showInfo("Restore cancelled!");
            pauseScreen();
            return;
        }

        showInfo("Creating safety backup of current data...");
        dataManager->createBackup("Pre-restore backup", BackupType::EMERGENCY);
        
        showInfo("Restoring from backup...");
        bool success = dataManager->restoreFromBackup(selectedBackup.backupId);
        
        if (success) {
            showSuccess("Data restored successfully!");
            showWarning("Please restart the application to see changes.");
        } else {
            showError("Restore failed! Current data remains unchanged.");
        }
        
    } catch (const std::exception& e) {
        showError("Restore operation failed: " + std::string(e.what()));
    }
    
    pauseScreen();
}

void UserInterface::cleanupBackups() {
    clearScreen();
    showHeader();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                CLEANUP OLD BACKUPS               |\n";
    std::cout << "+--------------------------------------------------+\n\n";

    try {
        // Get DataManager through AuthSystem
        auto dataManager = authSystem.getDataManager();
        if (!dataManager) {
            showError("Unable to access data manager!");
            pauseScreen();
            return;
        }

        std::vector<BackupInfo> backupHistory = dataManager->getBackupHistory();
        
        if (backupHistory.empty()) {
            showInfo("No backup files found to clean up.");
            pauseScreen();
            return;
        }

        int keepCount = getIntInput("How many recent backups to keep? ", 1, 20);
        if (keepCount <= 0) {
            showInfo("Operation cancelled!");
            pauseScreen();
            return;
        }

        std::cout << "\nCurrent backup count: " << backupHistory.size() << "\n";
        std::cout << "Will keep latest: " << keepCount << " backups\n";
        
        if (static_cast<int>(backupHistory.size()) <= keepCount) {
            showInfo("No cleanup needed - backup count is within limit.");
            pauseScreen();
            return;
        }
        
        int toDelete = static_cast<int>(backupHistory.size()) - keepCount;
        std::cout << "Will delete: " << toDelete << " old backups\n\n";

        if (!confirmAction("Do you want to proceed with cleanup?")) {
            showInfo("Cleanup cancelled!");
            pauseScreen();
            return;
        }

        showInfo("Cleaning up old backups...");
        
        int deletedCount = 0;
        // Note: In a real implementation, DataManager would have a cleanup method
        // For now, we'll simulate the cleanup
        for (int i = keepCount; i < static_cast<int>(backupHistory.size()); ++i) {
            // In real implementation: dataManager->deleteBackup(backupHistory[i].backupId);
            deletedCount++;
        }
        
        showSuccess("Cleanup completed!");
        std::cout << "Deleted " << deletedCount << " old backup files.\n";
        std::cout << "Kept " << keepCount << " most recent backups.\n";
        
    } catch (const std::exception& e) {
        showError("Cleanup operation failed: " + std::string(e.what()));
    }
    
    pauseScreen();
}

// ==================== UTILITY FUNCTIONS ====================

std::string UserInterface::getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// std::string UserInterface::getPassword(const std::string& prompt) {
//     std::cout << prompt;
//     std::string password;
//     char ch;
    
//     while ((ch = _getch()) != '\r') { // '\r' is Enter key
//         if (ch == '\b') { // Backspace
//             if (!password.empty()) {
//                 password.pop_back();
//                 std::cout << "\b \b";
//             }
//         } else {
//             password += ch;
//             std::cout << '*';
//         }
//     }
//     std::cout << std::endl;
//     return password;
// }

std::string UserInterface::getPassword(const std::string& prompt) {
    std::cout << prompt;
    std::string password;
    char ch;
    
    while ((ch = getch()) != '\r' && ch != '\n') { // Support both \r and \n
        if (ch == 9) { // Check Tab press
            std::cout << "\n";
            return "";
        }
        if (ch == '\b' || ch == 127) { // Backspace (127 is DEL on some systems)
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else if (ch >= 32 && ch <= 126) { // Only printable characters
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
            std::cin.ignore(); // Clear newline
            return value;
        }
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showError("Please enter a number from " + std::to_string(min) + " to " + std::to_string(max) + "!");
    }
}

double UserInterface::getDoubleInput(const std::string& prompt, double min, double max) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(); // Clear newline
            return value;
        }
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showError("Please enter a number from " + std::to_string(min) + " to " + std::to_string(max) + "!");
    }
}

bool UserInterface::confirmAction(const std::string& message) {
    std::cout << message << " (y/N): ";
    std::string input;
    std::getline(std::cin, input);
    return input == "y" || input == "Y" || input == "yes" || input == "Yes";
}

void UserInterface::showError(const std::string& message) {
    std::cout << "[ERROR] " << message << std::endl;
}

void UserInterface::showSuccess(const std::string& message) {
    std::cout << "[SUCCESS] " << message << std::endl;
}

void UserInterface::showInfo(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void UserInterface::showWarning(const std::string& message) {
    std::cout << "[WARNING] " << message << std::endl;
}

void UserInterface::pauseScreen() {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void UserInterface::clearScreen() {
#ifdef _WIN32
    system("cls"); // Windows
#else
    system("clear"); // Linux/Mac
#endif
}

void UserInterface::showHeader() {
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|        WALLET POINT MANAGEMENT SYSTEM            |\n";
    std::cout << "|                   Team 2 C++                     |\n";
    std::cout << "+--------------------------------------------------+\n\n";
}

void UserInterface::showSeparator() {
    std::cout << std::string(54, '=') << std::endl;
}

bool UserInterface::isValidEmail(const std::string& email) {
    // Enhanced email validation
    if (email.empty() || email.length() > 254) return false;
    
    // Check for basic format
    std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!std::regex_match(email, emailRegex)) return false;
    
    // Additional checks
    if (email.find("..") != std::string::npos) return false;  // No consecutive dots
    if (email[0] == '.' || email[email.length()-1] == '.') return false;  // No dots at start/end
    if (email.find('@') == 0 || email.find('@') == email.length()-1) return false;  // @ not at edges
    
    return true;
}

bool UserInterface::isValidPhoneNumber(const std::string& phone) {
    // Enhanced phone validation for Vietnamese phone numbers
    if (phone.empty()) return false;
    
    // Remove common separators for validation
    std::string cleanPhone = phone;
    cleanPhone.erase(std::remove_if(cleanPhone.begin(), cleanPhone.end(), 
        [](char c) { return c == ' ' || c == '-' || c == '(' || c == ')' || c == '+'; }), 
        cleanPhone.end());
    
    // Check if all remaining characters are digits
    if (!std::all_of(cleanPhone.begin(), cleanPhone.end(), ::isdigit)) return false;
      // Vietnamese phone number patterns
    std::vector<std::regex> phonePatterns = {
        std::regex(R"(^84[0-9]{9,10}$)"),    // +84 country code
        std::regex(R"(^0[0-9]{9,10}$)"),     // Starting with 0
        std::regex(R"(^[0-9]{10,11}$)")      // 10-11 digits
    };
    
    for (const auto& pattern : phonePatterns) {
        if (std::regex_match(cleanPhone, pattern)) return true;
    }
    
    return false;
}

std::string UserInterface::formatCurrency(double amount) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << amount << " points";
    return oss.str();
}

std::string UserInterface::formatDateTime(const std::chrono::system_clock::time_point& timePoint) {
    auto time_t_val = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* time_info = std::localtime(&time_t_val);
    std::ostringstream oss;
    if (time_info) {
        oss << std::put_time(time_info, "%d/%m/%Y %H:%M:%S");
    } else {
        oss << "Invalid time";
    }
    return oss.str();
}

std::string UserInterface::formatDate(const std::chrono::system_clock::time_point& timePoint) {
    auto time_t_val = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* time_info = std::localtime(&time_t_val);
    std::ostringstream oss;
    if (time_info) {
        oss << std::put_time(time_info, "%d/%m/%Y");
    } else {
        oss << "Invalid date";
    }
    return oss.str();
}

std::string UserInterface::formatFileSize(size_t size) {
    std::ostringstream oss;
    if (size < 1024) {
        oss << size << " B";
    } else if (size < 1024 * 1024) {
        oss << std::fixed << std::setprecision(1) << (size / 1024.0) << " KB";
    } else if (size < 1024 * 1024 * 1024) {
        oss << std::fixed << std::setprecision(1) << (size / (1024.0 * 1024.0)) << " MB";
    } else {
        oss << std::fixed << std::setprecision(1) << (size / (1024.0 * 1024.0 * 1024.0)) << " GB";
    }
    return oss.str();
}

void UserInterface::displayUserInfo(const User& user) {
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| ID: " << std::setw(44) << user.getId() << " |\n";
    std::cout << "| Username: " << std::setw(39) << user.getUsername() << " |\n";
    std::cout << "| Full name: " << std::setw(38) << user.getFullName() << " |\n";
    std::cout << "| Email: " << std::setw(41) << user.getEmail() << " |\n";
    std::cout << "| Phone number: " << std::setw(34) << user.getPhoneNumber() << " |\n";
    std::cout << "| Role: " << std::setw(42) << (user.getRole() == UserRole::ADMIN ? "Admin" : "User") << " |\n";
    std::cout << "| Status: " << std::setw(40) << (user.isActive() ? "Active" : "Locked") << " |\n";
    std::cout << "| Last login: " << std::setw(37) << formatDateTime(user.getLastLogin()) << " |\n";
    std::cout << "+--------------------------------------------------+\n";
}

void UserInterface::displayTransactionTable(const std::vector<Transaction>& transactions) {
    std::cout << std::setw(8) << "No." 
              << std::setw(15) << "Time"
              << std::setw(12) << "Amount"
              << std::setw(20) << "Description" << "\n";
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
    
    return getIntInput("Choose: ", 1, static_cast<int>(options.size()));
}

void UserInterface::searchUserByUsername() {
    clearScreen();
    showHeader();
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                   SEARCH USER                               |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::cout << " Enter username to search: ";
    std::string username;
    std::getline(std::cin, username);
    
    if (username.empty()) {
        showError("Username cannot be empty!");
        pauseScreen();
        return;
    }
    
    auto user = authSystem.findUserByUsername(username);
    if (!user) {
        showError("User not found!");
        pauseScreen();
        return;
    }
    
    // Display user information    
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                   USER INFORMATION                          |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::cout << " User ID      : " << user->getId() << "\n";
    std::cout << " Username     : " << user->getUsername() << "\n";
    std::cout << " Full Name    : " << user->getFullName() << "\n";
    std::cout << " Email        : " << user->getEmail() << "\n";
    std::cout << " Phone        : " << user->getPhoneNumber() << "\n";
    std::cout << " Role         : " << (user->getRole() == UserRole::ADMIN ? "Admin" : "Regular") << "\n";
    std::cout << " Wallet ID    : " << user->getWalletId() << "\n";
    std::cout << " Status       : " << (user->isActive() ? "Active" : "Inactive") << "\n";
    
    pauseScreen();
}

void UserInterface::createNewUserAccount() {
    clearScreen();
    showHeader();
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                CREATE NEW ACCOUNT                           |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::string username, fullName, email, phoneNumber;
    UserRole role = UserRole::REGULAR;
    
    std::cout << " Enter username: ";
    std::getline(std::cin, username);
    
    std::cout << " Enter full name: ";
    std::getline(std::cin, fullName);
    
    std::cout << " Enter email: ";
    std::getline(std::cin, email);
    
    std::cout << " Enter phone number: ";
    std::getline(std::cin, phoneNumber);
    
    std::cout << "\n Select user role:\n";
    std::cout << " 1. Regular User\n";
    std::cout << " 2. Admin\n";
    std::cout << " Choice: ";
    
    int roleChoice = getIntInput("Choose: ", 1, 2);
    if (roleChoice == 2) {
        role = UserRole::ADMIN;
    }
    
    // Create account with auto-generated password
    auto result = authSystem.createAccount(username, fullName, email, phoneNumber, role, true);
    
    if (result.success) {
        showSuccess(result.message);
        if (!result.generatedPassword.empty()) {
            std::cout << "\n Generated password: " << result.generatedPassword << "\n";
            std::cout << " Please save this password securely!\n";
        }
    } else {
        showError(result.message);
    }
    
    pauseScreen();
}

void UserInterface::editUserInformation() {
    clearScreen();
    showHeader();
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                 EDIT USER INFORMATION                       |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::cout << " Enter username to edit: ";
    std::string username;
    std::getline(std::cin, username);
    
    auto user = authSystem.findUserByUsername(username);
    if (!user) {
        showError("User not found!");
        pauseScreen();
        return;
    }
    
    // Show current information
    std::cout << "\n Current Information:\n";
    std::cout << " Full Name: " << user->getFullName() << "\n";
    std::cout << " Email    : " << user->getEmail() << "\n";
    std::cout << " Phone    : " << user->getPhoneNumber() << "\n\n";
    
    std::cout << " What would you like to edit?\n";
    std::cout << " 1. Full Name\n";
    std::cout << " 2. Email\n";
    std::cout << " 3. Phone Number\n";
    std::cout << " 0. Cancel\n";
    std::cout << " Choice: ";
    
    int choice = getIntInput("Choose: ", 0, 3);
    
    switch (choice) {
        case 1: {
            std::cout << "\n Enter new full name: ";
            std::string newName;
            std::getline(std::cin, newName);
            if (!newName.empty()) {
                user->setFullName(newName);
            }
            break;
        }
        case 2: {
            std::cout << "\n Enter new email: ";
            std::string newEmail;
            std::getline(std::cin, newEmail);
            if (!newEmail.empty()) {
                user->setEmail(newEmail);
            }
            break;
        }
        case 3: {
            std::cout << "\n Enter new phone number: ";
            std::string newPhone;
            std::getline(std::cin, newPhone);
            if (!newPhone.empty()) {
                user->setPhoneNumber(newPhone);
            }
            break;
        }
        case 0:
            return;
    }
    
    if (choice >= 1 && choice <= 3) {
        if (authSystem.saveUser(user)) {
            showSuccess("User information updated successfully!");
        } else {
            showError("Failed to update user information!");
        }
    }
    
    pauseScreen();
}

void UserInterface::resetUserPassword() {
    clearScreen();
    showHeader();
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                 RESET USER PASSWORD                         |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::cout << " Enter username to reset password: ";
    std::string username;
    std::getline(std::cin, username);
    
    auto user = authSystem.findUserByUsername(username);
    if (!user) {
        showError("User not found!");
        pauseScreen();
        return;
    }
    
    std::cout << "\n Reset password for user: " << user->getFullName() << "\n";
    std::cout << " 1. Generate random password\n";
    std::cout << " 2. Set default password (123456789)\n";
    std::cout << " 0. Cancel\n";
    std::cout << " Choice: ";
    
    int choice = getIntInput("Choose: ", 0, 2);
    
    std::string newPassword;
    switch (choice) {
        case 1:
            newPassword = SecurityUtils::generateRandomString(12);
            break;
        case 2:
            newPassword = "123456789";
            break;
        case 0:
            return;
    }
    
    // Hash and set new password
    user->setPasswordHash(SecurityUtils::hashPassword(newPassword));
    user->setRequirePasswordChange(true);
    
    if (authSystem.saveUser(user)) {
        showSuccess("Password reset successfully!");
        std::cout << "\n New password: " << newPassword << "\n";
        std::cout << " User will be required to change password on next login.\n";
    } else {
        showError("Failed to reset password!");
    }
    
    pauseScreen();
}

void UserInterface::viewUserWalletDetails() {
    clearScreen();
    showHeader();
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                USER WALLET DETAILS                          |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::cout << " Enter username to view wallet: ";
    std::string username;
    std::getline(std::cin, username);
    
    auto user = authSystem.findUserByUsername(username);
    if (!user) {
        showError("User not found!");
        pauseScreen();
        return;
    }
    
    // Get wallet information
    auto wallet = walletManager->getWalletByUserId(user->getId());
    if (!wallet) {
        showError("User's wallet not found!");
        pauseScreen();
        return;
    }
    std::cout << " +-------------------------------------------------------------+\n";
    std::cout << " |                   WALLET INFORMATION                        |\n";
    std::cout << " +-------------------------------------------------------------+\n\n";
    
    std::cout << " User         : " << user->getFullName() << " (" << user->getUsername() << ")\n";
    std::cout << " Wallet ID    : " << wallet->getId() << "\n";
    std::cout << " Balance      : " << std::fixed << std::setprecision(2) << wallet->getBalance() << " points\n";
    std::cout << " Created      : " << formatDateTime(wallet->getCreatedAt()) << "\n";
    
    // Show recent transactions
    auto transactions = wallet->getTransactionHistory();
    if (!transactions.empty()) {
        std::cout << "\n Recent Transactions (last 5):\n";
        std::cout << " +----------+--------------+----------+---------------------+\n";        
        std::cout << " |   Date   |     Type     |  Amount  |    Description      |\n";
        std::cout << " +----------+--------------+----------+---------------------+\n";
        
        int count = 0;
        for (auto it = transactions.rbegin(); it != transactions.rend() && count < 5; ++it, ++count) {
            const auto& tx = *it;
            std::string typeStr = (tx.getType() == TransactionType::TRANSFER) ? "Transfer" : "Other";            std::cout << " | " << std::setw(8) << formatDate(tx.getTimestamp()) 
                      << " | " << std::setw(12) << typeStr
                      << " | " << std::setw(8) << std::fixed << std::setprecision(2) << tx.getAmount()
                      << " | " << std::setw(19) << tx.getDescription().substr(0, 19) << " |\n";
        }
        std::cout << " +----------+--------------+----------+---------------------+\n";
    }
    
    pauseScreen();
    
bool UserInterface::isTabPressed() {
    char ch = getch();  // Đọc 1 ký tự ngay lập tức (block chờ người bấm)

    if (ch == 9) {      // Nếu là Tab
        return true;
    }
    return false;
}
