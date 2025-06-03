#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "../system/AuthSystem.h"
#include "../system/WalletManager.h"
#include "../storage/DatabaseManager.h"
#include <iostream>
#include <string>
#include <memory>

/**
 * @class UserInterface
 * @brief Text-based user interface
 * 
 * Provides console interface for:
 * - Login/Registration
 * - Personal information management
 * - Wallet transactions
 * - View reports
 */
class UserInterface {
private:
    AuthSystem& authSystem;                          // Authentication system
    std::unique_ptr<WalletManager> walletManager;    // Wallet manager
    bool isRunning;                                  // Running status

public:    /**
     * @brief Constructor
     * @param authSys Reference to authentication system
     */
    UserInterface(AuthSystem& authSys);

    /**
     * @brief Destructor
     */
    ~UserInterface();

    /**
     * @brief Run main interface
     */
    void run();

private:
    // ==================== MENU FUNCTIONS ====================
    
    /**
     * @brief Show main menu (not logged in)
     */
    void showMainMenu();

    /**
     * @brief Show regular user menu
     */
    void showUserMenu();

    /**
     * @brief Show admin menu
     */
    void showAdminMenu();

    /**
     * @brief Handle main menu selection
     * @param choice User's choice
     */
    void handleMainMenu(int choice);

    /**
     * @brief Handle user menu selection
     * @param choice User's choice
     */
    void handleUserMenu(int choice);

    /**
     * @brief Handle admin menu selection
     * @param choice Admin's choice
     */
    void handleAdminMenu(int choice);    // ==================== AUTH FUNCTIONS ====================
    
    /**
     * @brief Login screen
     */
    void loginScreen();

    /**
     * @brief Registration screen
     */
    void registerScreen();

    /**
     * @brief Logout
     */
    void logout();

    // ==================== USER PROFILE FUNCTIONS ====================
    
    /**
     * @brief View personal information
     */
    void viewProfile();

    /**
     * @brief Change password
     */
    void changePassword();

    /**
     * @brief Update personal information
     */
    void updateProfile();

    // ==================== WALLET FUNCTIONS ====================
    
    /**
     * @brief View wallet balance
     */
    void viewWalletBalance();

    /**
     * @brief Transfer points
     */
    void transferPoints();

    /**
     * @brief View transaction history
     */
    void viewTransactionHistory();

    /**
     * @brief View wallet report
     */
    void viewWalletReport();    // ==================== ADMIN FUNCTIONS ====================
    
    /**
     * @brief View list of all users
     */
    void viewAllUsers();

    /**
     * @brief Create new account (admin)
     */
    void createNewAccount();

    /**
     * @brief Manage user account information (admin)
     */
    void manageUserAccount();

    /**
     * @brief View system statistics
     */
    void viewSystemStatistics();

    /**
     * @brief Issue points from master wallet
     */
    void issuePointsFromMaster();

    /**
     * @brief Manage data backup
     */
    void manageBackup();

    /**
     * @brief Create manual backup
     */
    void createManualBackup();

    /**
     * @brief View backup history
     */
    void viewBackupHistory();

    /**
     * @brief Restore from backup
     */
    void restoreFromBackup();

    /**
     * @brief Clean up old backups
     */
    void cleanupBackups();    // ==================== UTILITY FUNCTIONS ====================
    
    /**
     * @brief Read input from user
     * @param prompt Prompt message
     * @return Input string
     */
    std::string getInput(const std::string& prompt);

    /**
     * @brief Read password (hide characters)
     * @param prompt Prompt message
     * @return Password string
     */
    std::string getPassword(const std::string& prompt);

    /**
     * @brief Read number from input
     * @param prompt Prompt message
     * @param min Minimum value
     * @param max Maximum value
     * @return Number entered
     */
    int getIntInput(const std::string& prompt, int min = 0, int max = 100);

    /**
     * @brief Read decimal number from input
     * @param prompt Prompt message
     * @param min Minimum value
     * @param max Maximum value
     * @return Decimal number entered
     */
    double getDoubleInput(const std::string& prompt, double min = 0.0, double max = 1000000.0);

    /**
     * @brief Confirm action
     * @param message Confirmation message
     * @return true if user confirms
     */
    bool confirmAction(const std::string& message);

    /**
     * @brief Show error message
     * @param message Error message
     */
    void showError(const std::string& message);

    /**
     * @brief Show success message
     * @param message Success message
     */
    void showSuccess(const std::string& message);    /**
     * @brief Show information
     * @param message Information
     */
    void showInfo(const std::string& message);

    /**
     * @brief Show warning message
     * @param message Warning message
     */
    void showWarning(const std::string& message);

    /**
     * @brief Pause screen and wait for user to press key
     */
    void pauseScreen();

    /**
     * @brief Clear console screen
     */
    void clearScreen();

    /**
     * @brief Show application header
     */
    void showHeader();

    /**
     * @brief Show separator
     */
    void showSeparator();    /**
     * @brief Validate email
     * @param email Email to check
     * @return true if email is valid
     */
    bool isValidEmail(const std::string& email);

    /**
     * @brief Validate phone number
     * @param phone Phone number to check
     * @return true if phone number is valid
     */
    bool isValidPhoneNumber(const std::string& phone);

    /**
     * @brief Format currency display
     * @param amount Amount
     * @return Formatted string
     */
    std::string formatCurrency(double amount);    /**
     * @brief Format time display
     * @param timePoint Time point
     * @return Formatted time string
     */
    std::string formatDateTime(const std::chrono::system_clock::time_point& timePoint);

    /**
     * @brief Format file size display
     * @param size File size in bytes
     * @return Formatted file size string
     */
    std::string formatFileSize(size_t size);

    /**
     * @brief Display user information as table
     * @param user User to display
     */
    void displayUserInfo(const User& user);

    /**
     * @brief Display transaction history as table
     * @param transactions Transaction list
     */
    void displayTransactionTable(const std::vector<Transaction>& transactions);

    /**
     * @brief Show menu selection
     * @param title Menu title
     * @param options Vector of choices
     * @return User's choice (1-based index)
     */
    int showMenuSelection(const std::string& title, const std::vector<std::string>& options);
};

#endif // USER_INTERFACE_H
