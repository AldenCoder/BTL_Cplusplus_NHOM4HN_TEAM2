/**
 * @file DataManager.h
 * @brief Quản lý lưu trữ và sao lưu dữ liệu
 * @author Team 2C
 */

#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "../models/User.h"
#include "../models/Wallet.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <chrono>
#include <sys/stat.h>  // For file operations instead of filesystem

/**
 * @enum BackupType
 * @brief Loại sao lưu
 */
enum class BackupType {
    MANUAL,     // Sao lưu thủ công
    AUTO,       // Sao lưu tự động
    EMERGENCY   // Sao lưu khẩn cấp
};

/**
 * @struct BackupInfo
 * @brief Thông tin sao lưu
 */
struct BackupInfo {
    std::string backupId;       // ID bản sao lưu
    std::string filename;       // Tên file sao lưu
    BackupType type;           // Loại sao lưu
    std::chrono::system_clock::time_point timestamp; // Thời gian sao lưu
    size_t fileSize;           // Kích thước file
    std::string checksum;      // Checksum để kiểm tra tính toàn vẹn
};

/**
 * @class DataManager
 * @brief Quản lý dữ liệu, lưu trữ và sao lưu
 * 
 * Chức năng:
 * - Lưu/đọc dữ liệu user và wallet
 * - Sao lưu dữ liệu định kỳ
 * - Phục hồi dữ liệu từ backup
 * - Kiểm tra tính toàn vẹn dữ liệu
 */
class DataManager {
private:
    std::string dataDirectory;      // Thư mục chứa dữ liệu
    std::string backupDirectory;    // Thư mục chứa backup
    std::string usersFile;          // File chứa danh sách users
    std::string walletsFile;        // File chứa danh sách wallets
    
    std::vector<BackupInfo> backupHistory;  // Lịch sử sao lưu
    
    static const int MAX_BACKUP_COUNT = 10;  // Số lượng backup tối đa
    static const int AUTO_BACKUP_INTERVAL_HOURS = 24; // Tự động backup mỗi 24h

public:
    /**
     * @brief Constructor
     * @param dataDir Thư mục dữ liệu (mặc định: "data")
     */
    DataManager(const std::string& dataDir = "data");

    /**
     * @brief Khởi tạo hệ thống lưu trữ
     * @return true nếu thành công
     */
    bool initialize();

    // ==================== USER DATA MANAGEMENT ====================
    
    /**
     * @brief Lưu thông tin user
     * @param user User cần lưu
     * @return true nếu thành công
     */
    bool saveUser(const User& user);    /**
     * @brief Tải thông tin user theo username
     * @param username Tên đăng nhập
     * @return Unique pointer đến User (nullptr nếu không tìm thấy)
     */
    std::unique_ptr<User> loadUser(const std::string& username);

    /**
     * @brief Tải thông tin user theo username (alias for loadUser)
     * @param username Tên đăng nhập
     * @return Unique pointer đến User (nullptr nếu không tìm thấy)
     */
    std::unique_ptr<User> loadUserByUsername(const std::string& username);

    /**
     * @brief Tải thông tin user theo ID
     * @param userId ID người dùng
     * @return Unique pointer đến User (nullptr nếu không tìm thấy)
     */
    std::unique_ptr<User> loadUserById(const std::string& userId);

    /**
     * @brief Lưu user (overload cho shared_ptr)
     * @param user Shared pointer đến User cần lưu
     * @return true nếu thành công
     */
    bool saveUser(std::shared_ptr<User> user);

    /**
     * @brief Tải tất cả users
     * @return Vector chứa tất cả users
     */
    std::vector<std::unique_ptr<User>> loadAllUsers();

    /**
     * @brief Xóa user
     * @param username Tên đăng nhập
     * @return true nếu thành công
     */
    bool deleteUser(const std::string& username);

    /**
     * @brief Kiểm tra user có tồn tại không
     * @param username Tên đăng nhập
     * @return true nếu tồn tại
     */
    bool userExists(const std::string& username);

    // ==================== WALLET DATA MANAGEMENT ====================
      /**
     * @brief Lưu thông tin wallet
     * @param wallet Wallet cần lưu
     * @return true nếu thành công
     */
    bool saveWallet(const Wallet& wallet);

    /**
     * @brief Lưu thông tin wallet từ shared_ptr
     * @param wallet Shared pointer đến Wallet cần lưu
     * @return true nếu thành công
     */
    bool saveWallet(std::shared_ptr<Wallet> wallet);

    /**
     * @brief Tải wallet theo ID
     * @param walletId ID ví
     * @return Unique pointer đến Wallet (nullptr nếu không tìm thấy)
     */
    std::unique_ptr<Wallet> loadWallet(const std::string& walletId);

    /**
     * @brief Tải wallet theo ID (trả về shared_ptr)
     * @param walletId ID ví
     * @return Shared pointer đến Wallet
     */
    std::shared_ptr<Wallet> loadWalletById(const std::string& walletId);

    /**
     * @brief Tải wallet theo user ID
     * @param userId ID người dùng
     * @return Shared pointer đến Wallet
     */
    std::shared_ptr<Wallet> loadWalletByUserId(const std::string& userId);

    /**
     * @brief Tải tất cả wallets
     * @return Vector chứa tất cả wallets
     */
    std::vector<std::unique_ptr<Wallet>> loadAllWallets();

    /**
     * @brief Tải wallet theo owner ID
     * @param ownerId ID chủ sở hữu
     * @return Unique pointer đến Wallet
     */
    std::unique_ptr<Wallet> loadWalletByOwner(const std::string& ownerId);

    /**
     * @brief Kiểm tra wallet có tồn tại không
     * @param walletId ID ví
     * @return true nếu tồn tại
     */
    bool walletExists(const std::string& walletId);

private:
    // ==================== HELPER METHODS ====================
    
    /**
     * @brief Helper method để load wallet by owner với shared_ptr
     */
    std::shared_ptr<Wallet> loadWalletByOwner_shared(const std::string& ownerId);

    // ==================== BACKUP & RECOVERY ====================
    
    /**
     * @brief Tạo bản sao lưu
     * @param type Loại sao lưu
     * @param description Mô tả (tùy chọn)
     * @return BackupInfo của bản sao lưu được tạo
     */
    BackupInfo createBackup(BackupType type, const std::string& description = "");

    /**
     * @brief Phục hồi từ bản sao lưu
     * @param backupId ID bản sao lưu
     * @return true nếu phục hồi thành công
     */
    bool restoreFromBackup(const std::string& backupId);

    /**
     * @brief Lấy danh sách các bản sao lưu
     * @return Vector chứa thông tin các bản backup
     */
    std::vector<BackupInfo> getBackupHistory() const { return backupHistory; }

    /**
     * @brief Xóa bản sao lưu cũ (giữ lại số lượng tối đa)
     */
    void cleanupOldBackups();

    /**
     * @brief Kiểm tra và tạo sao lưu tự động nếu cần
     */
    void checkAutoBackup();

    /**
     * @brief Xác thực tính toàn vẹn dữ liệu
     * @return true nếu dữ liệu toàn vẹn
     */
    bool verifyDataIntegrity();

    // ==================== UTILITY FUNCTIONS ====================
    
    /**
     * @brief Lấy thống kê dữ liệu
     * @return String chứa thông tin thống kê
     */
    std::string getDataStatistics();

    /**
     * @brief Dọn dẹp file tạm và dữ liệu không cần thiết
     */
    void cleanup();

    /**
     * @brief Xuất dữ liệu ra file CSV (cho báo cáo)
     * @param outputPath Đường dẫn file xuất
     * @return true nếu thành công
     */
    bool exportToCSV(const std::string& outputPath);

private:
    /**
     * @brief Tạo thư mục nếu chưa tồn tại
     * @param path Đường dẫn thư mục
     * @return true nếu thành công
     */
    bool createDirectoryIfNotExists(const std::string& path);

    /**
     * @brief Đọc file JSON
     * @param filepath Đường dẫn file
     * @return Nội dung file (rỗng nếu lỗi)
     */
    std::string readJsonFile(const std::string& filepath);

    /**
     * @brief Ghi file JSON
     * @param filepath Đường dẫn file
     * @param content Nội dung JSON
     * @return true nếu thành công
     */
    bool writeJsonFile(const std::string& filepath, const std::string& content);

    /**
     * @brief Tính checksum của file
     * @param filepath Đường dẫn file
     * @return Chuỗi checksum
     */
    std::string calculateChecksum(const std::string& filepath);

    /**
     * @brief Nén thư mục dữ liệu
     * @param sourceDir Thư mục nguồn
     * @param targetFile File nén đích
     * @return true nếu thành công
     */
    bool compressDirectory(const std::string& sourceDir, const std::string& targetFile);

    /**
     * @brief Giải nén file backup
     * @param backupFile File backup
     * @param targetDir Thư mục đích
     * @return true nếu thành công
     */
    bool extractBackup(const std::string& backupFile, const std::string& targetDir);

    /**
     * @brief Tải lịch sử backup từ file
     */
    void loadBackupHistory();

    /**
     * @brief Lưu lịch sử backup vào file
     */
    void saveBackupHistory();

    /**
     * @brief Tạo ID backup duy nhất
     * @return Chuỗi ID
     */
    std::string generateBackupId();
};

#endif // DATA_MANAGER_H
