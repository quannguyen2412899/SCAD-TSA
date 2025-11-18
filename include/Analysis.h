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

Ananlys a();
a.collectStatistics(trie);
a.report();
*/
class Analysis {

private:

    const StatTrie* trie;
    // unordered_set<const StatTrie::Node*> anomalyNodes;
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
    
    unsigned maxFreq;
    unsigned minFreq;
    unsigned maxDepth;
    unsigned minDepth;
    double maxEntropy;
    double minEntropy;
    unordered_map<unsigned, unsigned> lenFreq;

    double freqAnomaliesRate;
    double lenAnomaliesRate;
    double entropyAnomaliesRate;
    
    double computeLocalEntropy(const StatTrie::Node* node);
    void computePercentileThresholds();
    void getExtremum();
    void detectAnomalies();

    string escapeCSV(const std::string& s) const;
    void writeCSVToFilestream (ofstream& file, const vector<AnomalyEntry>& anomalies) const;
    // void exportJSON(const StatTrie &_trie, const string exportFile = "data/output/trie.json") const;


    public:

    Analysis(double freqPercentile = 5, double lenPercentile = 95, double entropyPercentile = 5);

    void collectStatistics(const StatTrie* _trie);
    void markAnomalyNodes(unordered_set<const StatTrie::Node*> &anomalyNodes, const char mode = 'a') const;

    // xuất report, json, csv
    void report(const string directory = "data/output") const;
    void exportReport(const string exportFile = "data/output/overall_report.txt") const;
    
    void exportCSV(const string exportFile = "data/output/all_entries.csv") const;
    void exportAnomaliesToCSV(const string directory = "data/output") const;
    // void exportAnomaliesToJSON(const string directory = "data/output") const;
    // void exportJSON(const string exportFile = "data/output/trie.json") const;

    // void printAnomalies(const std::vector<AnomalyEntry> &anomalies) const;

};

#endif
