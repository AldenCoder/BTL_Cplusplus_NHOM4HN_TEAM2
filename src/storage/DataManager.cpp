/**
 * @file DataManager.cpp
 * @brief Implementation của DataManager
 * @author Team 2C
 */

#include "DataManager.h"
#include "../security/SecurityUtils.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

DataManager::DataManager(const std::string& dataDir) 
    : dataDirectory(dataDir), 
      backupDirectory(dataDir + "/backup"),
      usersFile(dataDir + "/users.json"),
      walletsFile(dataDir + "/wallets.json") {
}

// Helper functions for file operations without filesystem
namespace {
    bool fileExists(const std::string& path) {
        std::ifstream file(path);
        return file.good();
    }
    
    size_t getFileSize(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.good()) return 0;
        return static_cast<size_t>(file.tellg());
    }
    
    bool removeFile(const std::string& path) {
        return std::remove(path.c_str()) == 0;
    }
}

bool DataManager::initialize() {
    // Tạo thư mục nếu chưa tồn tại
    if (!createDirectoryIfNotExists(dataDirectory)) {
        return false;
    }
    
    if (!createDirectoryIfNotExists(backupDirectory)) {
        return false;
    }
      // Tạo file rỗng nếu chưa tồn tại
    if (!fileExists(usersFile)) {
        writeJsonFile(usersFile, "{}");
    }
    
    if (!fileExists(walletsFile)) {
        writeJsonFile(walletsFile, "{}");
    }
    
    // Tải lịch sử backup
    loadBackupHistory();
    
    // Kiểm tra và tạo backup tự động
    checkAutoBackup();
    
    return true;
}

// ==================== USER DATA MANAGEMENT ====================

bool DataManager::saveUser(const User& user) {
    auto users = loadAllUsers();
    bool found = false;
    for (auto& existingUser : users) {
        if (existingUser->getUsername() == user.getUsername()) {
            *existingUser = user;
            found = true;
            break;
        }
    }
    
    if (!found) {
        users.push_back(std::make_unique<User>(user));
    }

    std::stringstream json;
    json << "{\n  \"users\": [\n";
    
    for (size_t i = 0; i < users.size(); ++i) {
        json << "    " << users[i]->toJson();
        if (i < users.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ]\n}";

    return writeJsonFile(usersFile, json.str());
}

std::unique_ptr<User> DataManager::loadUser(const std::string& username) {
    try {
        std::string content = readJsonFile(usersFile);
        if (content.empty()) {
            return nullptr;
        }
        
        // Parse JSON content to find users array
        size_t usersPos = content.find("\"users\":");
        if (usersPos == std::string::npos) {
            return nullptr;
        }
        
        // Find the user with matching username
        size_t pos = usersPos;
        while ((pos = content.find("\"username\":", pos)) != std::string::npos) {
            size_t valueStart = content.find("\"", pos + 11);
            size_t valueEnd = content.find("\"", valueStart + 1);
            
            if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                std::string foundUsername = content.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                if (foundUsername == username) {
                    // Find the start and end of this user object
                    size_t objStart = content.rfind("{", pos);
                    
                    // Find the complete object (handle nested braces)
                    int braceCount = 1;
                    size_t searchPos = objStart + 1;
                    while (searchPos < content.length() && braceCount > 0) {
                        if (content[searchPos] == '{') braceCount++;
                        else if (content[searchPos] == '}') braceCount--;
                        searchPos++;
                    }
                    size_t objEnd = searchPos - 1;
                    
                    if (objStart != std::string::npos && objEnd != std::string::npos) {
                        std::string userJson = content.substr(objStart, objEnd - objStart + 1);
                        return User::fromJson(userJson);
                    }
                }
            }
            pos = valueEnd;
        }
        
        return nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading user: " << e.what() << std::endl;
        return nullptr;
    }
}

std::vector<std::unique_ptr<User>> DataManager::loadAllUsers() {
    std::vector<std::unique_ptr<User>> users;
    
    try {
        std::string content = readJsonFile(usersFile);
        if (content.empty()) {
            return users;
        }
        
        // Parse JSON content to find users array
        size_t usersPos = content.find("\"users\":");
        if (usersPos == std::string::npos) {
            return users;
        }
        
        // Find all user objects
        size_t pos = usersPos;
        while ((pos = content.find("\"userId\":", pos)) != std::string::npos) {
            // Find the start and end of this user object
            size_t objStart = content.rfind("{", pos);
            
            // Find the complete object (handle nested braces)
            int braceCount = 1;
            size_t searchPos = objStart + 1;
            while (searchPos < content.length() && braceCount > 0) {
                if (content[searchPos] == '{') braceCount++;
                else if (content[searchPos] == '}') braceCount--;
                searchPos++;
            }
            size_t objEnd = searchPos - 1;
            
            if (objStart != std::string::npos && objEnd != std::string::npos) {
                std::string userJson = content.substr(objStart, objEnd - objStart + 1);
                auto user = User::fromJson(userJson);
                if (user) {
                    users.push_back(std::move(user));
                }
            }
            
            pos = objEnd;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading all users: " << e.what() << std::endl;
    }
      
    return users;
}

// Additional methods for interface compatibility
std::unique_ptr<User> DataManager::loadUserByUsername(const std::string& username) {
    return loadUser(username);  // Delegate to existing method
}

std::unique_ptr<User> DataManager::loadUserById(const std::string& userId) {
    try {
        std::string content = readJsonFile(usersFile);
        if (content.empty()) {
            return nullptr;
        }
        
        // Parse JSON content to find users array
        size_t usersPos = content.find("\"users\":");
        if (usersPos == std::string::npos) {
            return nullptr;
        }
        
        // Find the user with matching userId
        size_t pos = usersPos;
        while ((pos = content.find("\"userId\":", pos)) != std::string::npos) {
            size_t valueStart = content.find("\"", pos + 9);
            size_t valueEnd = content.find("\"", valueStart + 1);
            
            if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                std::string foundUserId = content.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                if (foundUserId == userId) {
                    // Find the start and end of this user object
                    size_t objStart = content.rfind("{", pos);
                    
                    // Find the complete object (handle nested braces)
                    int braceCount = 1;
                    size_t searchPos = objStart + 1;
                    while (searchPos < content.length() && braceCount > 0) {
                        if (content[searchPos] == '{') braceCount++;
                        else if (content[searchPos] == '}') braceCount--;
                        searchPos++;
                    }
                    size_t objEnd = searchPos - 1;
                    
                    if (objStart != std::string::npos && objEnd != std::string::npos) {
                        std::string userJson = content.substr(objStart, objEnd - objStart + 1);
                        return User::fromJson(userJson);
                    }
                }
            }
            pos = valueEnd;
        }
        
        return nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading user by ID: " << e.what() << std::endl;
        return nullptr;
    }
}

bool DataManager::saveUser(std::shared_ptr<User> user) {
    if (!user) {
        return false;
    }
    return saveUser(*user);  // Delegate to existing method
}

bool DataManager::deleteUser(const std::string& username) {
    try {
        // Đọc tất cả dữ liệu
        std::vector<std::string> lines;
        std::ifstream file(usersFile);
        if (!file.is_open()) {
            return false;
        }
        
        std::string line;
        bool found = false;
        while (std::getline(file, line)) {
            size_t delimPos = line.find('|');
            if (delimPos != std::string::npos) {
                std::string fileUsername = line.substr(0, delimPos);
                if (fileUsername != username) {
                    lines.push_back(line);
                } else {
                    found = true;
                }
            }
        }
        file.close();
        
        if (!found) {
            return false;
        }
        
        // Ghi lại file không có user đã xóa
        std::ofstream outFile(usersFile);
        if (!outFile.is_open()) {
            return false;
        }
        
        for (const auto& l : lines) {
            outFile << l << "\n";
        }
        
        outFile.close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Lỗi xóa user: " << e.what() << std::endl;
        return false;
    }
}

bool DataManager::userExists(const std::string& username) {
    auto user = loadUser(username);
    return user != nullptr;
}

// ==================== WALLET DATA MANAGEMENT ====================

bool DataManager::saveWallet(const Wallet& wallet) {
    auto wallets = loadAllWallets();
    
    bool found = false;
    for (auto& existingWallet : wallets) {
        if (existingWallet->getWalletId() == wallet.getWalletId()) {
            *existingWallet = wallet;
            found = true;
            break;
        }
    }
    
    if (!found) {
        wallets.push_back(std::make_unique<Wallet>(wallet));
    }

    // JSON
    std::stringstream json;
    json << "{\n  \"wallets\": [\n";
    
    for (size_t i = 0; i < wallets.size(); ++i) {
        json << "    " << wallets[i]->toJson();
        if (i < wallets.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ]\n}";

    return writeJsonFile(walletsFile, json.str());
}

std::unique_ptr<Wallet> DataManager::loadWallet(const std::string& walletId) {
    try {
        std::string content = readJsonFile(walletsFile);
        if (content.empty()) {
            return nullptr;
        }
        
        // Parse JSON content to find wallets array
        size_t walletsPos = content.find("\"wallets\":");
        if (walletsPos == std::string::npos) {
            return nullptr;
        }
        
        // Find the wallet with matching walletId
        size_t pos = walletsPos;
        while ((pos = content.find("\"walletId\":", pos)) != std::string::npos) {
            size_t valueStart = content.find("\"", pos + 11);
            size_t valueEnd = content.find("\"", valueStart + 1);
            
            if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                std::string foundWalletId = content.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                if (foundWalletId == walletId) {
                    // Find the start and end of this wallet object
                    size_t objStart = content.rfind("{", pos);
                    size_t objEnd = content.find("}", pos);
                    
                    // Find the complete object (handle nested braces)
                    int braceCount = 1;
                    size_t searchPos = objStart + 1;
                    while (searchPos < content.length() && braceCount > 0) {
                        if (content[searchPos] == '{') braceCount++;
                        else if (content[searchPos] == '}') braceCount--;
                        searchPos++;
                    }
                    objEnd = searchPos - 1;
                    
                    if (objStart != std::string::npos && objEnd != std::string::npos) {
                        std::string walletJson = content.substr(objStart, objEnd - objStart + 1);
                        return Wallet::fromJson(walletJson);
                    }
                }
            }
            pos = valueEnd;
        }
        
        return nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading wallet: " << e.what() << std::endl;
        return nullptr;
    }
}

std::vector<std::unique_ptr<Wallet>> DataManager::loadAllWallets() {
    std::vector<std::unique_ptr<Wallet>> wallets;
    
    try {
        std::string content = readJsonFile(walletsFile);
        if (content.empty()) {
            return wallets;
        }
        
        // Parse JSON content to find wallets array
        size_t walletsPos = content.find("\"wallets\":");
        if (walletsPos == std::string::npos) {
            return wallets;
        }
        
        // Find all wallet objects
        size_t pos = walletsPos;
        while ((pos = content.find("\"walletId\":", pos)) != std::string::npos) {
            // Find the start and end of this wallet object
            size_t objStart = content.rfind("{", pos);
            
            // Find the complete object (handle nested braces)
            int braceCount = 1;
            size_t searchPos = objStart + 1;
            while (searchPos < content.length() && braceCount > 0) {
                if (content[searchPos] == '{') braceCount++;
                else if (content[searchPos] == '}') braceCount--;
                searchPos++;
            }
            size_t objEnd = searchPos - 1;
            
            if (objStart != std::string::npos && objEnd != std::string::npos) {
                std::string walletJson = content.substr(objStart, objEnd - objStart + 1);
                auto wallet = Wallet::fromJson(walletJson);
                if (wallet) {
                    wallets.push_back(std::move(wallet));
                }
            }
            
            pos = objEnd;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading all wallets: " << e.what() << std::endl;
    }
    
    return wallets;
}

std::unique_ptr<Wallet> DataManager::loadWalletByOwner(const std::string& ownerId) {
    // Trong implementation đầy đủ sẽ tìm wallet theo ownerId
    // Ở đây trả về nullptr tạm thời
    return nullptr;
}

bool DataManager::walletExists(const std::string& walletId) {
    auto wallet = loadWallet(walletId);
    return wallet != nullptr;
}

bool DataManager::saveWallet(std::shared_ptr<Wallet> wallet) {
    if (!wallet) {
        return false;
    }
    return saveWallet(*wallet);
}

std::shared_ptr<Wallet> DataManager::loadWalletById(const std::string& walletId) {
    auto unique_wallet = loadWallet(walletId);
    if (unique_wallet) {
        return std::shared_ptr<Wallet>(unique_wallet.release());
    }
    return nullptr;
}

std::shared_ptr<Wallet> DataManager::loadWalletByUserId(const std::string& userId) {
    // Đọc file wallets.json
    std::string content = readJsonFile(walletsFile);
    if (content.empty()) {
        // File không tồn tại hoặc rỗng
        return nullptr;
    }
    
    // Parse JSON
    size_t pos = content.find("\"wallets\":");
    if (pos == std::string::npos) {
        return nullptr;
    }
    
    // Tìm ví theo userId
    pos = 0;
    while ((pos = content.find("\"ownerId\":", pos)) != std::string::npos) {
        size_t endPos = content.find("\n", pos);
        std::string line = content.substr(pos, endPos - pos);
        
        size_t valuePos = line.find(":", 0);
        if (valuePos != std::string::npos) {
            std::string value = line.substr(valuePos + 1);
            value.erase(0, value.find_first_not_of(" \t\r\""));
            value.erase(value.find_last_not_of(" \t\r\",") + 1);
            
            if (value == userId) {
                // Tìm walletId trong đoạn JSON này
                size_t walletIdPos = content.rfind("\"walletId\":", pos);
                if (walletIdPos != std::string::npos) {
                    // Extract wallet ID
                    std::string walletLine = content.substr(walletIdPos, content.find("\n", walletIdPos) - walletIdPos);
                    size_t idValuePos = walletLine.find(":", 0);
                    if (idValuePos != std::string::npos) {
                        std::string walletId = walletLine.substr(idValuePos + 1);
                        walletId.erase(0, walletId.find_first_not_of(" \t\r\""));
                        walletId.erase(walletId.find_last_not_of(" \t\r\",") + 1);
                        
                        return loadWalletById(walletId);
                    }
                }
                break;
            }
        }
        pos = endPos;
    }
    
    return nullptr;
}

std::shared_ptr<Wallet> DataManager::loadWalletByOwner_shared(const std::string& ownerId) {
    try {
        std::string content = readJsonFile(walletsFile);
        if (content.empty()) {
            return nullptr;
        }
        
        // Parse JSON content to find wallets array
        size_t walletsPos = content.find("\"wallets\":");
        if (walletsPos == std::string::npos) {
            return nullptr;
        }
        
        // Find the wallet with matching ownerId
        size_t pos = walletsPos;
        while ((pos = content.find("\"ownerId\":", pos)) != std::string::npos) {
            size_t valueStart = content.find("\"", pos + 10);
            size_t valueEnd = content.find("\"", valueStart + 1);
            
            if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                std::string foundOwnerId = content.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                if (foundOwnerId == ownerId) {
                    // Find the start and end of this wallet object
                    size_t objStart = content.rfind("{", pos);
                    
                    // Find the complete object (handle nested braces)
                    int braceCount = 1;
                    size_t searchPos = objStart + 1;
                    while (searchPos < content.length() && braceCount > 0) {
                        if (content[searchPos] == '{') braceCount++;
                        else if (content[searchPos] == '}') braceCount--;
                        searchPos++;
                    }
                    size_t objEnd = searchPos - 1;
                    
                    if (objStart != std::string::npos && objEnd != std::string::npos) {
                        std::string walletJson = content.substr(objStart, objEnd - objStart + 1);
                        auto unique_wallet = Wallet::fromJson(walletJson);
                        if (unique_wallet) {
                            return std::shared_ptr<Wallet>(unique_wallet.release());
                        }
                    }
                }
            }
            pos = valueEnd;
        }
        
        return nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading wallet by owner: " << e.what() << std::endl;
        return nullptr;
    }
}

// ==================== BACKUP & RECOVERY ====================

BackupInfo DataManager::createBackup(BackupType type, const std::string& description) {
    BackupInfo backupInfo;
    backupInfo.backupId = generateBackupId();
    backupInfo.type = type;
    backupInfo.timestamp = std::chrono::system_clock::now();
    
    // Tạo tên file backup
    auto time_t = std::chrono::system_clock::to_time_t(backupInfo.timestamp);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    backupInfo.filename = "backup_" + ss.str() + ".zip";
    
    try {
        std::string backupPath = backupDirectory + "/" + backupInfo.filename;
        
        // Nén thư mục dữ liệu (đơn giản hóa)
        bool success = compressDirectory(dataDirectory, backupPath);
        
        if (success) {
            // Tính checksum
            backupInfo.checksum = calculateChecksum(backupPath);
            backupInfo.fileSize = getFileSize(backupPath);
            
            // Lưu vào lịch sử
            backupHistory.push_back(backupInfo);
            saveBackupHistory();
            
            // Dọn dẹp backup cũ            cleanupOldBackups();
            
            std::cout << "Backup created successfully: " << backupInfo.filename << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Backup creation error: " << e.what() << std::endl;
        backupInfo.backupId = ""; // Mark as failed
    }
    
    return backupInfo;
}

bool DataManager::restoreFromBackup(const std::string& backupId) {
    // Tìm backup info
    auto it = std::find_if(backupHistory.begin(), backupHistory.end(),
        [&backupId](const BackupInfo& info) {
            return info.backupId == backupId;
        });
    
    if (it == backupHistory.end()) {
        return false;
    }
    
    try {
        std::string backupPath = backupDirectory + "/" + it->filename;
        
        // Kiểm tra file tồn tại
        if (!fileExists(backupPath)) {
            return false;
        }
        
        // Kiểm tra checksum
        std::string currentChecksum = calculateChecksum(backupPath);
        if (currentChecksum != it->checksum) {
            std::cerr << "Lỗi: Backup file bị hỏng (checksum không khớp)" << std::endl;
            return false;
        }
        
        // Backup dữ liệu hiện tại trước khi restore
        createBackup(BackupType::EMERGENCY, "Before restore");
        
        // Giải nén và phục hồi
        return extractBackup(backupPath, dataDirectory);
        
    } catch (const std::exception& e) {
        std::cerr << "Lỗi phục hồi backup: " << e.what() << std::endl;
        return false;
    }
}

void DataManager::cleanupOldBackups() {
    if (backupHistory.size() <= MAX_BACKUP_COUNT) {
        return;
    }
    
    // Sắp xếp theo thời gian
    std::sort(backupHistory.begin(), backupHistory.end(),
        [](const BackupInfo& a, const BackupInfo& b) {
            return a.timestamp < b.timestamp;
        });
    
    // Xóa các backup cũ nhất
    while (backupHistory.size() > MAX_BACKUP_COUNT) {
        const auto& oldestBackup = backupHistory.front();
        
        // Xóa file
        std::string backupPath = backupDirectory + "/" + oldestBackup.filename;
        try {            if (fileExists(backupPath)) {
                removeFile(backupPath);
            }
        } catch (const std::exception& e) {
            std::cerr << "Lỗi xóa backup cũ: " << e.what() << std::endl;
        }
        
        // Xóa khỏi lịch sử
        backupHistory.erase(backupHistory.begin());
    }
    
    saveBackupHistory();
}

void DataManager::checkAutoBackup() {
    // Kiểm tra backup cuối cùng
    if (backupHistory.empty()) {
        createBackup(BackupType::AUTO, "Initial auto backup");
        return;
    }
    
    auto lastBackup = std::max_element(backupHistory.begin(), backupHistory.end(),
        [](const BackupInfo& a, const BackupInfo& b) {
            return a.timestamp < b.timestamp;
        });
    
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastBackup = std::chrono::duration_cast<std::chrono::hours>(
        now - lastBackup->timestamp);
    
    if (timeSinceLastBackup.count() >= AUTO_BACKUP_INTERVAL_HOURS) {
        createBackup(BackupType::AUTO, "Scheduled auto backup");
    }
}

bool DataManager::verifyDataIntegrity() {
    try {
        // Kiểm tra file users tồn tại và đọc được
        if (!fileExists(usersFile)) {
            return false;
        }
        
        std::ifstream userFile(usersFile);
        if (!userFile.is_open()) {
            return false;
        }
        userFile.close();
        
        // Kiểm tra file wallets
        if (!fileExists(walletsFile)) {
            return false;
        }
        
        std::ifstream walletFile(walletsFile);
        if (!walletFile.is_open()) {
            return false;
        }
        walletFile.close();
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Lỗi kiểm tra tính toàn vẹn: " << e.what() << std::endl;
        return false;
    }
}

// ==================== UTILITY FUNCTIONS ====================

std::string DataManager::getDataStatistics() {
    std::ostringstream stats;
    
    try {
        auto users = loadAllUsers();
        auto wallets = loadAllWallets();
        
        stats << "=== THỐNG KÊ DỮ LIỆU ===\n";
        stats << "Số lượng người dùng: " << users.size() << "\n";
        stats << "Số lượng ví: " << wallets.size() << "\n";
        stats << "Số lượng backup: " << backupHistory.size() << "\n";
        
        // Tính tổng số dư
        double totalBalance = 0.0;
        for (const auto& wallet : wallets) {
            totalBalance += wallet->getBalance();
        }        stats << "Tổng số điểm trong hệ thống: " << totalBalance << "\n";
        
        // Kích thước file
        if (fileExists(usersFile)) {
            auto userFileSize = getFileSize(usersFile);
            stats << "Kích thước file users: " << userFileSize << " bytes\n";
        }
        if (fileExists(walletsFile)) {
            auto walletFileSize = getFileSize(walletsFile);
            stats << "Kích thước file wallets: " << walletFileSize << " bytes\n";
        }
        
        stats << "=======================\n";
        
    } catch (const std::exception& e) {
        stats << "Lỗi tạo thống kê: " << e.what() << "\n";
    }
    
    return stats.str();
}

void DataManager::cleanup() {
    // Dọn dẹp OTP hết hạn
    SecurityUtils::cleanupExpiredOTP();
    
    // Dọn dẹp backup cũ
    cleanupOldBackups();
    
    std::cout << "Temporary data cleaned up" << std::endl;
}

bool DataManager::exportToCSV(const std::string& outputPath) {
    try {
        std::ofstream csvFile(outputPath);
        if (!csvFile.is_open()) {
            return false;
        }
        
        // Export users
        csvFile << "=== USERS ===\n";
        csvFile << "Username,FullName,Email,Phone,Role,CreatedAt\n";
        
        auto users = loadAllUsers();
        for (const auto& user : users) {
            csvFile << user->getUsername() << ","
                   << user->getFullName() << ","
                   << user->getEmail() << ","
                   << user->getPhoneNumber() << ","
                   << static_cast<int>(user->getRole()) << ","
                   << "timestamp" << "\n"; // Sẽ format thời gian trong thực tế
        }
        
        // Export wallets
        csvFile << "\n=== WALLETS ===\n";
        csvFile << "WalletId,OwnerId,Balance,IsLocked\n";
        
        auto wallets = loadAllWallets();
        for (const auto& wallet : wallets) {
            csvFile << wallet->getWalletId() << ","
                   << wallet->getOwnerId() << ","
                   << wallet->getBalance() << ","
                   << (wallet->getIsLocked() ? "true" : "false") << "\n";
        }
        
        csvFile.close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Lỗi export CSV: " << e.what() << std::endl;
        return false;
    }
}

// ==================== PRIVATE HELPER FUNCTIONS ====================

bool DataManager::createDirectoryIfNotExists(const std::string& path) {
    try {
        // Simple directory creation using system calls
        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            // Directory doesn't exist, try to create it
#ifdef _WIN32
            return _mkdir(path.c_str()) == 0;
#else
            return mkdir(path.c_str(), 0755) == 0;
#endif
        }
        return S_ISDIR(st.st_mode); // Check if it's actually a directory
    } catch (const std::exception& e) {
        std::cerr << "Lỗi tạo thư mục " << path << ": " << e.what() << std::endl;
        return false;
    }
}

std::string DataManager::readJsonFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return "";
        }
        
        std::ostringstream ss;
        ss << file.rdbuf();
        file.close();
        
        return ss.str();
    } catch (const std::exception& e) {
        std::cerr << "Lỗi đọc file " << filepath << ": " << e.what() << std::endl;
        return "";
    }
}

bool DataManager::writeJsonFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

std::string DataManager::calculateChecksum(const std::string& filepath) {
    try {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        
        std::ostringstream ss;
        ss << file.rdbuf();
        file.close();
        
        // Dùng SHA256 để tính checksum
        return SecurityUtils::sha256(ss.str());
        
    } catch (const std::exception& e) {
        std::cerr << "Lỗi tính checksum: " << e.what() << std::endl;
        return "";
    }
}

bool DataManager::compressDirectory(const std::string& sourceDir, const std::string& targetFile) {
    // Simplified implementation for demo - just create a placeholder file
    try {
        std::ofstream backup(targetFile);
        backup << "Backup placeholder for: " << sourceDir << std::endl;
        backup.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Lỗi tạo backup: " << e.what() << std::endl;
        return false;
    }
}

bool DataManager::extractBackup(const std::string& backupFile, const std::string& targetDir) {
    // Simplified implementation for demo
    try {
        std::ifstream backup(backupFile);
        if (!backup.good()) {
            return false;
        }
        backup.close();
        // For demo, just indicate success without actual extraction
        std::cout << "Backup extraction simulated for: " << backupFile << " to " << targetDir << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Lỗi giải nén backup: " << e.what() << std::endl;
        return false;
    }
}

void DataManager::loadBackupHistory() {
    try {
        std::string historyFile = backupDirectory + "/backup_history.txt";
        if (!fileExists(historyFile)) {
            return;
        }
        
        std::ifstream file(historyFile);
        if (!file.is_open()) {
            return;
        }
        
        // Load backup history (implementation đơn giản)
        // Trong thực tế sẽ parse JSON
        
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Lỗi tải lịch sử backup: " << e.what() << std::endl;
    }
}

void DataManager::saveBackupHistory() {
    try {
        std::string historyFile = backupDirectory + "/backup_history.txt";
        std::ofstream file(historyFile);
        if (!file.is_open()) {
            return;
        }
        
        // Save backup history (implementation đơn giản)
        for (const auto& backup : backupHistory) {
            file << backup.backupId << "|" << backup.filename << "|" 
                 << static_cast<int>(backup.type) << "\n";
        }
        
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Lỗi lưu lịch sử backup: " << e.what() << std::endl;
    }
}

std::string DataManager::generateBackupId() {
    return "BACKUP_" + SecurityUtils::generateUUID().substr(0, 8);
}
