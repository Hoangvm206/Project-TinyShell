# TinyShell - Windows Process Management Shell

TinyShell là một chương trình mô phỏng giao diện dòng lệnh (Shell) mini chạy trên hệ điều hành Windows. Dự án được phát triển bằng ngôn ngữ C++ kết hợp với việc khai thác các Win32 API cốt lõi nhằm mục đích nghiên cứu cơ chế quản lý tiến trình, luồng (threads), và môi trường của hệ điều hành.

## 🚀 Các tính năng chính

TinyShell hỗ trợ một tập lệnh phong phú chia làm các nhóm chức năng chính:
* **Quản lý tiến trình nền (Background):** Khởi chạy, theo dõi, tạm dừng (`stop`), tiếp tục (`resume`), và tiêu diệt (`kill`) các tiến trình thông qua ID quản lý tự động.
* **Quản lý tiến trình nổi (Foreground):** Chuyển một tiến trình từ chạy ngầm lên chạy nổi (`fg`) và đồng bộ hóa thời gian chờ cho đến khi tiến trình con kết thúc.
* **Hệ thống bẫy lỗi (Error Handling):** Tự động phát hiện và dọn dẹp các tiến trình con đã chết (Zombie Cleanup), bẫy lỗi nhập chuỗi ký tự thay vì ID số, bẫy lỗi ID không tồn tại.
* **Thực thi kịch bản (Scripting):** Khả năng đọc và thông dịch hàng loạt lệnh tự động từ file `.bat`.
* **Môi trường hệ thống:** Xem và cập nhật biến môi trường `PATH` trực tiếp vào Registry của Windows.

---

## 📂 Cấu trúc thư mục Dự án

```text
Project-TinyShell/
│
├── CMakeLists.txt        # File cấu hình CMake để build dự án tự động
├── Tinyshell.cpp         # Mã nguồn chính của TinyShell
├── README.md             # Tài liệu hướng dẫn này
│
├── process_test.bat      # Testcase 1: Quản lý vòng đời tiến trình
├── batch_test.bat        # Testcase 2: Đọc file script hàng loạt & Cleanup
├── path_test.bat         # Testcase 3: Xem và cập nhật biến PATH
├── invalid_test.bat      # Testcase 4: Kiểm tra khả năng chịu lỗi (Bẫy lỗi)
│
└── apps/                 # Thư mục chứa các ứng dụng giả lập để test
    ├── countdown_app.cpp # Ứng dụng đếm ngược 60s (Giả lập bộ đếm hệ thống)
    ├── log_miner.cpp     # Ứng dụng chạy vô hạn (Giả lập dịch vụ thu thập Log)
    ├── quick_app.cpp     # Ứng dụng tự thoát sau 4s (Test tính năng dọn dẹp)
    └── loop_app.cpp      # Ứng dụng tăng tiến trình (Giả lập dịch vụ nền)
```

---

---

## 💻 Hướng dẫn chạy các Kịch bản Kiểm thử (Testcases)

### 1️⃣ Testcase 1: Quản lý vòng đời tiến trình ngầm (`process_test.bat`)
Kiểm tra khả năng tạo, theo dõi thông tin chi tiết, đóng băng, giải phóng và tiêu diệt một tiến trình thực tế.
```text
TinyShell> start process_test.bat
```
* **Luồng chạy:** Bật `countdown_app` ngầm -> Xem PID và RAM bằng `info` -> Đóng băng bằng `stop` -> Kích hoạt lại bằng `resume` -> Tiêu diệt hẳn bằng `kill`.

### 2️⃣ Testcase 2: Thông dịch lệnh kịch bản & Tự động dọn dẹp (`batch_test.bat`)
Kiểm tra khả năng đọc tập lệnh tự động và cơ chế phát hiện tiến trình con đã tự thoát để giải phóng tài nguyên hệ thống (Zombie Cleanup).
```text
TinyShell> start batch_test.bat
```
* **Luồng chạy:** Duyệt thư mục `apps` -> Bật `quick_app` ngầm -> Đợi 5 giây cho ứng dụng tự kết thúc -> Gọi `list` để xem trạng thái `FINISHED [cleaned up]`.

### 3️⃣ Testcase 3: Quản lý biến môi trường (`path_test.bat`)
Xem và bổ sung một đường dẫn mới vào biến môi trường hệ thống.
```text
TinyShell> start path_test.bat
```
* **Luồng chạy:** Xem danh sách `PATH` hiện tại -> Thêm đường dẫn `C:\Temp` -> In lại danh sách để xác nhận cập nhật thành công.

### 4️⃣ Testcase 4: Kiểm tra khả năng chịu lỗi (`invalid_test.bat`)
Chứng minh Shell hoạt động cực kỳ bền bỉ, không bị crash hoặc văng khi người dùng cố tình nhập sai hoặc phá hoại hệ thống.
```text
TinyShell> start invalid_test.bat
```
* **Luồng chạy:** Thử tác động (`stop`, `resume`, `kill`, `info`) lên một ID không tồn tại (`999`). Hệ thống sẽ bẫy lỗi thành công và đưa ra thông báo tường minh.

### 5️⃣ Live Demo: Đưa tiến trình ngầm lên chạy nổi (`fg`)
Tính năng này làm nghẽn (block) Shell để chờ tiến trình con nên cần gõ trực tiếp từ bàn phím để demo:
```text
TinyShell> start apps\log_miner.exe background
TinyShell> fg 1
```
* **Hiện tượng:** Tiến trình giám sát Log ngầm sẽ được kéo lên chạy nổi, in dữ liệu packet nhảy liên tục trên màn hình chính. Khi nhấn `Ctrl+C` hoặc tắt tiến trình con, quyền kiểm tra sẽ được trả lại cho TinyShell an toàn.

---

## 📋 Tổng hợp các lệnh được hỗ trợ

| Lệnh | Cú pháp | Mô tả chi tiết |
| :--- | :--- | :--- |
| `help` | `help` | Hiển thị bảng trợ giúp hướng dẫn sử dụng lệnh. |
| `dir` | `dir [path]` | Liệt kê danh sách file, thư mục, kích thước, thời gian sửa đổi. |
| `start` | `start <file> [mode]` | Khởi chạy ứng dụng dạng `background` hoặc `foreground`. |
| `stop` | `stop <id>` | Tạm dừng luồng (Suspend) của một tiến trình ngầm. |
| `resume` | `resume <id>` | Tiếp tục chạy lại (Resume) tiến trình đang bị đóng băng. |
| `kill` | `kill <id>` hoặc `kill -1` | Tiêu diệt một tiến trình được chỉ định hoặc toàn bộ tiến trình (`-1`). |
| `list` | `list` | Hiển thị danh sách tất cả các tiến trình kèm trạng thái (RUNNING/SUSPENDED). |
| `info` | `info <id>` | Đào sâu thông tin hệ thống: PID, Uptime, Mức chiếm dụng RAM, Đường dẫn. |
| `fg` | `fg <id>` | Đưa tiến trình nền lên chạy nổi (Foreground) và chờ đợi nó kết thúc. |
| `path` | `path` | Hiển thị toàn bộ danh sách các đường dẫn trong biến môi trường PATH. |
| `addpath` | `addpath <path>` | Đăng ký thêm một thư mục mới vào biến môi trường PATH. |
| `echo` | `echo <text>` | In dòng văn bản ra màn hình. |
| `clear` | `clear` | Xóa sạch màn hình console. |
| `exit` | `exit` | Dọn dẹp sạch sẽ toàn bộ các tiến trình con đang chạy ngầm và thoát Shell. |
