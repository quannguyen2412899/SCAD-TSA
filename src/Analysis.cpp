#include "../include/Analysis.h"
using json = nlohmann::json;


/* ==================== Constructor ==================== */

Analysis::Analysis(const StatTrie &trie, double freqPercentile, double entropyPercentile, double lenPercentile) : 
    trie(trie),
    freqThreshold(0), entropyThreshold(0), lenFreqThreshold(0),
    totalInsertedWords(0), totalUniqueWords(0), totalNodes(0), totalUniqueWordChar(0),
    maxFreq(0), minFreq(0),
    maxEntropy(0), minEntropy(0),
    maxDepth(0), minDepth(0),
    mostPopLength(0), leastPopLength(0) {}

    
/* ==================== Helper: Compute entropy ==================== */

double Analysis::computeLocalEntropy(const StatTrie::Node* node) {
    // if (node->children.empty()) return 0.0;
    double total = node->count;

    double H = 0.0;
    for (auto &p : node->children) {
        double p_i = p.second->count / total;
        H -= p_i * log2(p_i);
    }
    double p_end = node->countEnd() / total;
    if (node->isEnd) H -= p_end * log2(p_end);

    return H;
}


/* ==================== Traverse Trie and collect statistics ==================== */

void Analysis::collectStatistics() {
    allEntries.clear();
    
    auto callback = [&](const StatTrie::Node* node, const std::string &word){

        ++totalNodes;
        double localEntropy = computeLocalEntropy(node);

        if (localEntropy > 0) {
            AnomalyEntry entry;
            entry.isWord = false;
            entry.word = word;
            entry.count = node->count;
            entry.freqRate = (double)entry.count / trie.totalInsertedWords();
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

            if (entry.count > maxFreq) maxFreq = entry.count;
            if (entry.count < minFreq) minFreq = entry.count;
            if (entry.entropy > maxEntropy) maxEntropy = entry.entropy;
            if (entry.entropy < minEntropy) minEntropy = entry.entropy;
            if (entry.depth > maxDepth) maxDepth = entry.depth;
            if (entry.depth < minDepth) minDepth = entry.depth;
        }
    };
    trie.traverse(callback);

    // Compute thresholds based on percentile
    computePercentileThresholds(freqPercentile, entropyPercentile, lenPercentile);

    for (AnomalyEntry& e : allEntries) {
        totalInsertedWords += e.count;
        totalUniqueWords++;
        totalUniqueWordChar += e.depth;
    }

    detectAnomalies();
}


void Analysis::computePercentileThresholds(double freqPercentile, double entropyPercentile, double lenPercentile) {

    std::vector<unsigned> freqs;
    std::vector<double> entropies;
    std::vector<pair<unsigned, unsigned>> lenFreqs;

    for (pair<const unsigned, unsigned> &p : lenFreq) 
        lenFreqs.push_back(pair<unsigned, unsigned> (p.first, p.second));
    
    for (auto &e : allEntries) {
        if (e.isWord) freqs.push_back(e.count);
        if (!e.isWord) entropies.push_back(e.entropy);
    }

    std::sort(freqs.begin(), freqs.end());
    std::sort(entropies.begin(), entropies.end());
    std::sort(lenFreqs.begin(), lenFreqs.end(), [] (pair<unsigned, unsigned> &a, pair<unsigned, unsigned> &b) {
        return a.second < b.second;
    });

    mostPopLength = lenFreqs.front().first;
    leastPopLength = lenFreqs.back().first;
    
    size_t fIdx = (size_t)((freqPercentile/100.0) * freqs.size());
    size_t eIdx = (size_t)((entropyPercentile/100.0) * entropies.size());
    size_t lIdx = 0;
    for (size_t count = lenFreqs[0].second; count < (lenPercentile/100.0) * trie.totalInsertedWords(); count += lenFreqs[++lIdx].second);

    if (fIdx >= freqs.size()) fIdx = freqs.size() - 1;
    if (eIdx >= entropies.size()) eIdx = entropies.size() - 1;
    if (lIdx >= lenFreqs.size()) lIdx = lenFreqs.size() - 1;

    freqThreshold = freqs[fIdx];
    entropyThreshold = entropies[eIdx];
    lenFreqThreshold = lenFreqs[lIdx].second;

    detectAnomalies();
}



/* ==================== Export report, csv and json ====================*/

void Analysis::report(const string directory) {

    exportAnomaliesToCSV();
    exportAnomaliesToJSON();
    // Nếu > 50 node thì không xuất json toàn bộ cây trie
    if (trie.totalNodes() <= 256) exportJSON(trie);
    else cout << "trie too big" << endl;
    // xuất csv cho toàn bộ cây trie
    exportCSV();


}


/* ==================== Detect anomalies by frequency/length/entropy ==================== */

void Analysis::detectAnomalies() {
    
    freqAnomalies.clear();
    lenAnomalies.clear();
    entropyAnomalies.clear();

    for (auto &entry : allEntries) {
        if (entry.isWord) {
            if (entry.count <= freqThreshold) {
                freqAnomalies.push_back(entry);
                freqAnomalies.back().score = entry.count;
            }
            if (lenFreq[entry.depth] <= lenFreqThreshold) {
                lenAnomalies.push_back(entry);
                lenAnomalies.back().score = lenFreq[entry.depth];
            }
        }
        else if (entry.entropy <= entropyThreshold) {
            entropyAnomalies.push_back(entry);
            entropyAnomalies.back().score = entry.entropy;
        }
    }

    auto compare = [](const AnomalyEntry &a, const AnomalyEntry &b) {
        return a.score < b.score;
    };
    // sort anomalies theo score tăng dần (score càng nhỏ càng hiếm)
    sort(freqAnomalies.begin(), freqAnomalies.end(), compare);
    sort(lenAnomalies.begin(), lenAnomalies.end(), compare);
    sort(entropyAnomalies.begin(), entropyAnomalies.end(), compare);

    // return anomalies;
}



/* ==================== Print anomalies to screen ==================== */

void Analysis::printAnomalies(const vector<AnomalyEntry> &anomalies) const {
    // std::cout << "Word\tCount\tFreqRate\tEntropy\tDepth\tScore\n";
    std::cout << "String\tCount\tFreqRate\tEntropy\tDepth\n";
    std::cout << "---------------------------------------------------------------\n";
    for (auto &a : anomalies) {
        std::cout << a.word << "\t"
                    << a.count << "\t"
                    << std::fixed << std::setprecision(5) << a.freqRate << "\t"
                    << a.entropy << "\t"
                    << a.depth << "\n";
                    // << a.score << "\n";
    }
}


/* ==================== Export to json ==================== */

void Analysis::exportJSON(const string exportFile) {
    exportJSON(trie);
}


void Analysis::exportJSON (const StatTrie &trie, const string exportFile) {

    ofstream file (exportFile, ios::trunc);
    json jData, jRoot, jLabels;
    count_t id = 0;

    auto collector = [&] (const StatTrie::Node* node, const string &prefix) {

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


void Analysis::exportAnomaliesToJSON(const string directory) const {
    StatTrie rareTrie;
    for (const AnomalyEntry &entry : freqAnomalies) rareTrie.insert(entry.word, entry.count);
    exportJSON(rareTrie, directory+"/rare_frequency.json");

    rareTrie.clear();
    for (const AnomalyEntry &entry : lenAnomalies) rareTrie.insert(entry.word, entry.count);
    exportJSON(rareTrie, directory+"/rare_length.json");
    
    rareTrie.clear();
    for (const AnomalyEntry &entry : entropyAnomalies) rareTrie.insert(entry.word, entry.count);
    exportJSON(rareTrie, directory+"/rare_entropy.json");   
}


/* ==================== Export to csv ==================== */

// Helper: write csv content to file stream
void Analysis::writeToFilestream (ofstream& file, const vector<AnomalyEntry>& anomalies)  const{
    file << "String,Frequency,Length,Entropy,Rate,Status\n";
    for (const AnomalyEntry& entry : anomalies) {
        string status = "anomaly";
        if (entry.count <= freqThreshold) status += "-fequency";
        if (entry.isWord && lenFreq.at(entry.depth) <= lenFreqThreshold) status += "-length";
        if (!entry.isWord && entry.entropy <= entropyThreshold) status += "-entropy";

        file << escapeCSV(entry.word) << ','
             << entry.count << ',' 
             << entry.depth << ',' 
             << entry.entropy << ','
             << entry.freqRate << ','
             << status << '\n';
    }
}


void Analysis::exportAnomaliesToCSV(const string directory) const {
    ofstream fout(directory+"/rare_frequency.csv", ios::trunc);
    writeToFilestream(fout, freqAnomalies);
    fout.close();

    fout.open(directory+"/rare_length.csv", ios::trunc);
    writeToFilestream(fout, lenAnomalies);
    fout.close();

    fout.open(directory+"/rare_length.csv", ios::trunc);
    writeToFilestream(fout, entropyAnomalies);    
    fout.close();
}


void Analysis::exportCSV(const string exportFile) const {
    ofstream fout(exportFile, ios::trunc);
    fout << "String,Frequency,Length,Entropy,Rate,Status\n";
    for (const AnomalyEntry& entry : allEntries) {
        string status="";
        if (entry.count <= freqThreshold) status += "-fequency";
        if (entry.isWord && lenFreq.at(entry.depth) <= lenFreqThreshold) status += "-length";
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


/* ==================== Helper: format string to put in csv ==================== */

string Analysis::escapeCSV(const std::string& s) const {
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


/* ==================== Export report to txt ==================== */

void Analysis::exportReport(const string exportFile) const {
    ofstream file (exportFile, ios::trunc);
    file << "==================== TRIE ANALYSIS REPORT ====================\n"
         << "\n\n-------------------- Original trie statistics --------------------\n"
         << "Note: This information is used to compare statistics of the Trie with what the analysis module has collected.\n\n"
         << "- Total inserted words: " << trie.totalInsertedWords() << '\n'
         << "- Total unique words: " << trie.totalUniqueWords() << '\n'
         << "- Total nodes: " << trie.totalNodes() << '\n'
         << "\n\n------------------------ Analyzed statistics -----------------------\n\n"
         << "- Total inserted words (after analysis): " << totalInsertedWords << '\n'
         << "- Total unique words (after analysis): " << totalUniqueWords << "\n\n"
         << "- Total nodes (after analysis): " << totalNodes << '\n'
         << "- Total unique-word characters: " << totalUniqueWordChar << "\n\n"
         << "- Compressed rate (total nodes / total unique-word characters): " << (double)totalNodes/totalUniqueWordChar << '\n'
         << "\n\n-------------------------- Thresholds ----------------------------\n\n"
         << "- Word frequency threshold (" << freqPercentile << "% lower percentile): " << freqThreshold << '\n'
         << "- Length frequency threshold (" << lenPercentile << "% lower percentile): " << lenFreqThreshold << '\n'
         << "- Entropy threshold (" << entropyPercentile << "% lower percentile): " << entropyThreshold << '\n'
         << "\n\n----------------------- Extremum statistics ------------------------\n\n"
         << "Word frequency:\n"
         << "- Max frequency: " << maxFreq << '\n'
         << "- Min frequency: " << minFreq << "\n\n"
         << "Word length (depth):\n"
         << "- Max length: " << maxDepth << '\n'
         << "- Min length: " << minDepth << '\n'
         << "- Most popular length: " << mostPopLength << '\n'
         << "- Least popular length: " << leastPopLength << "\n\n"
         << "Entropy (local node entropy):\n"
         << "- Max entropy: " << maxEntropy << '\n'
         << "- Min entropy: " << minEntropy << '\n'
         << "\n\n\n-------------------- ANOMALIES DETECTION --------------------"
         << "\n\n-------------------- Anomalies: frequency-based --------------------\n\n";
         
         // To be continue...
}