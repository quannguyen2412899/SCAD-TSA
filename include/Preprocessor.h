#ifndef _PREPROCESSOR_
#define _PREPROCESSOR_

#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;


enum class DataType {
    AUTO,       
    TEXT,       
    LOG,        
    DNA,        
    UNKNOWN     
};

/**
 * @brief Lớp Preprocessor:
 *  - Đọc dữ liệu từ file
 *  - Làm sạch ký tự (loại bỏ ký tự thừa, chuẩn hóa chữ thường)
 *  - Tùy chọn chia nhỏ chuỗi (chunking)
 *  - Xuất danh sách chuỗi sạch sẵn sàng đưa vào Trie
 */
class Preprocessor {
private:
    string filename;
    bool toLower;
    bool chunking;
    size_t chunkSize;
    unordered_set<char> ignoredChars;
    unordered_set<char> delimiters;
    vector<string> processedData;

    static string cleanLine(const string& line, bool toLower, const unordered_set<char>& ignored);
    static vector<string> chunkString(const string& s, size_t k);

public:
    // Constructor
    Preprocessor(string filename, bool toLower = true, bool chunking = false, size_t chunkSize = 50);

    // Cấu hình
    void setIgnoredCharacters(const string& chars);
    void setDelimiters(const string& chars);

    // Thực thi
    void run(DataType datatype = DataType::AUTO);

    // Tiện ích
    static DataType detectDataType(const string& sample);
    static string toString(DataType t);

    // Getter
    const vector<string>& getProcessedData() const { return processedData; }
    size_t getChunkSize() const { return chunkSize; }
    bool isChunking() const { return chunking; }
};

#endif
