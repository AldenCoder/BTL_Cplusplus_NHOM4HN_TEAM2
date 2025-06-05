# Hệ thống Đăng nhập và Quản lý Ví Điểm Thưởng

Một hệ thống quản lý điểm thưởng an toàn dựa trên SQLite được xây dựng bằng C++, xử lý xác thực người dùng, giao dịch ví và kiểm soát quản trị với tính toàn vẹn dữ liệu tuân thủ ACID.

## 🌟 Tính năng chính

### 🔐 **Hệ thống Xác thực An toàn**
- **Mã hóa Mật khẩu SHA256**: Mật khẩu được băm với salt ngẫu nhiên để bảo mật
- **Kiểm soát Truy cập theo Vai trò**: Người dùng thường và quản trị viên với quyền khác nhau
- **Người dùng Đầu tiên làm Admin**: Người dùng đăng ký đầu tiên tự động trở thành quản trị viên
- **Quản lý Phiên**: Đăng nhập/đăng xuất an toàn với theo dõi phiên
- **Xác minh OTP**: Xác thực hai yếu tố cho các thao tác nhạy cảm

### 💰 **Quản lý Ví và Giao dịch**
- **Ví Cá nhân**: Mỗi người dùng có một ví cá nhân với ID duy nhất
- **Chuyển Điểm**: Chuyển điểm an toàn giữa người dùng với xác minh OTP
- **Lịch sử Giao dịch**: Dấu vết kiểm toán đầy đủ của tất cả giao dịch
- **Ví Tổng**: Ví quản trị để phát hành điểm mới cho người dùng
- **Tuân thủ ACID**: Giao dịch cơ sở dữ liệu đảm bảo tính toàn vẹn dữ liệu
- **Theo dõi Số dư**: Cập nhật số dư thời gian thực với các thao tác nguyên tử

### 👑 **Tính năng Quản trị**
- **Quản lý Người dùng**: Tạo, xem và quản lý tài khoản người dùng
- **Thống kê Hệ thống**: Xem thống kê sử dụng hệ thống toàn diện
- **Phát hành Điểm**: Phát hành điểm mới từ ví tổng đến ví người dùng
- **Tạo Tài khoản**: Tạo tài khoản với mật khẩu bảo mật tự động tạo
- **Sao lưu Cơ sở dữ liệu**: Sao lưu thủ công và tự động với khả năng khôi phục

### 🛡️ **Bảo mật & Tính toàn vẹn Dữ liệu**
- **Cơ sở dữ liệu SQLite**: Lưu trữ tuân thủ ACID với chế độ WAL cho hiệu suất
- **Thao tác An toàn Thread**: Các thao tác cơ sở dữ liệu được bảo vệ bằng mutex
- **Xác thực Dữ liệu**: Xác thực email và số điện thoại Việt Nam
- **Sao lưu Tự động**: Chức năng sao lưu theo lịch và thủ công
- **Ràng buộc Khóa ngoại**: Thực thi tính toàn vẹn cơ sở dữ liệu

## 👥 Thành viên nhóm và phân công công việc

| STT | Họ tên | MSSV | Công việc được giao |
|-----|--------|------|-------------------|
| 1 | [Tên thành viên 1] | [MSSV] | **Kiến trúc Hệ thống & Bảo mật**: Thiết kế kiến trúc hệ thống, triển khai SecurityUtils, hệ thống OTP |
| 2 | [Tên thành viên 2] | [MSSV] | **Quản lý Người dùng**: Triển khai lớp User, AuthSystem, xác thực người dùng |
| 3 | [Tên thành viên 3] | [MSSV] | **Hệ thống Ví**: Triển khai lớp Wallet, WalletManager, xử lý giao dịch |
| 4 | [Tên thành viên 4] | [MSSV] | **Quản lý Dữ liệu & Giao diện**: Triển khai DatabaseManager, UserInterface, hệ thống sao lưu |

## ✨ Tính năng nổi bật (Phiên bản hiện tại)

### 🔄 **Khởi tạo Cơ sở dữ liệu Trống**
- **Thay đổi quan trọng**: Hệ thống bắt đầu với cơ sở dữ liệu hoàn toàn trống
- **Không tạo admin mặc định**: Không tự động tạo tài khoản admin như trước
- **Người dùng đầu tiên làm admin**: Người đăng ký đầu tiên sẽ tự động trở thành quản trị viên
- **Kiểm soát bảo mật**: Đảm bảo kiểm soát thủ công đối với tài khoản admin đầu tiên

### 🗃️ **Chuyển đổi sang Cơ sở dữ liệu SQLite**
- **Trước đây**: Hệ thống lưu trữ dựa trên file JSON
- **Hiện tại**: Cơ sở dữ liệu SQLite với tuân thủ ACID
- **Lợi ích**: Tính toàn vẹn dữ liệu tốt hơn, truy cập đồng thời và hiệu suất
- **Chuyển đổi tự động**: Tạo schema tự động khi chạy lần đầu

### 🔑 **Tạo Mật khẩu Tự động**
- **Chức năng**: Admin có thể tạo tài khoản cho người dùng với mật khẩu được sinh tự động
- **Bảo mật**: Mật khẩu ngẫu nhiên 12 ký tự (chữ cái, số, ký tự đặc biệt)
- **Cải tiến Giao diện**: Hiển thị mật khẩu đã tạo với cảnh báo rõ ràng cho admin
- **Triển khai**: Sử dụng `SecurityUtils::generateSecurePassword()`

### 📦 **Hệ thống Sao lưu Nâng cao**
- **Sao lưu thủ công**: Admin có thể tạo sao lưu bất kỳ lúc nào với mô tả tùy chỉnh
- **Lịch sử sao lưu**: Xem danh sách tất cả sao lưu với thông tin chi tiết (ID, thời gian, kích thước)
- **Phục hồi dữ liệu**: Chọn từ danh sách sao lưu và phục hồi với sao lưu an toàn tự động
- **Dọn dẹp sao lưu**: Tự động xóa sao lưu cũ, chỉ giữ lại số lượng theo cấu hình
- **Theo dõi Metadata**: Theo dõi loại sao lưu (Manual/Auto/Emergency), checksum, kích thước file

### 🎯 **Phát hành Điểm từ Ví Tổng**
- **Tích hợp Ví Tổng**: Kết nối với ví tổng để phát hành điểm
- **Kiểm soát Admin**: Chỉ admin có quyền phát hành điểm
- **Theo dõi Giao dịch**: Ghi nhận tất cả giao dịch phát hành với lý do
- **Quản lý Số dư**: Tự động cập nhật số dư ví người dùng

### ✅ **Xác thực Nâng cao**
- **Xác thực Email**: Kiểm tra định dạng email theo chuẩn RFC với kiểm tra bổ sung
  - Không cho phép dấu chấm liên tiếp (..)
  - Không cho phép dấu chấm ở đầu/cuối
  - Xác thực độ dài email tối đa 254 ký tự
- **Xác thực Số điện thoại**: Hỗ trợ số điện thoại Việt Nam
  - Định dạng: 84xxxxxxxxx, 0xxxxxxxxx, hoặc 10-11 chữ số
  - Tự động loại bỏ ký tự phân cách (space, dash, brackets)
  - Kiểm tra pattern phù hợp với tiêu chuẩn VN

### 🔧 **Cải thiện Giao diện/Trải nghiệm Người dùng**
- **Hệ thống Cảnh báo**: Thêm phương thức `showWarning()` cho các cảnh báo quan trọng
- **Hiển thị Được định dạng**: Hiển thị kích thước file, datetime theo định dạng dễ đọc
- **Cải tiến Menu**: Quản lý sao lưu với submenu đầy đủ chức năng
- **Xử lý Lỗi**: Xử lý lỗi tốt hơn với try-catch và thông báo rõ ràng

## 🚀 Hướng dẫn Cài đặt

### Yêu cầu Tiên quyết

#### **Windows**
```bash
cài sqlite
.\build.ps1
.\bin\wallet_system.exe
```

#### **macOS**
```bash
# Cài đặt Homebrew nếu chưa có
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Cài đặt dependencies cần thiết
brew install sqlite3
brew install pkg-config

# Cài đặt Xcode Command Line Tools
xcode-select --install
```

#### **Linux (Ubuntu/Debian)**
```bash
# Cập nhật danh sách package
sudo apt update

# Cài đặt dependencies cần thiết
sudo apt install g++ sqlite3 libsqlite3-dev pkg-config make cmake

# Cho các distro khác:
# CentOS/RHEL: sudo yum install gcc-c++ sqlite-devel pkgconfig make cmake
# Arch Linux: sudo pacman -S gcc sqlite pkg-config make cmake
```

### 📥 Cài đặt & Biên dịch

#### **Lựa chọn 1: Sử dụng Makefile (Khuyến nghị)**
```bash
# Clone repository
git clone <repository-url>
cd team2C

# Biên dịch dự án
make

# Chạy ứng dụng
./bin/wallet_system
```

#### **Lựa chọn 2: Sử dụng CMake**
```bash
# Clone repository
git clone <repository-url>
cd team2C

# Tạo thư mục build
mkdir build && cd build

# Cấu hình và biên dịch
cmake ..
make

# Chạy ứng dụng
./WalletSystem
```

## 🎯 Hướng dẫn Sử dụng

### **Thiết lập Lần đầu**

1. **Khởi chạy Ứng dụng**
   ```bash
   ./bin/wallet_system
   ```

2. **Tạo Tài khoản Quản trị viên Đầu tiên**
   - Hệ thống bắt đầu với cơ sở dữ liệu trống
   - Người dùng đăng ký đầu tiên tự động trở thành quản trị viên
   - Chọn tên người dùng và mật khẩu mạnh để bảo mật

3. **Cấu hình Hệ thống Ban đầu**
   - Đăng nhập với tài khoản admin của bạn
   - Thiết lập các tài khoản người dùng bổ sung nếu cần
   - Cấu hình thiết lập sao lưu

### **Thao tác Người dùng**

#### **Đăng ký & Đăng nhập**
- Người dùng mới có thể đăng ký với tên người dùng, mật khẩu, họ tên, email và số điện thoại
- Đăng nhập bằng tên người dùng và mật khẩu
- Người dùng đầu tiên tự động trở thành admin

#### **Quản lý Ví**
- Xem số dư hiện tại và lịch sử giao dịch
- Chuyển điểm cho người dùng khác (yêu cầu xác minh OTP)
- Xem báo cáo giao dịch chi tiết

#### **Quản lý Hồ sơ**
- Cập nhật thông tin cá nhân (yêu cầu xác minh OTP)
- Đổi mật khẩu với xác minh mật khẩu cũ

### **Thao tác Quản trị**

#### **Quản lý Người dùng**
- Xem tất cả người dùng đã đăng ký
- Tạo tài khoản người dùng mới với mật khẩu tự động tạo
- Tìm kiếm người dùng theo tên người dùng
- Quản lý vai trò và quyền người dùng

#### **Quản trị Hệ thống**
- Phát hành điểm từ ví tổng đến ví người dùng
- Xem thống kê hệ thống toàn diện
- Tạo và khôi phục sao lưu cơ sở dữ liệu
- Giám sát hoạt động giao dịch

## 🏗️ Kiến trúc Hệ thống

### **Các Thành phần Hệ thống**

```
├── Hệ thống Xác thực (AuthSystem)
│   ├── Đăng ký và đăng nhập người dùng
│   ├── Quản lý phiên
│   ├── Bảo mật mật khẩu
│   └── Kiểm soát truy cập theo vai trò
│
├── Quản lý Ví (WalletManager)
│   ├── Tạo và quản lý ví
│   ├── Thao tác chuyển điểm
│   ├── Xử lý giao dịch
│   └── Thao tác ví tổng
│
├── Lớp Cơ sở dữ liệu (DatabaseManager)
│   ├── Thao tác cơ sở dữ liệu SQLite
│   ├── Quản lý giao dịch ACID
│   ├── Sao lưu và khôi phục dữ liệu
│   └── Truy cập dữ liệu an toàn thread
│
├── Lớp Bảo mật (SecurityUtils, OTPManager)
│   ├── Băm mật khẩu (SHA256 + salt)
│   ├── Tạo và xác minh OTP
│   ├── Xác thực đầu vào
│   └── Tạo ngẫu nhiên an toàn
│
└── Giao diện Người dùng (UserInterface)
    ├── Tương tác dựa trên console
    ├── Điều hướng menu
    ├── Xử lý đầu vào
    └── Định dạng hiển thị
```

### **Schema Cơ sở dữ liệu**

#### **Bảng Users**
```sql
CREATE TABLE users (
    user_id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    full_name TEXT NOT NULL,
    email TEXT NOT NULL,
    phone_number TEXT NOT NULL,
    role INTEGER NOT NULL,
    is_password_generated INTEGER DEFAULT 0,
    is_first_login INTEGER DEFAULT 1,
    wallet_id TEXT NOT NULL,
    created_at INTEGER NOT NULL,
    last_login INTEGER
);
```

#### **Bảng Wallets**
```sql
CREATE TABLE wallets (
    wallet_id TEXT PRIMARY KEY,
    owner_id TEXT NOT NULL,
    balance REAL NOT NULL DEFAULT 0.0,
    created_at INTEGER NOT NULL,
    is_locked INTEGER DEFAULT 0,
    FOREIGN KEY (owner_id) REFERENCES users (user_id)
);
```

#### **Bảng Transactions**
```sql
CREATE TABLE transactions (
    transaction_id TEXT PRIMARY KEY,
    from_wallet_id TEXT,
    to_wallet_id TEXT,
    amount REAL NOT NULL,
    description TEXT,
    transaction_type INTEGER NOT NULL,
    timestamp INTEGER NOT NULL,
    FOREIGN KEY (from_wallet_id) REFERENCES wallets (wallet_id),
    FOREIGN KEY (to_wallet_id) REFERENCES wallets (wallet_id)
);
```

## 🔒 Tính năng Bảo mật

### **Bảo mật Mật khẩu**
- **Thuật toán Băm**: SHA256 với salt ngẫu nhiên
- **Định dạng Lưu trữ**: `salt$hash` để lưu trữ an toàn
- **Mật khẩu Tự động Tạo**: Mật khẩu an toàn 12 ký tự cho tài khoản do admin tạo

### **Bảo mật Giao dịch**
- **Xác minh OTP**: OTP 6 chữ số yêu cầu cho chuyển điểm
- **Giao dịch ACID**: Giao dịch cơ sở dữ liệu đảm bảo tính nhất quán
- **Thao tác Nguyên tử**: Xử lý giao dịch all-or-nothing
- **Dấu vết Kiểm toán**: Lịch sử giao dịch hoàn chỉnh với timestamps

### **Bảo vệ Dữ liệu**
- **Chế độ SQLite WAL**: Write-Ahead Logging cho tính toàn vẹn dữ liệu
- **Ràng buộc Khóa ngoại**: Tính toàn vẹn tham chiếu cơ sở dữ liệu
- **Thao tác Thread-Safe**: Truy cập cơ sở dữ liệu được bảo vệ bằng mutex
- **Sao lưu Tự động**: Sao lưu dữ liệu thường xuyên cho khôi phục thảm họa

## 🔧 Cấu hình

### **Lưu trữ Dữ liệu**
- **File Cơ sở dữ liệu**: `data/wallet_system.db`
- **Thư mục Sao lưu**: `data/backup/`
- **File Log**: `logs/` (nếu logging được bật)

### **Thiết lập Mặc định**
- **Điểm Ban đầu của Người dùng**: 100 điểm cho người dùng mới
- **Thời hạn OTP**: 5 phút
- **Độ dài OTP**: 6 chữ số
- **Số lượng Sao lưu Tối đa**: 10 sao lưu
- **Khoảng thời gian Sao lưu Tự động**: 24 giờ

## 🎯 Thay đổi Chính từ Phiên bản Trước

### **🔄 Khởi tạo Cơ sở dữ liệu Trống**
- **Trước**: Hệ thống tự động tạo tài khoản admin mặc định
- **Hiện tại**: Cơ sở dữ liệu bắt đầu hoàn toàn trống
- **Người dùng Đầu tiên**: Tự động trở thành quản trị viên
- **Bảo mật**: Đảm bảo kiểm soát thủ công đối với tài khoản admin ban đầu

### **🗃️ Chuyển đổi Cơ sở dữ liệu SQLite**
- **Trước**: Hệ thống lưu trữ dựa trên file JSON
- **Hiện tại**: Cơ sở dữ liệu SQLite với tuân thủ ACID
- **Lợi ích**: Tính toàn vẹn dữ liệu tốt hơn, truy cập đồng thời và hiệu suất
- **Chuyển đổi**: Tạo schema tự động khi chạy lần đầu

### **⚡ Hiệu suất Nâng cao**
- **Lập chỉ mục Cơ sở dữ liệu**: Truy vấn được tối ưu hóa với chỉ mục phù hợp
- **Chế độ WAL**: Write-Ahead Logging cho hiệu suất đồng thời tốt hơn
- **Tập hợp Kết nối**: Quản lý kết nối cơ sở dữ liệu hiệu quả
- **Sử dụng Bộ nhớ**: Sử dụng bộ nhớ được tối ưu hóa với smart pointers

## 🐛 Khắc phục Sự cố

### **Các Vấn đề Thường gặp**

#### **Lỗi Biên dịch**
```bash
# Không tìm thấy SQLite3
sudo apt install libsqlite3-dev  # Ubuntu/Debian
brew install sqlite3             # macOS
# Windows: Tải SQLite3 development libraries

# Lỗi compiler
# Đảm bảo hỗ trợ C++17 (GCC 7+ hoặc Clang 5+)
g++ --version
```

#### **Lỗi Runtime**
```bash
# Lỗi quyền cơ sở dữ liệu
chmod 755 data/
chmod 644 data/wallet_system.db

# Thiếu dependencies
ldd ./bin/wallet_system  # Linux
otool -L ./bin/wallet_system  # macOS
```

#### **Vấn đề Cơ sở dữ liệu**
```bash
# Kiểm tra tính toàn vẹn cơ sở dữ liệu
sqlite3 data/wallet_system.db "PRAGMA integrity_check;"

# Reset cơ sở dữ liệu (CẢNH BÁO: Điều này xóa tất cả dữ liệu)
rm data/wallet_system.db
# Khởi động lại ứng dụng để tạo lại cơ sở dữ liệu
```

### **Tối ưu hóa Hiệu suất**
```bash
# Bật chế độ WAL cho hiệu suất tốt hơn
sqlite3 data/wallet_system.db "PRAGMA journal_mode=WAL;"

# Phân tích hiệu suất cơ sở dữ liệu
sqlite3 data/wallet_system.db "ANALYZE;"
```

## 🔄 Sao lưu & Khôi phục

### **Sao lưu Tự động**
- **Được lên lịch**: Sao lưu tự động hàng ngày
- **Được kích hoạt**: Sao lưu trước các thao tác quan trọng
- **Lưu giữ**: Chính sách lưu giữ sao lưu có thể cấu hình

### **Sao lưu Thủ công**
```bash
# Tạo sao lưu thủ công
# Truy cập qua menu admin -> Quản lý Sao lưu -> Tạo Sao lưu Thủ công

# File sao lưu được lưu trữ trong: data/backup/
# Định dạng: backup_YYYYMMDD_HHMMSS.db
```

### **Quy trình Khôi phục**
1. Truy cập menu admin
2. Chọn "Quản lý Sao lưu"
3. Chọn "Khôi phục từ Sao lưu"
4. Chọn file sao lưu từ danh sách
5. Xác nhận khôi phục (tạo sao lưu an toàn trước)

## 📚 Tài liệu Tham khảo

1. **CPP OTP**: [https://github.com/patzol768/cpp-otp](https://github.com/patzol768/cpp-otp) - Thư viện OTP cho C++
2. **COTP**: [https://github.com/tilkinsc/COTP](https://github.com/tilkinsc/COTP) - Thư viện OTP alternative
3. **ACID Properties**: [https://200lab.io/blog/acid-la-gi/](https://200lab.io/blog/acid-la-gi/) - Tính chất ACID trong cơ sở dữ liệu
4. **OpenSSL Documentation**: [https://www.openssl.org/docs/](https://www.openssl.org/docs/) - Tài liệu OpenSSL
5. **C++17 Reference**: [https://en.cppreference.com/w/cpp/17](https://en.cppreference.com/w/cpp/17) - Tài liệu C++17
6. **SQLite Documentation**: [https://www.sqlite.org/docs.html](https://www.sqlite.org/docs.html) - Tài liệu SQLite

**Nhóm 2 C++** - Hệ thống Quản lý Ví Điểm Thưởng © 2025