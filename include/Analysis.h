#ifndef _ANALYSIS_
#define _ANALYSIS_

#include "StatTrie.h"


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
    std::vector<AnomalyEntry> freqAnomalies;
    std::vector<AnomalyEntry> lenAnomalies;
    std::vector<AnomalyEntry> entropyAnomalies;
    
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
    std::unordered_map<unsigned, unsigned> lenFreq;

    double freqAnomaliesRate;
    double lenAnomaliesRate;
    double entropyAnomaliesRate;
    
    double computeLocalEntropy(const Node* node);
    void computePercentileThresholds();
    void getExtremum();
    void detectAnomalies();

    std::string escapeCSV(const std::string& s) const;
    void writeCSVToFilestream (std::ofstream& file, const std::vector<AnomalyEntry>& anomalies) const;
    // void exportJSON(const StatTrie &_trie, const string exportFile = "data/output/trie.json") const;


    public:

    Analysis(double freqPercentile = 5, double lenPercentile = 5, double entropyPercentile = 95);

    void collectStatistics(const StatTrie* _trie);
    void markAnomalyNodes(std::unordered_set<const Node*> &anomalyNodes, const char mode = 'a') const;

    // xuất report, json, csv
    // void report(const std::string directory = "data/output") const;
    void exportReport(const std::string exportFile = "data/output/overall_report.txt") const;
    void exportCSV(const std::string exportFile = "data/output/all_entries.csv", char mode = 'a') const;

    // void exportAnomaliesToCSV(const std::string exportFile = "data/output") const;
};

#endif
