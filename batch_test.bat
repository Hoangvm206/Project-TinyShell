echo === START BATCH TESTING ===

# 1. Bật quick_app thứ nhất chạy NGẦM (Thằng này sẽ được đưa vào danh sách quản lý)
start apps\quick_app.exe background

# 2. Bật TIẾP một quick_app thứ hai nhưng ở chế độ NỔI (Foreground)
echo Dang dung tieng trinh Foreground de tao do tre 4 giay...
start apps\quick_app.exe foreground

# 3. Sau khi thằng thứ hai chạy xong (hết 4s), Shell mới chạy xuống lệnh tiếp theo
echo Kiem tra xem quick_app chay ngam ban dau da FINISHED chua:
list

echo === FINISHED BATCH TESTING ===