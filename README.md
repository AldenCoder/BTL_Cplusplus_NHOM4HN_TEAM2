# Hệ thống Đăng nhập và Quản lý Ví Điểm Thưởng

## Giới thiệu dự án

Dự án **Hệ thống Đăng nhập và Quản lý Ví Điểm Thưởng** là một ứng dụng C++ được phát triển để quản lý tài khoản người dùng và thực hiện các giao dịch chuyển đổi điểm thưởng giữa các ví điện tử.

### Tính năng chính:
- 🔐 **Hệ thống xác thực an toàn**: Đăng ký, đăng nhập với mật khẩu được băm SHA256
- 👥 **Quản lý người dùng**: Phân quyền user thường và admin
- 💰 **Quản lý ví điểm thưởng**: Chuyển điểm, xem lịch sử giao dịch
- 🔒 **Xác thực 2 lớp**: Sử dụng OTP cho các giao dịch quan trọng
- 💾 **Lưu trữ dữ liệu**: File-based storage với hệ thống backup tự động
- 🔧 **Giao diện console**: Dễ sử dụng với menu điều hướng rõ ràng

## Thành viên nhóm và phân công công việc

| STT | Họ tên | MSSV | Công việc được giao |
|-----|--------|------|-------------------|
| 1 | [Tên thành viên 1] | [MSSV] | **System Architecture & Security**: Thiết kế kiến trúc hệ thống, implement SecurityUtils, OTP system |
| 2 | [Tên thành viên 2] | [MSSV] | **User Management**: Implement User class, AuthSystem, user authentication |
| 3 | [Tên thành viên 3] | [MSSV] | **Wallet System**: Implement Wallet class, WalletManager, transaction processing |
| 4 | [Tên thành viên 4] | [MSSV] | **Data Management & UI**: Implement DataManager, UserInterface, backup system |

## Đặc tả chức năng

### A. Quản lý tài khoản người dùng

#### 1. Đăng ký tài khoản
- **Input**: Username, mật khẩu, họ tên, email, số điện thoại
- **Process**: 
  - Validate thông tin đầu vào
  - Kiểm tra username trùng lặp
  - Băm mật khẩu bằng SHA256 + salt
  - Tạo ví điểm thưởng tự động
- **Output**: Thông báo thành công/thất bại

#### 2. Đăng nhập
- **Input**: Username, mật khẩu
- **Process**: 
  - Tìm user trong database
  - Verify mật khẩu
  - Tạo session
- **Output**: Chuyển đến menu tương ứng với quyền user

#### 3. Quản lý thông tin cá nhân
- **Chức năng**: Xem, cập nhật thông tin (tên, email, SĐT)
- **Bảo mật**: Yêu cầu OTP khi thay đổi thông tin quan trọng

### B. Hệ thống phân quyền

#### 1. User thường
- Quản lý thông tin cá nhân
- Xem số dư ví
- Chuyển điểm cho user khác
- Xem lịch sử giao dịch

#### 2. Admin
- Tất cả chức năng của user thường
- Tạo tài khoản cho user khác
- Xem danh sách tất cả user
- Phát hành điểm từ ví tổng
- Quản lý backup dữ liệu

### C. Quản lý ví điểm thưởng

#### 1. Cấu trúc ví
```cpp
class Wallet {
    string walletId;        // ID duy nhất
    string ownerId;         // ID chủ sở hữu  
    double balance;         // Số dư hiện tại
    vector<Transaction> transactions; // Lịch sử giao dịch
};
```

#### 2. Giao dịch chuyển điểm
- **Quy trình ACID**: Đảm bảo tính toàn vẹn dữ liệu
- **Xác thực OTP**: Bắt buộc cho mọi giao dịch
- **Rollback**: Tự động khôi phục khi có lỗi

#### 3. Ví tổng (Master Wallet)
- Nguồn phát sinh điểm duy nhất trong hệ thống
- Chỉ admin có quyền phát hành điểm

### D. Hệ thống bảo mật

#### 1. Mã hóa mật khẩu
- **Thuật toán**: SHA256 với salt ngẫu nhiên
- **Format lưu trữ**: `salt$hash`

#### 2. OTP (One-Time Password)
- **Độ dài**: 6 chữ số
- **Thời gian hiệu lực**: 5 phút
- **Mục đích**: 
  - Xác thực giao dịch chuyển điểm
  - Xác thực thay đổi thông tin cá nhân

#### 3. Session Management
- Quản lý phiên đăng nhập
- Tự động đăng xuất khi không hoạt động

## Cài đặt và chạy chương trình

### Yêu cầu hệ thống
- **OS**: Linux/Unix, Windows (với MinGW), macOS
- **Compiler**: g++ hỗ trợ C++17 trở lên
- **Libraries**: OpenSSL (cho SHA256 hashing)

### Cài đặt dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
```

#### Windows (MinGW):
```bash
# Cài đặt MinGW-w64 và OpenSSL
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-openssl
```

#### macOS:
```bash
brew install openssl
```

### Biên dịch chương trình

#### Sử dụng Makefile:
```bash
# Tải dependencies (Linux)
make install-deps

# Tạo thư mục cần thiết
make setup-dirs

# Biên dịch
make all

# Hoặc build và chạy luôn
make run
```

#### Sử dụng CMake:
```bash
mkdir build
cd build
cmake ..
make
```

#### Biên dịch thủ công:
```bash
g++ -std=c++17 -Wall -Wextra -O2 -Isrc \
    src/**/*.cpp src/*.cpp \
    -lssl -lcrypto -o wallet_system
```

### Chạy chương trình
```bash
# Nếu dùng Makefile
make run

# Hoặc chạy trực tiếp
./bin/wallet_system

# Windows
wallet_system.exe
```

## Hướng dẫn sử dụng

### 1. Khởi động ứng dụng
```
=================================================
    HỆ THỐNG ĐĂNG NHẬP VÀ QUẢN LÝ VÍ ĐIỂM THƯỞNG  
=================================================

MENU CHÍNH:
1. Đăng nhập
2. Đăng ký tài khoản mới  
3. Thoát

Chọn chức năng (1-3): 
```

### 2. Đăng ký tài khoản mới
- Nhập username (duy nhất, không thay đổi được)
- Nhập mật khẩu (tối thiểu 8 ký tự)
- Nhập thông tin cá nhân (họ tên, email, SĐT)

### 3. Đăng nhập
- Nhập username và mật khẩu
- Nếu là lần đăng nhập đầu tiên với mật khẩu tự sinh → bắt buộc đổi mật khẩu

### 4. Chức năng chính

#### User thường:
```
MENU NGƯỜI DÙNG:
1. Xem thông tin cá nhân
2. Thay đổi mật khẩu  
3. Cập nhật thông tin cá nhân
4. Xem số dư ví
5. Chuyển điểm
6. Xem lịch sử giao dịch
7. Đăng xuất
```

#### Admin:
```
MENU QUẢN TRỊ:
1. Quản lý thông tin cá nhân
2. Xem danh sách người dùng
3. Tạo tài khoản mới
4. Phát hành điểm từ ví tổng
5. Xem thống kê hệ thống
6. Quản lý backup
7. Đăng xuất
```

### 5. Quy trình chuyển điểm
1. Chọn "Chuyển điểm" từ menu
2. Nhập ID ví đích
3. Nhập số điểm muốn chuyển
4. Nhập mô tả giao dịch (tùy chọn)
5. Hệ thống tạo và hiển thị mã OTP
6. Nhập mã OTP để xác nhận
7. Giao dịch được thực hiện

## Cấu trúc dữ liệu

### Lưu trữ User (JSON):
```json
{
  "userId": "uuid-string",
  "username": "user123", 
  "passwordHash": "salt$hash",
  "fullName": "Nguyễn Văn A",
  "email": "user@example.com",
  "phoneNumber": "0123456789",
  "role": 0,
  "isPasswordGenerated": false,
  "isFirstLogin": false,
  "walletId": "W_12345678",
  "createdAt": 1640995200,
  "lastLogin": 1640995200
}
```

### Lưu trữ Wallet (JSON):
```json
{
  "walletId": "W_12345678",
  "ownerId": "uuid-string",
  "balance": 1000.0,
  "transactions": [...],
  "createdAt": 1640995200,
  "isLocked": false
}
```

### Cấu trúc thư mục:
```
project/
├── src/                    # Source code
│   ├── models/            # Data models (User, Wallet)
│   ├── security/          # Security utilities (Hash, OTP)
│   ├── storage/           # Data management
│   ├── system/            # Core systems (Auth, WalletManager)
│   ├── ui/               # User interface
│   └── main.cpp          # Entry point
├── data/                  # User data storage
├── backup/               # Backup files  
├── logs/                 # Log files
├── Makefile              # Build configuration
├── CMakeLists.txt        # CMake configuration
└── README.md             # Documentation
```

## Hệ thống Backup và Recovery

### Tự động backup:
- Backup hàng ngày vào lúc khởi động ứng dụng
- Giữ tối đa 10 bản backup gần nhất
- Checksum để kiểm tra tính toàn vẹn

### Thủ công backup:
```bash
# Từ menu admin chọn "Quản lý backup"
# Hoặc copy thư mục data/
cp -r data/ backup/manual_backup_$(date +%Y%m%d)
```

### Phục hồi dữ liệu:
1. Chọn bản backup từ danh sách
2. Xác nhận phục hồi
3. Hệ thống tự động restore và restart

## Tài liệu tham khảo

1. **CPP_OTP**: [https://github.com/patzol768/cpp-otp](https://github.com/patzol768/cpp-otp) - Thư viện OTP cho C++
2. **COTP**: [https://github.com/tilkinsc/COTP](https://github.com/tilkinsc/COTP) - Thư viện OTP khác
3. **ACID Properties**: [https://200lab.io/blog/acid-la-gi/](https://200lab.io/blog/acid-la-gi/) - Tính chất ACID trong database
4. **OpenSSL Documentation**: [https://www.openssl.org/docs/](https://www.openssl.org/docs/) - Tài liệu OpenSSL
5. **C++17 Reference**: [https://en.cppreference.com/w/cpp/17](https://en.cppreference.com/w/cpp/17) - Tài liệu C++17

## Thông tin thêm

### Liên hệ hỗ trợ:
- Email nhóm: team2c@example.com
- Repository: [https://github.com/team2c/wallet-system](https://github.com/team2c/wallet-system)

### License:
MIT License - Xem file LICENSE để biết thêm chi tiết.

---
*Phát triển bởi Team 2C - Đại học [Tên trường]*