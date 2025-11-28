
// Preprocessor.h
#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>

class Preprocessor {
private:
    bool toLower;
    std::unordered_set<char> ignoredChars;
    std::unordered_set<char> delimiters;


    std::string normalizeWhitespace(const std::string& s) const;

public:
    Preprocessor(bool toLower = true);

    void setIgnoredCharacters(const std::string& chars);
    void setDelimiters(const std::string& chars);

    std::string cleanLine(const std::string& line) const;
    
    // Xử lý file → trả về danh sách chuỗi sạch (đã bỏ timestamp)
    std::vector<std::string> processFile(
        const std::string& inputFile,
        const std::string& outputFile = ""
    );
    

    std::vector<std::string> filterByRegex(
        const std::string& inputFile,
        const std::string& outputFile,
        const std::string& pattern
    );
};

#endif
