#ifndef _ANALYSIS_
#define _ANALYSIS_

#include "StatTrie.h"
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <functional>
#include <iostream>
#include <iomanip>
#include <queue>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

/**
 * @brief Class phân tích thống kê cây Trie:
 *  - Tận dụng hàm traverse() của StatTrie
 *  - Tính toán entropy, độ sâu, tỷ lệ anomaly
 *  - Phát hiện và xuất danh sách các chuỗi bất thường
 */
class Analysis {
private:
    
    const StatTrie& trie;       // Trie được phân tích
    double freqThreshold;       // ngưỡng hiếm
    double avrEntropy, avrDepth, expectLen;             // call computeAverages() to compute these
    double stdDeviEntropy, stdDeviDepth, stdDeviLen;    // call computeStandardDeviations() to compute these
    int depthZscore, lengthZ_score, entropyZscore;      // z score thresholds

    // Helpers
    double totalWords() const;
    void writeHeader(ofstream& fout) const;

    void computeAverages();
    void computeStandardDeviations();

public:

    explicit Analysis(const StatTrie& trie, double freq = 0.05, double expect_len_z = 3, double entropy_z = 3, double avr_dep_z = 3);

    void analyze();
    double Z_score (vector<double> elements);
    double entropyOf(const StatTrie::Node* node);
   
    // double computeAverageBranchingFactor() const;
    double computeAnomalyRatio() const;

    // ===== Detection =====
    vector<string> detectAnomalies() const;

    // ===== Reporting =====
    void generateReport(const string& filename = "analysis_report.txt") const;
    
    void exportJSON(const string exportPath = "data/trie.json");
};

#endif
