#include "../include/Analysis.h"

/*
how to use:
Ananlys a(trie);
a.analyze();
a.report();
*/



/**
 * @brief
- phát hiện bất thường cho từng trường hợp: tần suất, độ dài, entropy

- mỗi trường hợp xuất ra csv và json

- xuất csv và json cho toàn bộ trie

- xuất file txt report
*/
void Analysis::report(const string directory) {
    vector<AnomalyEntry> anomalies;

    // bất thường tần suất
    anomalies = detectAnomalies ('f');
    exportAnomaliesToCSV(directory + "rare_frequency.csv");
    exportAnomaliesToJSON(directory + "rare_frequency.json");
    
    // bất thường độ dài
    anomalies = detectAnomalies ('l');
    exportAnomaliesToCSV(directory + "rare_length.csv");
    exportAnomaliesToJSON(directory + "rare_length.json");
    
    // bất thường entropy
    anomalies = detectAnomalies ('e');
    exportAnomaliesToCSV(directory + "rare_entropy.csv");
    exportAnomaliesToJSON(directory + "rare_entropy.json");
    

    // Nếu > 50 node thì không xuất json toàn bộ cây trie
    if (trie.totalNodes() <= 50) exportJSON();
    else cout << "trie too big" << endl;

    // xuất csv cho toàn bộ cây trie
    exportCSV();
}



/**
 * @brief
- duyệt trie, thu thập dữ liệu

- tính toán các thresholds để phát hiện bất thường
*/
void Analysis::analyze(double freqPercentile, double entropyPercentile, double lenPercentile) {
    allEntries.clear();
    // traverseTrie();
    // string prefix;

    auto callback = [&](const StatTrie::Node* node, const std::string &word){
        if (node->isEnd) {
            AnomalyEntry entry;
            entry.word = word;
            entry.count = node->count;
            for (const pair<const char, StatTrie::Node*> &p : node->children) {
                entry.count -= p.second->count; // them method cho node cho de tinh
            }
            entry.freqRate = (double)node->count / trie.totalInsertedWords();
            entry.depth = word.size();
            entry.entropy = computeLocalEntropy(node);

            allEntries.push_back(entry);

            if (lenFreq.count(entry.depth)) lenFreq[entry.depth] += 1;
            else lenFreq[entry.depth] = 1;
            if (entry.freqRate > maxFreqRate) maxFreqRate = entry.freqRate;
            if (entry.entropy > maxEntropy) maxEntropy = entry.entropy;
            if (entry.depth > maxDepth) maxDepth = entry.depth;
        }
    };
    trie.traverse(callback);

    // Compute thresholds based on percentile
    computePercentileThresholds(freqPercentile, entropyPercentile, lenPercentile);

}



/**
* @brief
- Tính điểm cho từng chuỗi trong allEntries, nếu dưới threshold thì coi là bất thường

- Trả về các bất thường
*/
vector<AnomalyEntry> Analysis::detectAnomalies(char mode) {
    
    vector<AnomalyEntry> anomalies;

    if (mode == 'f') {
        for (auto &entry : allEntries) {
            entry.score = entry.freqRate;
            if (entry.score <= freqThreshold) anomalies.push_back(entry);
        }
    }

    else if (mode == 'l') {
        for (auto &entry : allEntries) {
            entry.score = lenFreq[entry.depth];
            if (entry.score <= lenFreqThreshold) anomalies.push_back(entry);
        }
    }

    else if (mode == 'e') {
        for (auto &entry : allEntries) {
            entry.score = entry.entropy;
            if (entry.score <= entropyThreshold) anomalies.push_back(entry);
        }
    }

    else cout << "Mode not supported" << endl;

    // sort anomalies theo score tăng dần (score càng nhỏ càng hiếm)
    sort(anomalies.begin(), anomalies.end(), [](const AnomalyEntry &a, const AnomalyEntry &b){
        return a.score < b.score;
    });

    return anomalies;
}



/**
 * @brief
 * Tính thresholds dựa trên phần vị
 */
void Analysis::computePercentileThresholds(double freqPercentile, double entropyPercentile, double lenPercentile) {
    // frequency threshold
    std::vector<double> freqRates;
    std::vector<double> entropies;
    std::vector<pair<const unsigned, unsigned>> lenFreqs (lenFreq.begin(), lenFreq.end());
    
    for (auto &e : allEntries) {
        freqRates.push_back(e.freqRate);
        entropies.push_back(e.entropy);
    }

    std::sort(freqRates.begin(), freqRates.end());
    std::sort(entropies.begin(), entropies.end());
    std::sort(lenFreqs.begin(), lenFreqs.end(), [] (pair<const unsigned, unsigned> &a, pair<const unsigned, unsigned> &b) {
        return a.second < b.second;
    });
    
    size_t fIdx = (size_t)((freqPercentile/100.0) * freqRates.size());
    size_t eIdx = (size_t)((entropyPercentile/100.0) * entropies.size());
    size_t lIdx = 0;
    for (size_t count = lenFreqs[0].second; count < (lenPercentile/100.0) * trie.totalInsertedWords(); count += lenFreqs[++lIdx].second);

    if (fIdx >= freqRates.size()) fIdx = freqRates.size() - 1;
    if (eIdx >= entropies.size()) eIdx = entropies.size() - 1;
    if (lIdx >= lenFreqs.size()) lIdx = lenFreqs.size() - 1;

    freqThreshold = freqRates[fIdx];
    entropyThreshold = entropies[eIdx];
    lenFreqThreshold = lenFreqs[lIdx].second;
}


/**
 * @brief
 * Tính entropy cục bộ ở một node
 */
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


/**
 * @brief
 * In thống kê ra màn hình
 */
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


/**
 * @brief
 * xuất cây trie ra file json
 */
void Analysis::exportJSON (const string exportPath) {

    ofstream file (exportPath, ios::trunc);
    json jData, jRoot, jLabels;
    count_t id = 0;

    auto collector = [&jRoot, &jLabels, &id] (const StatTrie::Node* node, const string &prefix) {

        if (prefix.empty()) jLabels[0] = "";
        else jLabels[id] = string(1, prefix.back());
        json* j = &jRoot;
        for (char c : prefix) j = &((*j)["children"][string(1, c)]);

        (*j)["children"] = json::object();
        for (const pair<const char, StatTrie::Node*> p : node->children) {
            (*j)["children"][string(1, p.first)] = json::object();
        }
        (*j)["count"] = node->count;
        (*j)["isEnd"] = node->isEnd;
        (*j)["ID"] = id++;

    };

    trie.traverse(collector);
    jData["root"] = jRoot;
    jData["labels"] = jLabels;
    jData["totalUnique"] = trie.totalUniqueWords();
    jData["threshold"] = trie.getAnomalyRate();
    
    file << jData.dump(2);
}