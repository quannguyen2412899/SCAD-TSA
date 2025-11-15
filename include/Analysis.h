#ifndef _ANALYSIS_
#define _ANALYSIS_

#include "StatTrie.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>

struct AnomalyEntry {
    std::string word;
    count_t count;
    double freqRate;
    double entropy;
    unsigned depth;
    double score; // tổng hợp multi-metric
};

class Analysis {
public:
    Analysis(const StatTrie &trie) : trie(trie) {}

    // main function: detect anomalies
    std::vector<AnomalyEntry> detectAnomalies(double freqPercentile = 5.0, 
                                              double entropyPercentile = 5.0);

    void printAnomalies(const std::vector<AnomalyEntry> &anomalies) ;

private:
    const StatTrie &trie;
    std::vector<AnomalyEntry> allEntries;

    double freqThreshold = 0.0;
    double entropyThreshold = 0.0;
    double maxFreqRate = 0.0;
    double maxEntropy = 0.0;
    unsigned maxDepth = 1;

    // traverse trie and collect info
    void traverseTrie() ;
    // compute entropy of children
    double computeLocalEntropy(const StatTrie::Node* node);

    void computePercentileThresholds(double freqPercentile, double entropyPercentile);

};

#endif
