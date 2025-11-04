#include "../include/Analysis.h"

/* ===================== Constructor ===================== */
Analysis::Analysis(const StatTrie& trie) : trie(trie) {}

/* ===================== Helpers ===================== */
double Analysis::totalWords() const {
    return (double)trie.totalInsertedWords();
}

void Analysis::writeHeader(ofstream& fout) const {
    fout << "=== Trie Statistical Analysis Report ===\n";
    fout << "Total Nodes: " << trie.totalNodes() << "\n";
    fout << "Unique Sequences: " << trie.totalUniqueWords() << "\n";
    fout << "Inserted Sequences: " << trie.totalInsertedWords() << "\n";
    fout << "Inserted Chars:" << trie.totalInsertedCharacters() << "\n";
    fout << "Anomaly Threshold: " << trie.getAnomalyRate() << "\n\n";
}


double Analysis::computeAverageDepth() const {
    double totalDepth = 0.0;
    double totalWordsCounted = 0.0;

    trie.traverse([&](const StatTrie::Node* node, const string& prefix) {
        if (node->isEnd) {
            totalDepth += prefix.size();
            totalWordsCounted++;
        }
    });
    return totalWordsCounted > 0 ? totalDepth / totalWordsCounted : 0.0;
}

double Analysis::computeAverageBranchingFactor() const {
    double totalBranches = 0.0;
    double totalNodes = 0.0;

    trie.traverse([&](const StatTrie::Node* node, const string&) {
        totalBranches += node->children.size();
        totalNodes++;
    });
    return totalNodes > 0 ? totalBranches / totalNodes : 0.0;
}

// Tính tỷ lệ chuỗi bất thường trên tổng chuỗi (dựa vào tần suất)
double Analysis::computeAnomalyRatio() const {
    double total = totalWords();
    if (total == 0) return 0.0;

    int anomalyCount = 0, uniqueCount = 0;

    trie.traverse([&](const StatTrie::Node* node, const string&) {
        if (node->isEnd) {
            uniqueCount++;
            if ((double)(node->count / total) < trie.getAnomalyRate())
                anomalyCount++;
        }
    });

    return uniqueCount > 0 ? (double)anomalyCount / uniqueCount : 0.0;
}

//Lưu các chuỗi bất thường (dựa trên tần suất/độ sâu/độ thưa thớt nhánh của node)
vector<string> Analysis::detectAnomalies() const {
    vector<string> anomalies;

    double total = totalWords();
    if (total == 0) return anomalies;

    double avgDepth = computeAverageDepth();
    double avgBranch = computeAverageBranchingFactor();

    trie.traverse([&](const StatTrie::Node* node, const string& prefix) {
        if (node->isEnd) {
            double freq = node->count / total;
            bool rare = freq < trie.getAnomalyRate();
            bool deep = prefix.size() > avgDepth * 2.5;
            bool sparse = node->children.size() < avgBranch / 2.0;
            bool shallow = prefix.size() < avgDepth * 0.5;

            /*if (rare || deep || sparse)*/   if(deep)
                anomalies.push_back(prefix);
        }
    });

    return anomalies;
}

/* ===================== Generate Report ===================== */
void Analysis::generateReport(const string& filename) const {
    ofstream fout(filename);
    if (!fout.is_open()) {
        cerr << "[ERROR] Cannot open report file: " << filename << endl;
        return;
    }

    fout << fixed << setprecision(4);
    writeHeader(fout);

    double avgDepth = computeAverageDepth();
    double avgBranch = computeAverageBranchingFactor();
    double anomalyRatio = computeAnomalyRatio();

    fout << "--- Statistical Summary ---\n";
    fout << "Average Depth: " << avgDepth << "\n";
    fout << "Average Branching Factor: " << avgBranch << "\n";
    fout << "Anomaly Ratio: " << anomalyRatio * 100 << "%\n\n";

    auto anomalies = detectAnomalies();
    fout << "--- Detected Anomalies ---\n";
    fout << "Count: " << anomalies.size() << "\n";
    fout << "Ratio: " << (double)anomalies.size() / trie.totalUniqueWords() * 100 << "%\n\n";
    fout << "Examples:\n";
    for (size_t i = 0; i < anomalies.size(); ++i)
        fout << anomalies[i] << "\n";

    fout.close();
    cout << "[INFO] Analysis report saved to " << filename << endl;
}
