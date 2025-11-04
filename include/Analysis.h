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

using namespace std;

/**
 * @brief Class phân tích thống kê cây Trie:
 *  - Tận dụng hàm traverse() của StatTrie
 *  - Tính toán entropy, độ sâu, tỷ lệ anomaly
 *  - Phát hiện và xuất danh sách các chuỗi bất thường
 */
class Analysis {
private:
    const StatTrie& trie; // Trie được phân tích

    // Helpers
    double totalWords() const;
    void writeHeader(ofstream& fout) const;

public:
    friend class StatTrie;
    explicit Analysis(const StatTrie& trie);

    double computeAverageDepth() const;
    double computeAverageBranchingFactor() const;
    double computeAnomalyRatio() const;

    // ===== Detection =====
    vector<string> detectAnomalies() const;

    // ===== Reporting =====
    void generateReport(const string& filename = "analysis_report.txt") const;
};

#endif
