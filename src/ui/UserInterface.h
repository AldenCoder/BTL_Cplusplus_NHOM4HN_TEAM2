#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "../system/AuthSystem.h"
#include "../system/WalletManager.h"
#include "../storage/DatabaseManager.h"
#include <iostream>
#include <string>
#include <memory>

class UserInterface {
private:
    AuthSystem& authSystem;
    std::unique_ptr<WalletManager> walletManager;
    bool isRunning;

public:
    UserInterface(AuthSystem& authSys);
    ~UserInterface();
    void run();

private:
    void showMainMenu();
    void showUserMenu();
    void showAdminMenu();
    void handleMainMenu(int choice);
    void handleUserMenu(int choice);
    void handleAdminMenu(int choice);
    
    void loginScreen();
    void registerScreen();
    void logout();
    
    void viewProfile();
    void changePassword();
    void updateProfile();
    
    void viewWalletBalance();
    void transferPoints();
    void viewTransactionHistory();
    void viewWalletReport();
    
    void viewAllUsers();
    void searchUserByUsername();
    void createNewUserAccount();
    void editUserInformation();
    void resetUserPassword();
    void viewUserWalletDetails();
    void createNewAccount();
    void manageUserAccount();
    void viewSystemStatistics();
    void issuePointsFromMaster();
    void manageBackup();
    void createManualBackup();
    void viewBackupHistory();
    void restoreFromBackup();
    void cleanupBackups();
    
    std::string getInput(const std::string& prompt);
    std::string getPassword(const std::string& prompt);
    int getIntInput(const std::string& prompt, int min = 0, int max = 100);
    double getDoubleInput(const std::string& prompt, double min = 0.0, double max = 1000000.0);
    double getDoubleInputWithCancel(const std::string& prompt, double min, double max, bool& cancelled);
    bool confirmAction(const std::string& message);
    void showError(const std::string& message);
    void showSuccess(const std::string& message);
    void showInfo(const std::string& message);
    void showWarning(const std::string& message);
    void pauseScreen();
    void clearScreen();
    void showHeader();
    void showSeparator();
    std::string formatCurrency(double amount);
    std::string formatDateTime(const std::chrono::system_clock::time_point& timePoint);
    std::string formatDate(const std::chrono::system_clock::time_point& timePoint);
    std::string formatFileSize(size_t size);
    void displayUserInfo(const User& user);
    void displayTransactionTable(const std::vector<Transaction>& transactions);
    int showMenuSelection(const std::string& title, const std::vector<std::string>& options);

    template<typename Validator>
    std::string getValidatedInput(const std::string& prompt, Validator validator, const std::string& errorMsg = "", bool isSensitive = false, int maxAttempts = 3);
};

#endif
