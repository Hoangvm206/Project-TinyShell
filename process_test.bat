# Testcase 1: Quản lý vòng đời tiến trình countdown
echo === RUNNING PROCESS TEST ===
start apps\countdown_app.exe background
list
echo === KIỂM TRA THÔNG TIN TIẾN TRÌNH VỪA TẠO ===
info 1
echo === TAM DỪNG TIẾN TRÌNH ===
stop 1
list
echo === TIẾP TỤC TIẾN TRÌNH ===
resume 1
list
echo === TIÊU DIỆT TIẾN TRÌNH ===
kill 1
list
echo === END PROCESS TEST ===