#include "../include/Preprocessor.h"
#include <cctype>

// ===================== Constructor =====================
Preprocessor::Preprocessor(string filename, bool toLower, bool chunking, size_t chunkSize)
    : filename(filename), toLower(toLower), chunking(chunking), chunkSize(chunkSize) {}

// ===================== Setup methods =====================
void Preprocessor::setIgnoredCharacters(const string& chars) {
    for (char c : chars) ignoredChars.insert(c);
}

void Preprocessor::setDelimiters(const string& chars) {
    for (char c : chars) delimiters.insert(c);
}

// ===================== Utility =====================
string Preprocessor::cleanLine(const string& line, bool toLower, const unordered_set<char>& ignored) {
    string result;
    for (char c : line) {
        if (ignored.count(c) || isdigit(c)) continue;
        result += toLower ? (char)tolower(c) : c;
    }
    return result;
}

vector<string> Preprocessor::chunkString(const string& s, size_t k) {
    vector<string> chunks;
    if (s.empty()) return chunks;
    if (s.size() <= k) { chunks.push_back(s); return chunks; }

    for (size_t i = 0; i + k <= s.size(); ++i)
        chunks.push_back(s.substr(i, k));
    return chunks;
}

// ===================== Detect datatype =====================
DataType Preprocessor::detectDataType(const string& sample) {
    bool hasLetters = false, hasDigits = false;
    for (char c : sample) {
        if (isalpha(c)) hasLetters = true;
        if (isdigit(c)) hasDigits = true;
    }

    if (sample.find("INFO") != string::npos || sample.find("ERROR") != string::npos || sample.find("WARN") != string::npos)
        return DataType::LOG;
    if (!hasDigits && sample.find_first_not_of("ACGTacgt") == string::npos)
        return DataType::DNA;
    if (hasLetters)
        return DataType::TEXT;
    return DataType::UNKNOWN;
}

string Preprocessor::toString(DataType t) {
    switch (t) {
        case DataType::TEXT: return "text";
        case DataType::LOG: return "log";
        case DataType::DNA: return "dna";
        case DataType::AUTO: return "auto";
        default: return "unknown";
    }
}

// ===================== Main processing =====================
void Preprocessor::run(DataType datatype) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open file: " << filename << endl;
        return;
    }

    processedData.clear();
    string line;
    DataType dtype = datatype;

    // Nếu auto -> đoán loại dữ liệu
    if (dtype == DataType::AUTO) {
        if (getline(fin, line))
            dtype = detectDataType(line);
        else dtype = DataType::TEXT;
        fin.clear(); fin.seekg(0, ios::beg);
        cout << "[INFO] Auto-detected datatype: " << toString(dtype) << endl;
    }

    size_t lineCount = 0;
    while (getline(fin, line)) {
        line = cleanLine(line, toLower, ignoredChars);
        if (line.empty()) continue;
        ++lineCount;

        switch (dtype) {
            case DataType::TEXT: {
                string sentence;
                for (char c : line) {
                    if (delimiters.count(c)) {
                        if (!sentence.empty()) {
                            if (chunking)
                                for (auto& s : chunkString(sentence, chunkSize))
                                    processedData.push_back(s);
                            else processedData.push_back(sentence);
                            sentence.clear();
                        }
                    } else sentence += c;
                }
                if (!sentence.empty()) processedData.push_back(sentence);
                break;
            }

            case DataType::LOG:
            case DataType::DNA: {
                if (chunking)
                    for (auto& s : chunkString(line, chunkSize))
                        processedData.push_back(s);
                else processedData.push_back(line);
                break;
            }

            case DataType::UNKNOWN:
            default: {
                line = cleanLine(line, toLower, ignoredChars);
                if (chunking)
                    for (auto& s : chunkString(line, chunkSize))
                        processedData.push_back(s);
                else processedData.push_back(line);
                cerr << "[WARN] Unknown data type detected. Processed generically." << endl;
                break;
            }
        }
    }

    fin.close();
    cout << "[INFO] Processed " << processedData.size()
         << " sequences from " << lineCount << " lines." << endl;
}
