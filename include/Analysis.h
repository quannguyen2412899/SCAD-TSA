#ifndef _ANALYSIS_
#define _ANALYSIS_

#include "StatTrie.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <unordered_set>
#include <fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct AnomalyEntry {
    std::string word;
    count_t count;
    double freqRate;
    double entropy;
    unsigned depth;
    double score; // tổng hợp multi-metric
    bool isWord;
};

class Analysis {
public:
    Analysis(const StatTrie &trie) : trie(trie) {}

    void analyze(double freqPercentile = 5, double entropyPercentile = 5, double lenPercentile = 5);
    vector<AnomalyEntry> detectAnomalies(char mode);

    // xuất report, json, csv
    void report(const string directory = "data/output");
    
    static string escapeCSV(const std::string& s);
    static void exportJSON(const StatTrie& trie, const string exportFile = "data/output/trie.json");
    void exportCSV(const string exportFile = "data/output/all_entries.csv");
    void exportAnomaliesToJSON(const string exportFile) const;
    void exportAnomaliesToCSV(const string exportFile) const;

    void printAnomalies(const std::vector<AnomalyEntry> &anomalies) ;

private:
    const StatTrie &trie;
    std::vector<AnomalyEntry> allEntries;
    vector<AnomalyEntry> anomalies;

    double freqThreshold = 0.0;
    double entropyThreshold = 0.0;
    double lenFreqThreshold = 0.0;

    double maxFreqRate = 0.0;
    double maxEntropy = 0.0;
    unsigned maxDepth = 1;
    unsigned mostPopDepth = 0;
    
    unordered_map<unsigned, unsigned> lenFreq;

    double computeLocalEntropy(const StatTrie::Node* node);
    void computePercentileThresholds(double freqPercentile, double entropyPercentile, double lenPercentile);

};

#endif
