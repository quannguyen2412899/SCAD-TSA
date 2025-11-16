#include "../include/Analysis.h"

/*
how to use:
Ananlys a(trie);
a.analyze();
a.report();
*/



void Analysis::report(const string directory) {

    // bất thường tần suất
    detectAnomalies ('f');
    exportAnomaliesToCSV(directory + "/rare_frequency.csv");
    exportAnomaliesToJSON(directory + "/rare_frequency.json");
    
    // bất thường độ dài
    detectAnomalies ('l');
    exportAnomaliesToCSV(directory + "/rare_length.csv");
    exportAnomaliesToJSON(directory + "/rare_length.json");
    
    // bất thường entropy
    detectAnomalies ('e');
    exportAnomaliesToCSV(directory + "/rare_entropy.csv");
    exportAnomaliesToJSON(directory + "/rare_entropy.json");
    

    // Nếu > 50 node thì không xuất json toàn bộ cây trie
    if (trie.totalNodes() <= 256) exportJSON(trie);
    else cout << "trie too big" << endl;

    // xuất csv cho toàn bộ cây trie
    exportCSV();
}




void Analysis::analyze(double freqPercentile, double entropyPercentile, double lenPercentile) {
    allEntries.clear();
    
    auto callback = [&](const StatTrie::Node* node, const std::string &word){
        double localEntropy = computeLocalEntropy(node);

        if (localEntropy > 0) {
            AnomalyEntry entry;
            entry.isWord = false;
            entry.word = word;
            entry.count = node->count;
            entry.freqRate = (double)node->count / trie.totalInsertedWords();
            entry.depth = word.size();
            entry.entropy = localEntropy;

            allEntries.push_back(entry);

            if (entry.entropy > maxEntropy) maxEntropy = entry.entropy;
        }

        if (node->isEnd) {
            AnomalyEntry entry;
            entry.isWord = true;
            entry.word = word;
            entry.count = node->countEnd();
            entry.freqRate = (double)entry.count / trie.totalInsertedWords();
            entry.depth = word.size();
            entry.entropy = localEntropy;

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




vector<AnomalyEntry> Analysis::detectAnomalies(char mode) {
    
    anomalies.clear();

    if (mode == 'f') {
        for (auto &entry : allEntries) {
            if (!entry.isWord) continue;
            entry.score = entry.freqRate;
            if (entry.score <= freqThreshold) anomalies.push_back(entry);
        }
    }

    else if (mode == 'l') {
        for (auto &entry : allEntries) {
            if (!entry.isWord) continue;
            entry.score = lenFreq[entry.depth];
            if (entry.score <= lenFreqThreshold) anomalies.push_back(entry);
        }
    }

    else if (mode == 'e') {
        for (auto &entry : allEntries) {
            if (entry.isWord) continue;
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





void Analysis::computePercentileThresholds(double freqPercentile, double entropyPercentile, double lenPercentile) {
    // frequency threshold
    std::vector<double> freqRates;
    std::vector<double> entropies;
    std::vector<pair<unsigned, unsigned>> lenFreqs;

    size_t s = lenFreq.size();
    for (pair<const unsigned, unsigned> &p : lenFreq) 
        lenFreqs.push_back(pair<unsigned, unsigned> (p.first, p.second));
    
    for (auto &e : allEntries) {
        if (e.isWord) freqRates.push_back(e.freqRate);
        if (!e.isWord) entropies.push_back(e.entropy);
    }

    std::sort(freqRates.begin(), freqRates.end());
    std::sort(entropies.begin(), entropies.end());
    std::sort(lenFreqs.begin(), lenFreqs.end(), [] (pair<unsigned, unsigned> &a, pair<unsigned, unsigned> &b) {
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




void Analysis::exportJSON (const StatTrie &trie, const string exportFile) {

    ofstream file (exportFile, ios::trunc);
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


void Analysis::exportAnomaliesToJSON(const string exportFile) const {
    StatTrie rareTrie;
    for (const AnomalyEntry &entry : anomalies) rareTrie.insert(entry.word, entry.count);
    exportJSON(rareTrie, exportFile);
}


void Analysis::exportAnomaliesToCSV(const string exportFile) const {
    ofstream fout(exportFile, ios::trunc);
    fout << "String,Frequency,Length,Entropy,Rate,Status\n";
    for (const AnomalyEntry& entry : anomalies) {
        string status = "anomaly";
        if (entry.freqRate <= freqThreshold) status += "-fequency";
        if (entry.isWord && lenFreq.at(entry.depth) <= lenFreqThreshold) status += "-length";
        if (!entry.isWord && entry.entropy <= entropyThreshold) status += "-entropy";

        fout << escapeCSV(entry.word) << ','
             << entry.count << ',' 
             << entry.depth << ',' 
             << entry.entropy << ','
             << entry.freqRate << ','
             << status << '\n';
    }
}



void Analysis::exportCSV(const string exportFile) {
    ofstream fout(exportFile, ios::trunc);
    fout << "String,Frequency,Length,Entropy,Rate,Status\n";
    for (AnomalyEntry& entry : allEntries) {
        string status="";
        if (entry.freqRate <= freqThreshold) status += "-fequency";
        if (entry.isWord && lenFreq[entry.depth] <= lenFreqThreshold) status += "-length";
        if (!entry.isWord && entry.entropy <= entropyThreshold) status += "-entropy";

        if(status.empty()) status = "normal";
        else status = "anomaly" + status;

        fout << escapeCSV(entry.word) << ','
             << entry.count << ',' 
             << entry.depth << ',' 
             << entry.entropy << ','
             << entry.freqRate << ','
             << status << '\n';
    }
}


string Analysis::escapeCSV(const std::string& s) {
    bool needQuotes = s.find(',') != std::string::npos ||
                      s.find('"') != std::string::npos ||
                      s.find('\n') != std::string::npos;

    std::string out = s;
    // replace " with ""
    size_t pos = 0;
    while ((pos = out.find('"', pos)) != std::string::npos) {
        out.replace(pos, 1, "\"\"");
        pos += 2;
    }

    if (needQuotes) {
        return "\"" + out + "\"";
    }
    return out;
}