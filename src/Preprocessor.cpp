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
    // Bỏ các ký tự nhiễu phổ biến trong log
    ignoredChars = {'\r', '[', ']', '{', '}', '"', '\'', '(', ')'};
    delimiters = {'\n'};
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
    if (!out.empty() && out.front() == ' ') out.erase(0, 1);
    if (!out.empty() && out.back() == ' ') out.pop_back();
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

    std::cout << "Processing completed: " << sequences.size()
              << " sequences from " << lineCount << " lines.\n";

    return sequences;
}

std::vector<std::string> Preprocessor::filterByRegex(
    const std::string& inputFile,
    const std::string& outputFile,
    const std::string& pattern
) {
    std::vector<std::string> results;

    std::ifstream fin(inputFile);
    if (!fin.is_open()) {
        return results;
    }

    std::ofstream fout(outputFile);
    if (!fout.is_open()) {
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
                        fout << cap << "\n";
                        results.push_back(cap);
                    }
                }
            } else {
                std::string full = match[0].str();
                if (!full.empty()) {
                    fout << full << "\n";
                    results.push_back(full);
                }
            }

            it = match.suffix().first;
        }
    }

    fin.close();
    fout.close();
    std::cout << "Cleaned: " << outputFile << std::endl;

    return results;
}