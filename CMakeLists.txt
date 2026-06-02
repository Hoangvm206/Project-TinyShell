cmake_minimum_required(VERSION 3.12)
project(Operating-System-Project VERSION 1.0 LANGUAGES CXX)

# Bắt buộc sử dụng chuẩn C++17 để hỗ trợ các hàm nâng cao
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------
# 1. Cấu hình Biên dịch cho Ứng dụng Chính (TinyShell)
# --------------------------------------------------------
add_executable(Operating-System-Project Tinyshell.cpp)

# Liên kết các thư viện hệ thống Windows cần thiết cho Win32 API
if(WIN32)
    target_link_libraries(Operating-System-Project PRIVATE user32 kernel32 psapi)
endif()

# --------------------------------------------------------
# 2. Cấu hình Biên dịch cho các Ứng dụng Test trong apps/
# --------------------------------------------------------
add_executable(countdown_app apps/countdown_app.cpp)
add_executable(log_miner apps/log_miner.cpp)
add_executable(quick_app apps/quick_app.cpp)
add_executable(loop_app apps/loop_app.cpp)

# --------------------------------------------------------
# 3. Cấu hình Đóng gói và Cài đặt (Target: install)
# --------------------------------------------------------

# Đưa file thực thi chính vào thư mục gốc của 'install'
install(TARGETS Operating-System-Project DESTINATION .)

# Đưa 4 ứng dụng con vào đúng thư mục 'install/apps'
install(TARGETS countdown_app log_miner quick_app loop_app DESTINATION apps)

# Tự động copy các file kịch bản .bat vào thư mục gốc 'install' để chạy test luôn
install(FILES 
    process_test.bat 
    batch_test.bat 
    path_test.bat 
    invalid_test.bat 
    DESTINATION .
)