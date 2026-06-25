# Hướng dẫn cài đặt và khởi chạy Backend (SCAD-TSA)

Tài liệu này hướng dẫn cách biên dịch mã nguồn C++ và khởi chạy API server bằng Python cho dự án SCAD-TSA.

## 1. Yêu cầu chuẩn bị
Đảm bảo máy tính của bạn đã cài đặt các công cụ và thư viện sau:
- **C++ Compiler** (g++) hỗ trợ chuẩn C++17 trở lên.
- **Python 3.9+** (Đã cài các thư viện `fastapi`, `uvicorn`, `python-multipart`, `requests`, `graphviz`).
  * Cài đặt nhanh các thư viện Python cần thiết:
    ```bash
    pip install fastapi uvicorn python-multipart requests graphviz
    ```

---

## 2. Các bước khởi chạy

### Bước 1: Biên dịch các module C++
Mở terminal tại thư mục gốc của dự án (`SCAD-TSA`) và chạy các lệnh sau để tạo thư mục `bin` và biên dịch các module:
```bash
# Tạo thư mục chứa file thực thi
mkdir bin

# Biên dịch các module xử lý C++
g++ -std=c++17 -I./include src/preprocess.cpp src/Preprocessor.cpp -o bin/preprocess
g++ -std=c++17 -I./include src/analyze.cpp src/Analysis.cpp src/StatTrie.cpp -o bin/analyze
g++ -std=c++17 -I./include src/visualize.cpp -o bin/visualize

# Biên dịch chương trình điều phối chính (main_pipeline)
g++ -std=c++17 -I./include src/main_pipeline.cpp -o main_pipeline
```
*(Trên hệ điều hành Windows, trình biên dịch `g++` sẽ tự động sinh ra các file `.exe` tương ứng).*

### Bước 2: Khởi chạy API server FastAPI
Chạy file backend `app.py` trực tiếp bằng Python:
```bash
# Di chuyển terminal vào thư mục web
cd web

# Chạy server
python app.py
```
Server sẽ chạy ở chế độ **live reload** tại cổng **8000** (`http://127.0.0.1:8000`).

---

## 3. Trải nghiệm ứng dụng Web và Kiểm thử API

### A. Truy cập giao diện Web trực quan (Khuyến nghị)
Sau khi khởi chạy API server thành công ở Bước 2, hãy mở trình duyệt web và truy cập địa chỉ:
```
http://127.0.0.1:8000
```
Tại giao diện web này, bạn có thể:
1. Nhấp chọn file để tải lên file log của bạn.
2. Thiết lập các tham số tiền xử lý (Delimiters, Ignored characters, Regex) và các ngưỡng bách phân vị thống kê (Frequency, Length, Entropy).
3. Nhấp nút **Run analysis** để thực hiện phân tích. Các bảng dữ liệu bất thường được phát hiện sẽ hiển thị trực quan dưới 3 tab: **Frequency**, **Length**, và **Entropy**.

### B. Kiểm thử API `/api/analyze` bằng dòng lệnh (Dành cho Developer)
Bạn có thể gửi một request POST bằng `curl` ở một cửa sổ terminal mới:
```bash
curl -X POST -F "uploadFile=@path/to/your/log_file.log" -F "config={\"delim\":\"\\n\",\"ignore\":\"\\r\",\"regex\":\"\",\"frequencyPerc\":5,\"lengthPerc\":5,\"entropyPerc\":95,\"visual\":\"partial\"}" http://127.0.0.1:8000/api/analyze
```
*(Thay thế `path/to/your/log_file.log` bằng đường dẫn thực tế đến file log của bạn).*

Hoặc chạy file script kiểm thử tự động đi kèm trong môi trường phát triển:
```bash
python C:\Users\trant\.gemini\antigravity-ide\scratch\test_api_running.py
```

