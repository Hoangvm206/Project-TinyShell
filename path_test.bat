# Testcase 3: Kiểm tra xem và thêm đường dẫn môi trường PATH
echo === CURRENT PATH ENVIROMENT ===
path
echo === ADDING NEW PATH (CẦN QUYỀN ADMIN NẾU LÀ THƯ MỤC HỆ THỐNG) ===
# Lưu ý: Nên tạo sẵn thư mục C:\Temp trên máy trước khi chạy lệnh này để tránh Reg từ chối
addpath C:\Temp
echo === VERIFYING PATH UPDATE ===
path