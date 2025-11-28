#include "../include/Preprocessor.h"

// Preprocessor.cpp
#include "../include/Preprocessor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <regex>
#include <cctype>

Preprocessor::Preprocessor(bool toLower)
    : toLower(toLower) {
}

void Preprocessor::setIgnoredCharacters(const std::string& chars) {
    for (char c : chars) {
        ignoredChars.insert(toLower ? (char)std::tolower(c) : c);
    }
}

void Preprocessor::setDelimiters(const std::string& chars) {
    for (char c : chars) {
        delimiters.insert(toLower ? (char)std::tolower(c) : c);
    }
}


std::string Preprocessor::normalizeWhitespace(const std::string& s) const {
    std::string out;
    bool lastSpace = false;
    for (char c : s) {
        if (c == ' ') {
            if (!lastSpace) out += ' ';
            lastSpace = true;
        } else {
            out += c;
            lastSpace = false;
        }
    }

    size_t start = out.find_first_not_of(' ');
    if (start == std::string::npos) return "";
    out = out.substr(start);

    // Xóa toàn bộ space ở cuối
    size_t end = out.find_last_not_of(' ');
    out = out.substr(0, end + 1);
    return out;
}

std::string Preprocessor::cleanLine(const std::string& line) const {
    std::string result;

    for (char c : line) {
        char processed = toLower ? (char)std::tolower(c) : c;

        if (ignoredChars.count(processed)) continue;
        if (delimiters.count(processed)) processed = '\n';

        if (processed == '\n') {
            if (!result.empty() && result.back() == '\n') continue;
        }

    if (processed == ' ' && (result.empty() || result.back() == '\n')) {
        continue;
    }
    
        result += processed;
    }

    result = normalizeWhitespace(result);

    return result;
}

std::vector<std::string> Preprocessor::processFile(
    const std::string& inputFile,
    const std::string& outputFile
) {

    std::ifstream fin(inputFile);
    if (!fin.is_open()) {
        return {};
    }

    std::ofstream fout;
    if (!outputFile.empty()) {
        fout.open(outputFile);
        if (!fout.is_open()) {
            fin.close();
            return {};
        }
    }

    std::vector<std::string> sequences;
    std::string line;
    int lineCount = 0;

    while (std::getline(fin, line)) {
        line = cleanLine(line);
        if (line.empty()) continue;

        ++lineCount;
        sequences.push_back(line);

        if (fout.is_open()) {
            fout << line << '\n';
        }
    }

    fin.close();
    if (fout.is_open()) {
        fout.close();
    }
    return sequences;
}

std::vector<std::string> Preprocessor::filterByRegex(
    const std::string& inputFile,
    const std::string& pattern
) {
    std::vector<std::string> results;

    std::ifstream fin(inputFile);
    if (!fin.is_open()) {
        return results;
    }

    std::regex re;
    try {
        re.assign(pattern);
    } catch (const std::regex_error& e) {
        return results;
    }
    
    std::string line;

    while (std::getline(fin, line)) {
        std::smatch match;

        auto it = line.cbegin();
        while (std::regex_search(it, line.cend(), match, re)) {

            if (match.size() > 1) {
                for (size_t i = 1; i < match.size(); i++) {
                    std::string cap = match[i].str();
                    if (!cap.empty()) {
                        results.push_back(cap);
                    }
                }
            } else {
                std::string full = match[0].str();
                if (!full.empty()) {
                    results.push_back(full);
                }
            }

            it = match.suffix().first;
        }
    }

    fin.close();
    return results;
}

void Preprocessor::exportCollected(const std::string& outputFile, vector<string> collected) {
    
    ofstream fout(outputFile, ios::trunc | ios::binary);
    if (!fout.is_open()) return;


    for (std::string s : collected) {
        if (toLower) {
            for (char& c : s) c = std::tolower(c);
        }
        fout << s << "\n";
    }

    fout.close();
    return ;
}