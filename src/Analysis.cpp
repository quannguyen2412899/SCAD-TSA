#include "../include/Analysis.h"

vector<AnomalyEntry> Analysis::detectAnomalies(double freqPercentile, double entropyPercentile){
    allEntries.clear();
    traverseTrie();

    // Compute thresholds based on percentile
    computePercentileThresholds(freqPercentile, entropyPercentile);

    vector<AnomalyEntry> anomalies;
    for (auto &entry : allEntries) {
        double freqScore = 1.0 - entry.freqRate / maxFreqRate; // 0..1, càng cao càng hiếm
        double entropyScore = (maxEntropy - entry.entropy) / maxEntropy; // 0..1, thấp entropy → cao score
        double depthScore = (double)entry.depth / maxDepth; // normalize depth

        // tổng hợp multi-metric score (cân bằng đơn giản)
        entry.score = freqScore + entropyScore + depthScore;

        // đánh dấu là anomaly nếu vượt threshold (tần suất thấp hoặc entropy thấp)
        if (entry.freqRate <= freqThreshold || entry.entropy <= entropyThreshold) {
            anomalies.push_back(entry);
        }
    }

    // sort anomalies theo score giảm dần
   sort(anomalies.begin(), anomalies.end(), [](const AnomalyEntry &a, const AnomalyEntry &b){
        return a.score > b.score;
    });

    return anomalies;
}

void Analysis::printAnomalies(const vector<AnomalyEntry> &anomalies){
    std::cout << "Word\tCount\tFreqRate\tEntropy\tDepth\tScore\n";
    std::cout << "---------------------------------------------------------------\n";
    for (auto &a : anomalies) {
        std::cout << a.word << "\t"
                    << a.count << "\t"
                    << std::fixed << std::setprecision(5) << a.freqRate << "\t"
                    << a.entropy << "\t"
                    << a.depth << "\t"
                    << a.score << "\n";
    }
}

void Analysis::traverseTrie(){
    string prefix;
    auto callback = [&](const StatTrie::Node* node, const std::string &word){
        if (node->isEnd) {
            AnomalyEntry entry;
            entry.word = word;
            entry.count = node->count;
            entry.freqRate = (double)node->count / trie.totalInsertedWords();
            entry.depth = word.size();
            entry.entropy = computeLocalEntropy(node);

            allEntries.push_back(entry);

            if (entry.freqRate > maxFreqRate) maxFreqRate = entry.freqRate;
            if (entry.entropy > maxEntropy) maxEntropy = entry.entropy;
            if (entry.depth > maxDepth) maxDepth = entry.depth;
        }
    };
    trie.traverse(callback);
}


double Analysis::computeLocalEntropy(const StatTrie::Node* node) {
    if (node->children.empty()) return 0.0;
    double total = 0.0;
    for (auto &p : node->children) total += p.second->count;
    double H = 0.0;
    for (auto &p : node->children) {
        double p_i = p.second->count / total;
        H -= p_i * log2(p_i);
    }
    return H;
}

void Analysis::computePercentileThresholds(double freqPercentile, double entropyPercentile) {
    // frequency threshold
    std::vector<double> freqRates;
    std::vector<double> entropies;
    for (auto &e : allEntries) {
        freqRates.push_back(e.freqRate);
        entropies.push_back(e.entropy);
    }
    std::sort(freqRates.begin(), freqRates.end());
    std::sort(entropies.begin(), entropies.end());

    size_t fIdx = (size_t)((freqPercentile/100.0) * freqRates.size());
    size_t eIdx = (size_t)((entropyPercentile/100.0) * entropies.size());
    if (fIdx >= freqRates.size()) fIdx = freqRates.size() - 1;
    if (eIdx >= entropies.size()) eIdx = entropies.size() - 1;

    freqThreshold = freqRates[fIdx];
    entropyThreshold = entropies[eIdx];
}