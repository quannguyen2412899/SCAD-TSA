# 🌳 SCAD-TSA: NÉN VÀ PHÁT HIỆN CHUỖI BẤT THƯỜNG BẰNG TRIE VÀ PHÂN TÍCH THỐNG KÊ

### 🎓 Thông tin đề tài
| Học phần | Cấu trúc dữ liệu và Giải thuật Mở rộng|
| :--- | :--- |
| **Sinh viên** | Trần Tấn Phát - 2412610<br>Nguyễn Anh Quân - 2412899|

### 🌐 Kho lưu trữ và Tài liệu
| Mô tả | Liên kết |
| :--- | :--- |
| **GitHub Repository** | [https://github.com/quannguyen2412899/SCAD-TSA](https://github.com/quannguyen2412899/SCAD-TSA) |
| **Google Colab Notebook** | [https://colab.research.google.com/github/quannguyen2412899/SCAD-TSA/blob/master/colab/demo.ipynb](https://colab.research.google.com/github/quannguyen2412899/SCAD-TSA/blob/master/colab/demo.ipynb) |
| **File báo cáo** | [https://github.com/quannguyen2412899/SCAD-TSA/blob/master/report/Report.pdf](https://github.com/quannguyen2412899/SCAD-TSA/blob/master/report/Report.pdf) |

---
## 🔧 Yêu cầu hệ thống và phụ thuộc

Để chạy toàn bộ pipeline, hệ thống của bạn cần đáp ứng các yêu cầu sau:

#### 1. C++
* **Compiler:** GNU C++ Compiler (g++), hỗ trợ chuẩn C++17 trở lên.
* **Thư viện Bên ngoài:** **nlohmann/json**
    * Sử dụng bởi module **`bin/analyze`** để xuất các file JSON cấu trúc Trie.
    * File header `json.hpp` được đặt trong thư mục `include/` của dự án và được link vào lúc biên dịch.

#### 2. Python
* **Python:** Python 3.9.
* **Thư viện Python:** **`graphviz`**
    ```bash
    pip install graphviz
    ```

#### 3. Graphviz
**Graphviz Engine (dot):** Chuyển đổi file DOT (được sinh ra bởi script Python) thành hình ảnh.
* **Trên Debian/Ubuntu:** `sudo apt-get install graphviz`
* **Trên macOS (Homebrew):** `brew install graphviz`
* **Hoặc tải trực tiếp từ website:** `https://graphviz.org/`
    
*Lưu ý:* Nếu chạy trên Google Colab, bạn có thể cần dùng lệnh `!apt install graphviz` trong notebook.

---

## 🏗️ Biên dịch

```bash
# Bước 1: Tạo thư mục chứa các executable
mkdir -p bin

# Bước 2: Biên dịch các module C++
g++ -std=c++17 -I./include src/preprocess.cpp src/Preprocessor.cpp -o bin/preprocess
g++ -std=c++17 -I./include src/analyze.cpp src/Analysis.cpp src/StatTrie.cpp -o bin/analyze
g++ -std=c++17 -I./include src/visualize.cpp -o bin/visualize
g++ -std=c++17 -I./include src/main_pipeline.cpp -o main_pipeline
```

-----

## 🚀 Chạy chương trình

Toàn bộ quá trình xử lý được điều phối thông qua chương trình chính: **`./main_pipeline`**.

#### Cú pháp chung
```bash
./main_pipeline <input_file> <output_dir> [flags]
```

#### Ví dụ

Lệnh chạy cơ bản, thực hiện phân tích đầy đủ và xuất biểu đồ cây **rút gọn** (chỉ hiển thị các bất thường) vào thư mục `results/`:
```bash
./main_pipeline data/sample.log results --visual-partial
```
-----

## 🚩 Chi tiết các cờ

#### 1\. Cờ Tiền xử lý

Các cờ này kiểm soát cách dữ liệu đầu vào được làm sạch và tách chuỗi bởi `bin/preprocess`.

  * **Mặc định:** `--delim`: `\n` (luôn bao gồm). `--ignore`: `\r` (luôn bao gồm).
  * **Ưu tiên:** Cờ `--regex` có **ưu tiên cao nhất**, ghi đè mọi thiết lập `delim`/`ignore` khác.

| Flag | Mô tả | Ví dụ |
| :--- | :--- | :--- |
| **`--regex="<pattern>"`** | Lọc chuỗi theo biểu thức chính quy để trích xuất các trường dữ liệu cụ thể. | `--regex="[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"` |
| **`--delim="<chars>"`** | Chuỗi ký tự phân cách. | `--delim=","` |
| **`--ignore="<chars>"`** | Chuỗi ký tự cần loại bỏ khỏi chuỗi. | `--ignore=",.!?"` |

#### 2\. Cờ Xử lý

Cấu hình percentile đầu vào của `bin/analyze` để xác định ngưỡng bất thường.
* **Mặc định:** `--perc-freq=5`, `--perc-len=5`, `--perc-entropy=95`

| Flag | Mô tả | Ví dụ |
| :--- | :--- | :--- |
| **`--perc-freq=<val>`** | Cài đặt percentile để xác định ngưỡng bất thường **tần suất**. | `--perc-freq=1` |
| **`--perc-len=<val>`** | Cài đặt percentile để xác định ngưỡng bất thường **độ dài**. | `--perc-len=1` |
| **`--perc-entropy=<val>`** | Cài đặt percentile để xác định ngưỡng bất thường **entropy**. | `--perc-entropy=99` |

#### 3\. Cờ Trực quan hóa

Yêu cầu `bin/analyze` tạo ra file JSON, sau đó được `bin/visualize` chuyển đổi thành hình ảnh.

| Flag | Mô tả | File JSON/PNG Xuất ra |
| :--- | :--- | :--- |
| **`--visual-complete`** | Xuất JSON và vẽ **toàn bộ** cây Trie. | `complete_trie.json`, `complete_trie.png` |
| **`--visual-partial`** | **Khuyến nghị:** Xuất JSON và vẽ cây rút gọn (chỉ hiển thị các nhánh chứa bất thường). | `partial_trie.json`, `partial_trie.png` |
| **`--visual-freq`** | Xuất JSON và vẽ chỉ các nhánh liên quan đến **Bất thường Tần suất**. | `frequency_anomalies.json`, `frequency_anomalies.png` |
| **`--visual-len`** | Xuất JSON và vẽ chỉ các nhánh liên quan đến **Bất thường Độ dài**. | `length_anomalies.json`, `length_anomalies.png` |
| **`--visual-entropy`** | Xuất JSON và vẽ chỉ các nhánh liên quan đến **Bất thường Entropy**. | `entropy_anomalies.json`, `entropy_anomalies.png` |


-----
### 📋 Chi tiết các File Output
Module **`bin/preprocess`** làm sạch file input và tạo ra file clean: `cleaned_data.txt`

Module **`bin/analyze`** tạo ra các kết quả định lượng sau:
* `overall_report.txt`: Báo cáo tóm tắt chứa các **ngưỡng bách phân vị (P5, P95)** được tính toán thực tế.
* `all_entries.csv`: Bảng thống kê đầy đủ (Count, Length, Entropy) của TẤT CẢ các chuỗi/tiền tố duy nhất.
* `frequency_anomalies.csv`: Danh sách các chuỗi bất thường về tần suất.
* `length_anomalies.csv`: Danh sách các chuỗi bất thường về độ dài.
* `entropy_anomalies.csv`: Danh sách các chuỗi bất thường về entropy.
* `*.json` (nếu có flag trực quan): Các file chứa cấu trúc Trie đã được lọc/đánh dấu, dùng làm đầu vào cho module `visualize`.

Module **`bin/visualize`** sẽ trực quan cây trie từ các file `*json`:

---


## Chi tiết hệ thống
### ⚙️ Tóm tắt
- Hệ thống sử dụng cấu trúc dữ liệu **Trie** để nén dữ liệu và áp dụng các đặc trưng thống kê như **tần suất**, **độ dài** và **entropy cục bộ** tại mỗi nút để xác định các chuỗi có cấu trúc hoặc hành vi **bất thường**.
- Phương pháp phát hiện bất thường sử dụng kỹ thuật **bách phân vị (Percentile)**, đảm bảo tính vững chắc trước sự lệch của phân phối tần suất chuỗi.

### 💡 Ý tưởng Cốt lõi và Lợi ích của Trie
Hệ thống sử dụng cấu trúc dữ liệu **Trie** với hai mục đích chính:
- **Nén Dữ liệu:** Chia sẻ prefix chung giữa các chuỗi, giúp tiết kiệm bộ nhớ và đạt được mức nén đáng kể trên các tập dữ liệu log/URL lớn.
- **Lưu trữ Thống kê Hiệu quả:** Mỗi nút (Node) trên Trie là một vị trí lý tưởng để lưu trữ các đặc trưng thống kê cục bộ (như **Tần suất**, **Độ dài**, **Entropy**) cần thiết cho việc phân tích nhánh.

### 🔬 Phương pháp Phát hiện Bất thường
Thay vì sử dụng các chỉ số thống kê truyền thống như Mean/Standard Deviation (dễ bị ảnh hưởng bởi dữ liệu lệch), hệ thống áp dụng phương pháp **bách phân vị** để xác định ngưỡng một cách vững chắc.

Bất thường được xác định dựa trên ba tiêu chí chính:
| Tiêu chí | Đặc trưng | Ngưỡng Quyết định | Giải thích |
| :--- | :--- | :--- | :--- |
| **Bất thường Tần suất** | Tần suất | **≤ input percentile** | Chuỗi hiếm, xuất hiện không đủ thường xuyên để được coi là mẫu chuẩn. |
| **Bất thường Độ dài** | Tần suất độ dài | **≤ input percentile** | Chuỗi có độ dài hiếm, ít gặp trong phân phối tần suất độ dài chung. |
| **Bất thường Entropy** | Entropy Cục bộ | **≥ input percentile** | Node có sự phân nhánh quá mức, chỉ ra sự đa dạng ký tự bất thường trong chuỗi. |

*Các giá trị bách phân vị được tính dựa trên phân phối trọng số của toàn bộ dữ liệu, đảm bảo ngưỡng ổn định ngay cả khi dữ liệu đầu vào bị lệch mạnh.*


### 🏗️ Sơ đồ pipeline tổng thể

Hệ thống được thiết kế dưới dạng một chuỗi lệnh (pipeline) được điều phối bởi executable **`main_pipeline`**. Dữ liệu được xử lý tuần tự qua ba module chính:

1.  **Tiền xử lý (Preprocess):** Chuẩn hóa dữ liệu thô.
2.  **Phân tích (Analyze):** Xây dựng Trie, tính toán thống kê và phát hiện bất thường.
3.  **Trực quan hóa (Visualize):** Chuyển đổi kết quả JSON thành biểu đồ cây Trie dạng PNG/SVG.

### ✉️ Luồng dữ liệu giữa các module

| Executable | Chức năng Chính | Đầu vào | Đầu ra Chính |
| :--- | :--- | :--- | :--- |
| **`main_pipeline`** | **Điều phối** các module. | `<input_file>`, `<output_dir>` | Là các đầu ra của 3 module còn lại |
| **`bin/preprocess`** | Chuẩn hóa, làm sạch chuỗi. | `<input_file>` | `cleaned_data.txt` |
| **`bin/analyze`** | Xây dựng Trie, tính toán bách phân vị, đánh dấu bất thường, xuất báo cáo. | `cleaned_data.txt` | `overall_report.txt`<br>`all_entries.csv`<br>`frequency_anomalies.csv`<br> `length_anomalies.csv` <br> `entropy_anomalies.csv` <br>`*.json` |
| **`bin/visualize`** | Wrapper C++ gọi script Python để vẽ trie. | `*.json` | `*.png` |

