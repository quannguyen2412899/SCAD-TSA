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


struct AnomalyEntry {
    std::string word;
    unsigned count;
    double freqRate;
    double entropy;
    unsigned depth;
    double score; // tổng hợp multi-metric
    bool isWord;
};


/**
 * @brief
 * How to use:

Ananlys a(trie);
a.collectStatistics();
a.report();

* What's next:
 remove trie as an attribute
 instead read from an outside trie?

*/
class Analysis {

private:

    const StatTrie &trie;
    std::vector<AnomalyEntry> allEntries;
    vector<AnomalyEntry> freqAnomalies;
    vector<AnomalyEntry> lenAnomalies;
    vector<AnomalyEntry> entropyAnomalies;
    
    double freqPercentile;
    double entropyPercentile;
    double lenPercentile;

    double freqThreshold;
    double entropyThreshold;
    double lenFreqThreshold;
    
    unsigned totalInsertedWords;
    unsigned totalUniqueWords;
    unsigned totalNodes;
    unsigned totalUniqueWordChar;
    
    double maxEntropy;
    double minEntropy;
    unsigned maxFreq;
    unsigned minFreq;
    unsigned maxDepth;
    unsigned minDepth;
    unsigned mostPopLength;
    unsigned leastPopLength;
    unordered_map<unsigned, unsigned> lenFreq;

    double computeLocalEntropy(const StatTrie::Node* node);
    void computePercentileThresholds(double freqPercentile, double entropyPercentile, double lenPercentile);
    string escapeCSV(const std::string& s) const;
    void writeToFilestream (ofstream& file, const vector<AnomalyEntry>& anomalies) const;

    void detectAnomalies();


    public:

    Analysis(const StatTrie &trie, double freqPercentile = 5, double entropyPercentile = 5, double lenPercentile = 5);

    void collectStatistics();

    // xuất report, json, csv
    void report(const string directory = "data/output");

    void exportReport(const string exportFile = "data/output/overall_report.txt") const;
    
    void exportCSV(const string exportFile = "data/output/all_entries.csv") const;
    void exportAnomaliesToJSON(const string directory = "data/output") const;

    static void exportJSON(const StatTrie &trie, const string exportFile = "data/output/trie.json");
    void exportJSON(const string exportFile = "data/output/trie.json");
    void exportAnomaliesToCSV(const string directory = "data/output") const;

    void printAnomalies(const std::vector<AnomalyEntry> &anomalies) const;

};

#endif
