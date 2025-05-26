/**
 * @file Wallet.h
 * @brief Định nghĩa lớp Wallet - quản lý ví điểm thưởng
 * @author Team 2C
 */

#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

/**
 * @enum TransactionType
 * @brief Loại giao dịch trong ví
 */
enum class TransactionType {
    TRANSFER_IN,    // Nhận điểm
    TRANSFER_OUT,   // Chuyển điểm
    TRANSFER,       // Giao dịch chuyển điểm chung
    INITIAL,        // Điểm khởi tạo từ ví tổng
    ROLLBACK        // Hoàn tác giao dịch
};

/**
 * @enum TransactionStatus
 * @brief Trạng thái giao dịch
 */
enum class TransactionStatus {
    PENDING,        // Đang chờ xác nhận
    COMPLETED,      // Hoàn thành
    FAILED,         // Thất bại
    CANCELLED       // Đã hủy
};

/**
 * @struct Transaction
 * @brief Cấu trúc dữ liệu giao dịch
 */
struct Transaction {
    std::string transactionId;          // ID giao dịch (UUID)
    std::string fromWalletId;          // ID ví gửi
    std::string toWalletId;            // ID ví nhận
    double amount;                     // Số điểm
    TransactionType type;              // Loại giao dịch
    TransactionStatus status;          // Trạng thái
    std::string description;           // Mô tả giao dịch
    std::chrono::system_clock::time_point timestamp;  // Thời gian giao dịch
    std::string otpUsed;              // OTP đã sử dụng (nếu có)    /**
     * Constructor
     */
    Transaction(const std::string& fromId, const std::string& toId, 
                double amt, TransactionType t, const std::string& desc = "");

    /**
     * Constructor with full parameters
     */
    Transaction(const std::string& id, const std::string& fromId, const std::string& toId,
                double amt, TransactionType t, TransactionStatus stat, const std::string& desc);

    // Getter methods
    std::string getId() const { return transactionId; }
    double getAmount() const { return amount; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }

    /**
     * @brief Chuyển đổi transaction thành JSON
     */
    std::string toJson() const;

    /**
     * @brief Tạo transaction từ JSON
     */
    static Transaction fromJson(const std::string& json);
};

/**
 * @class Wallet
 * @brief Lớp quản lý ví điểm thưởng
 * 
 * Mỗi user có một ví với các chức năng:
 * - Lưu trữ số dư điểm
 * - Thực hiện giao dịch chuyển điểm
 * - Lưu lịch sử giao dịch
 * - Xác thực giao dịch bằng OTP
 */
class Wallet {
protected:  // Changed from private to protected for inheritance
    std::string walletId;              // ID duy nhất của ví
    std::string ownerId;               // ID chủ sở hữu
    double balance;                    // Số dư hiện tại
    std::vector<Transaction> transactions;  // Lịch sử giao dịch
    std::chrono::system_clock::time_point createdAt;  // Thời gian tạo ví
    bool isLocked;                     // Ví có bị khóa không

public:
    /**
     * @brief Constructor tạo ví mới
     * @param walletId ID của ví
     * @param ownerId ID chủ sở hữu
     * @param initialBalance Số dư ban đầu (mặc định 0)
     */
    Wallet(const std::string& walletId, const std::string& ownerId, 
           double initialBalance = 0.0);    // Getter methods
    const std::string& getWalletId() const { return walletId; }
    std::string getId() const { return walletId; } // Alias for getWalletId
    const std::string& getOwnerId() const { return ownerId; }
    double getBalance() const { return balance; }
    const std::vector<Transaction>& getTransactions() const { return transactions; }
    std::vector<Transaction> getTransactionHistory() const { return transactions; } // Overload without parameters
    bool getIsLocked() const { return isLocked; }
    bool isLockedStatus() const { return isLocked; } // Alias for getIsLocked    /**
     * @brief Kiểm tra số dư có đủ để thực hiện giao dịch không
     * @param amount Số điểm cần kiểm tra
     * @return true nếu đủ số dư
     */
    bool hasSufficientBalance(double amount) const;

    /**
     * @brief Nạp điểm vào ví
     * @param amount Số điểm cần nạp
     * @return true nếu thành công
     */
    bool deposit(double amount);

    /**
     * @brief Rút điểm từ ví
     * @param amount Số điểm cần rút
     * @return true nếu thành công
     */
    bool withdraw(double amount);

    /**
     * @brief Đặt ID cho ví (cần thiết cho một số trường hợp)
     * @param newId ID mới
     */
    void setId(const std::string& newId) { walletId = newId; }

    /**
     * @brief Thực hiện giao dịch chuyển điểm đi
     * @param amount Số điểm chuyển
     * @param toWalletId ID ví đích
     * @param description Mô tả giao dịch
     * @return ID giao dịch nếu thành công, empty string nếu thất bại
     */
    std::string transferOut(double amount, const std::string& toWalletId, 
                           const std::string& description = "");

    /**
     * @brief Nhận điểm từ giao dịch
     * @param amount Số điểm nhận
     * @param fromWalletId ID ví gửi
     * @param transactionId ID giao dịch
     * @param description Mô tả
     */
    void receiveTransfer(double amount, const std::string& fromWalletId,
                        const std::string& transactionId, 
                        const std::string& description = "");

    /**
     * @brief Hủy giao dịch (rollback)
     * @param transactionId ID giao dịch cần hủy
     * @return true nếu hủy thành công
     */
    bool cancelTransaction(const std::string& transactionId);

    /**
     * @brief Xác nhận giao dịch
     * @param transactionId ID giao dịch
     * @param otpCode Mã OTP xác thực
     * @return true nếu xác nhận thành công
     */
    bool confirmTransaction(const std::string& transactionId, 
                           const std::string& otpCode);

    /**
     * @brief Lấy lịch sử giao dịch trong khoảng thời gian
     * @param fromDate Từ ngày
     * @param toDate Đến ngày
     * @return Vector các giao dịch trong khoảng thời gian
     */
    std::vector<Transaction> getTransactionHistory(
        const std::chrono::system_clock::time_point& fromDate,
        const std::chrono::system_clock::time_point& toDate) const;    /**
     * @brief Khóa/mở khóa ví
     * @param locked true để khóa, false để mở khóa
     */
    void setLocked(bool locked) { isLocked = locked; }

    /**
     * @brief Thêm giao dịch vào lịch sử (public để WalletManager có thể truy cập)
     * @param transaction Giao dịch cần thêm
     */
    void addTransaction(const Transaction& transaction);

    /**
     * @brief Chuyển đổi ví thành JSON để lưu file
     * @return Chuỗi JSON
     */
    std::string toJson() const;

    /**
     * @brief Tạo ví từ chuỗi JSON
     * @param json Chuỗi JSON
     * @return Unique pointer đến Wallet object
     */
    static std::unique_ptr<Wallet> fromJson(const std::string& json);

private:
    /**
     * @brief Tạo ID giao dịch duy nhất
     * @return UUID string
     */
    std::string generateTransactionId();
};

/**
 * @class MasterWallet
 * @brief Ví tổng - nguồn phát sinh điểm trong hệ thống
 * 
 * Ví đặc biệt có khả năng tạo ra điểm mới và phân phối cho các ví khác
 */
class MasterWallet : public Wallet {
private:
    static const std::string MASTER_WALLET_ID;
    static const std::string MASTER_OWNER_ID;

public:
    /**
     * @brief Constructor tạo ví tổng
     * @param initialSupply Tổng số điểm ban đầu trong hệ thống
     */
    MasterWallet(double initialSupply = 1000000.0);    /**
     * @brief Phát hành điểm mới cho ví
     * @param toWalletId ID ví nhận
     * @param amount Số điểm phát hành
     * @param description Lý do phát hành
     * @return ID giao dịch
     */
    std::string issuePoints(const std::string& toWalletId, double amount,
                           const std::string& description = "Initial points");

    /**
     * @brief Kiểm tra ví tổng có đủ điểm không
     * @param amount Số điểm cần kiểm tra
     * @return true nếu đủ
     */
    bool hasEnoughPoints(double amount) const;

    /**
     * @brief Lấy tổng số điểm trong ví tổng
     * @return Số điểm hiện có
     */
    double getTotalPoints() const;

    /**
     * @brief Chuyển điểm ra khỏi ví tổng (đơn giản)
     * @param amount Số điểm cần chuyển
     * @return true nếu thành công
     */
    bool transferOut(double amount);

    /**
     * @brief Lấy instance duy nhất của ví tổng (Singleton)
     * @return Reference đến ví tổng
     */
    static MasterWallet& getInstance();
};

#endif // WALLET_H
