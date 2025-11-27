#include "Analysis.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <vector>
#include <cstdlib> // std::stod, std::exit

using namespace std;

// Định nghĩa tên file output mặc định cho JSON
const string FN_JSON_COMPLETE = "complete_trie.json";
const string FN_JSON_PARTIAL  = "partial_trie.json";
const string FN_JSON_FREQ     = "freq_anomalies.json";
const string FN_JSON_LEN      = "len_anomalies.json";
const string FN_JSON_ENTROPY  = "entropy_anomalies.json";

void printHelp();

// Hàm tiện ích kiểm tra tiền tố chuỗi
bool startsWith(const string& str, const string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

int main(int argc, char *argv[]) {

    // Giữ nguyên thông báo khởi động (có thể bỏ nếu muốn im lặng hoàn toàn)
    cerr << "========== Analyze and report ==========" << endl;

    if (argc == 2 && string(argv[1]) == "--help") {
        printHelp();
        return 0;
    }

    if (argc < 3) {
        cerr << "[ERROR] Expect: analyze <input_file> <output_dir> [flags]\nRun 'analyze --help' for usage info" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputDir = argv[2];

    /* Check paths' existence */
    if (!filesystem::exists(inputFile)) {
        cerr << "[ERROR] Input file '" <<  inputFile << "' does not exist" << endl;
        return 1;
    }
    if (!filesystem::exists(outputDir)) {
        cerr << "[ERROR] Output directory '" << outputDir << "' does not exist" << endl;
        return 1;
    }

    ifstream fin (inputFile);
    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file at '" << inputFile << "'\n";
        return 1;
    }

    /* --- Configuration Variables (Default Values) --- */
    // Mặc định: 5 (freq thấp), 5 (len thấp), 95 (entropy cao)
    double valPercFreq = 5.0;
    double valPercLen = 5.0;
    double valPercEntropy = 95.0;

    /* --- JSON Flags (Booleans) --- */
    bool doJsonComplete = false;
    bool doJsonPartial  = false;
    bool doJsonFreq     = false;
    bool doJsonLen      = false;
    bool doJsonEntropy  = false;

    /* --- Parse Flags --- */
    for (int i = 3; i < argc; i++) {
        string arg = argv[i];

        // 1. Parsing Percentile Flags (Value flags)
        if (startsWith(arg, "--perc-freq=")) {
            try {
                valPercFreq = stod(arg.substr(12)); // Length of "--perc-freq=" is 12
            } catch (...) {
                cerr << "[ERROR] Invalid value for --perc-freq: " << arg << endl;
                return 1;
            }
        }
        else if (startsWith(arg, "--perc-len=")) {
            try {
                valPercLen = stod(arg.substr(11)); // Length of "--perc-len=" is 11
            } catch (...) {
                cerr << "[ERROR] Invalid value for --perc-len: " << arg << endl;
                return 1;
            }
        }
        else if (startsWith(arg, "--perc-entropy=")) {
            try {
                valPercEntropy = stod(arg.substr(15)); // Length of "--perc-entropy=" is 15
            } catch (...) {
                cerr << "[ERROR] Invalid value for --perc-entropy: " << arg << endl;
                return 1;
            }
        }
        // 2. Parsing JSON Export Flags (Boolean flags)
        else if (arg == "--json-complete") doJsonComplete = true;
        else if (arg == "--json-partial")  doJsonPartial = true;
        else if (arg == "--json-freq")     doJsonFreq = true;
        else if (arg == "--json-len")      doJsonLen = true;
        else if (arg == "--json-entropy")  doJsonEntropy = true;
        else {
            cerr << "[ERROR] Invalid flag: " << arg << "\nRun 'analyze --help' for usage info\n";
            return 1;
        }
    }

    cout << valPercFreq << ' ' << valPercLen << ' ' << valPercEntropy << endl;
    // return 0;

    /* Build trie */
    StatTrie trie;
    string line;
    while (getline(fin, line)) trie.insert(line);

    /* Analyze trie */
    Analysis a(valPercFreq, valPercLen, valPercEntropy);
    a.collectStatistics(&trie);

    /* Output Reports & CSV */
    
    a.exportReport(outputDir + "/overall_report.txt");

    a.exportCSV(outputDir + "/all_entries.csv");
    // Sử dụng ký tự hằng số 'f', 'l', 'e' như code cũ (hoặc hằng số nếu đã define)
    a.exportCSV(outputDir + "/frequency_anomalies.csv", 'f');
    a.exportCSV(outputDir + "/length_anomalies.csv", 'l');
    a.exportCSV(outputDir + "/entropy_anomalies.csv", 'e');


    /* Output JSONs */
    // Logic mới: Sử dụng bool flags và đường dẫn cố định
    
    // 1. Complete & Partial (Dùng chung set anomaly tổng hợp)
    if (doJsonComplete || doJsonPartial) {
        unordered_set<const Node*> allAbnormalNodes;
        a.markAnomalyNodes(allAbnormalNodes); // Mặc định là mark all types

        if (doJsonComplete) {
            string path = outputDir + "/" + FN_JSON_COMPLETE;
            trie.exportAllJSON(path, allAbnormalNodes);
        }
        if (doJsonPartial) {
            string path = outputDir + "/" + FN_JSON_PARTIAL;
            trie.exportPartialJSON(path, allAbnormalNodes);
        }
    }

    // 2. Frequency Anomalies Only
    if (doJsonFreq) {
        unordered_set<const Node*> freqNodes;
        a.markAnomalyNodes(freqNodes, 'f');
        string path = outputDir + "/" + FN_JSON_FREQ;
        trie.exportPartialJSON(path, freqNodes);
    }

    // 3. Length Anomalies Only
    if (doJsonLen) {
        unordered_set<const Node*> lenNodes;
        a.markAnomalyNodes(lenNodes, 'l');
        string path = outputDir + "/" + FN_JSON_LEN;
        trie.exportPartialJSON(path, lenNodes);
    }

    // 4. Entropy Anomalies Only
    if (doJsonEntropy) {
        unordered_set<const Node*> entropyNodes;
        a.markAnomalyNodes(entropyNodes, 'e');
        string path = outputDir + "/" + FN_JSON_ENTROPY;
        trie.exportPartialJSON(path, entropyNodes);
    }

    return 0;
}

void printHelp() {
    cout << "Usage: analyze <input_file> <output_dir> [flags]\n\n"
         << "Configuration flags:\n"
         << "  --perc-freq=<val>      Percentile threshold for Frequency (Low, default: 5)\n"
         << "  --perc-len=<val>       Percentile threshold for Length (Low, default: 5)\n"
         << "  --perc-entropy=<val>   Percentile threshold for Entropy (High, default: 95)\n\n"
         << "JSON export flags (Outputs saved to <output_dir>):\n"
         << "  --json-complete        Export " << FN_JSON_COMPLETE << "\n"
         << "  --json-partial         Export " << FN_JSON_PARTIAL << " (trimmed)\n"
         << "  --json-freq            Export " << FN_JSON_FREQ << "\n"
         << "  --json-len             Export " << FN_JSON_LEN << "\n"
         << "  --json-entropy         Export " << FN_JSON_ENTROPY << "\n"
         << "\nOther flags:\n"
         << "  --help                 Show this help message\n";
}

// Compile: g++ -std=c++17 -Iinclude -o bin/analyze src/analyze.cpp src/Analysis.cpp src/StatTrie.cpp