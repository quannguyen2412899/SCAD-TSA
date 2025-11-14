#include "../include/Preprocessor.h"
#include <cctype>
#include <sstream>

// ===================== Constructor =====================
Preprocessor::Preprocessor(bool toLower, bool chunking, size_t chunkSize) {
    this->toLower = toLower;
    this->chunking = chunking;
    this->chunkSize = chunkSize;
    this->ignoredChars.insert('\r');    // default ignore
    this->delimiters.insert('\n');      // default delim
}

// ===================== Setup methods =====================
void Preprocessor::setIgnoredCharacters(const string chars) {
    if (toLower)
        for (char c : chars) ignoredChars.insert((char)tolower(c));
    else
        for (char c : chars) ignoredChars.insert(c);
}

void Preprocessor::setDelimiters(const string chars) {
    if (toLower)
        for (char c : chars) delimiters.insert((char)tolower(c));
    else
        for (char c : chars) delimiters.insert(c);
}

// ===================== Utility =====================

string Preprocessor::cleanLine(const string& line) const {
    string result;
    if (toLower) {
        for (char c : line) {
            // lower & split & ignore
            c = (char)tolower(c);
            if (delimiters.count(c)) c = '\n';
            else if (ignoredChars.count(c)) continue;
            if (result.empty() && c == '\n') continue;
            if (result.size() > 0 && result.back() == '\n' && c == '\n') continue;
            result.push_back(c);
        }
    }
    else {
        for (char c : line) {
            if (delimiters.count(c)) c = '\n';
            else if (ignoredChars.count(c)) continue;
            if (result.empty() && c == '\n') continue;
            if (result.size() > 0 && result.back() == '\n' && c == '\n') continue;
            result.push_back(c);
        }
    }
    return result;
}

void Preprocessor::splitLine(string& line) const {
    for (char &c : line) {
        if (delimiters.count(c)) c = '\n';
    }
}

vector<string> Preprocessor::chunkString(const string& s, size_t k) {
    vector<string> chunks;
    if (s.empty()) return chunks;
    if (s.size() <= k) { chunks.push_back(s); return chunks; }

    for (size_t i = 0; i + k <= s.size(); ++i)
        chunks.push_back(s.substr(i, k));
    return chunks;
}


// ===================== Main processing =====================

void Preprocessor::run(const string file_in, const string file_out) {

    cout << "Processing data from: " << file_in << endl;

    // file_in = empty string
    if (file_in.empty()) {
        cerr << "[ERROR] No input file determined.";
        return;
    }
    ifstream fin (file_in);
    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file: " << file_in << endl;
        return;
    }
    ofstream fout (file_out, ios::trunc | ios::binary);
    if (!fout.is_open()) {
        cerr << "[ERROR] Error while opening ouput file: " << file_out << endl;
        return;
    }

    string line;
    while (getline(fin, line)) {
        line = cleanLine(line);
        fout << line;
    }

    cout << "Cleaned data is written to: " << file_out << endl;

}


// void Preprocessor::run(DataType datatype) {
//     ifstream fin(filename);
//     if (!fin.is_open()) {
//         cerr << "[ERROR] Cannot open file: " << filename << endl;
//         return;
//     }

//     processedData.clear();
//     string line;
//     DataType dtype = datatype;

//     // Nếu auto -> đoán loại dữ liệu
//     if (dtype == DataType::AUTO) {
//         if (getline(fin, line))
//             dtype = detectDataType(line);
//         else dtype = DataType::TEXT;
//         fin.clear(); fin.seekg(0, ios::beg);
//         cout << "[INFO] Auto-detected datatype: " << toString(dtype) << endl;
//     }

//     size_t lineCount = 0;
//     while (getline(fin, line)) {
//         line = cleanLine(line, toLower, ignoredChars);
//         if (line.empty()) continue;
//         ++lineCount;

//         switch (dtype) {
//             case DataType::TEXT: {
//                 string sentence;
//                 for (char c : line) {
//                     if (delimiters.count(c)) {
//                         if (!sentence.empty()) {
//                             if (chunking)
//                                 for (auto& s : chunkString(sentence, chunkSize))
//                                     processedData.push_back(s);
//                             else processedData.push_back(sentence);
//                             sentence.clear();
//                         }
//                     } else sentence += c;
//                 }
//                 if (!sentence.empty()) processedData.push_back(sentence);
//                 break;
//             }

//             case DataType::LOG:
//             case DataType::DNA: {
//                 if (chunking)
//                     for (auto& s : chunkString(line, chunkSize))
//                         processedData.push_back(s);
//                 else processedData.push_back(line);
//                 break;
//             }

//             case DataType::UNKNOWN:
//             default: {
//                 line = cleanLine(line, toLower, ignoredChars);
//                 if (chunking)
//                     for (auto& s : chunkString(line, chunkSize))
//                         processedData.push_back(s);
//                 else processedData.push_back(line);
//                 cerr << "[WARN] Unknown data type detected. Processed generically." << endl;
//                 break;
//             }
//         }
//     }

//     fin.close();
//     cout << "[INFO] Processed " << processedData.size()
//          << " sequences from " << lineCount << " lines." << endl;
// }
