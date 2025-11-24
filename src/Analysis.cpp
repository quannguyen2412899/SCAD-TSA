#include "../include/Analysis.h"
using namespace std;
using json = nlohmann::json;


/* ==================== Constructor ==================== */

Analysis::Analysis(double freqPercentile, double entropyPercentile, double lenPercentile) : 
    // trie(trie),
    freqPercentile(freqPercentile), entropyPercentile(entropyPercentile), lenPercentile(lenPercentile),
    freqThreshold(0), entropyThreshold(0), lenFreqThreshold(0),
    totalInsertedWords(0), totalUniqueWords(0), totalNodes(0), totalUniqueWordChar(0),
    maxFreq(0), minFreq(0),
    maxDepth(0), minDepth(0),
    maxEntropy(0), minEntropy(0),
    // mostPopLength(0), leastPopLength(0),
    freqAnomaliesRate(0), lenAnomaliesRate(0), entropyAnomaliesRate(0) {}

    
/* ==================== Helper: Compute entropy ==================== */

double Analysis::computeLocalEntropy(const Node* node) {
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

void Analysis::collectStatistics(const StatTrie* _trie) {

    trie = _trie;
    totalInsertedWords = trie->totalInsertedWords();
    totalUniqueWords = trie->totalUniqueWords();
    totalNodes = trie->totalNodes();

    allEntries.clear();
    
    auto callback = [&](const Node* node, const std::string &word){
        double localEntropy = computeLocalEntropy(node);
        if (localEntropy > 0) {
            AnomalyEntry entry;
            entry.isWord = false;
            entry.word = word;
            entry.count = node->count;
            entry.freqRate = (double)entry.count / totalInsertedWords;
            entry.depth = word.size();
            entry.entropy = localEntropy;

            allEntries.push_back(entry);
        }
        if (node->isEnd) {
            AnomalyEntry entry;
            entry.isWord = true;
            entry.word = word;
            entry.count = node->countEnd();
            entry.freqRate = (double)entry.count / totalInsertedWords;
            entry.depth = word.size();
            entry.entropy = localEntropy;

            allEntries.push_back(entry);

            if (lenFreq.count(entry.depth)) lenFreq[entry.depth] += 1;
            else lenFreq[entry.depth] = 1;

            totalUniqueWordChar += entry.depth;
        }
    };
    trie->traverse(callback);

    sort(allEntries.begin(), allEntries.end(), [](AnomalyEntry& a, AnomalyEntry& b) {
        if (a.word.compare(b.word) == 0) return !a.isWord && b.isWord;
        return a.word.compare(b.word) < 0;
    });

    getExtremum();
    computePercentileThresholds();
    detectAnomalies();
}

void Analysis::getExtremum() {

    for (AnomalyEntry &e : allEntries) {
        if (e.isWord) {
            if (e.count > maxFreq) maxFreq = e.count;
            if (e.depth > maxDepth) maxDepth = e.depth;
        }
        if (e.entropy > maxEntropy) maxEntropy = e.entropy;
    }

    minFreq = maxFreq;
    minDepth = maxDepth;
    minEntropy = maxEntropy;
    for (AnomalyEntry &e : allEntries) {    
        if (e.isWord) {
            if (e.count < minFreq) minFreq = e.count;
            if (e.depth < minDepth) minDepth = e.depth;
        }
        if (e.entropy < minEntropy) minEntropy = e.entropy;
    }
}


void Analysis::computePercentileThresholds() {

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

    // mostPopLength = lenFreqs.front().first;
    // leastPopLength = lenFreqs.back().first;
    
    size_t fIdx = (size_t)((freqPercentile/100.0) * freqs.size());
    size_t eIdx = (size_t)((entropyPercentile/100.0) * entropies.size());
    size_t lIdx = 0;
    for (size_t count = lenFreqs[0].second; count < (lenPercentile/100.0) * totalInsertedWords; count += lenFreqs[++lIdx].second);

    if (fIdx >= freqs.size()) fIdx = freqs.size() - 1;
    if (eIdx >= entropies.size()) eIdx = entropies.size() - 1;
    if (lIdx >= lenFreqs.size()) lIdx = lenFreqs.size() - 1;

    freqThreshold = freqs[fIdx];
    entropyThreshold = entropies[eIdx];
    lenFreqThreshold = lenFreqs[lIdx].second;
}


void Analysis::markAnomalyNodes(unordered_set<const Node*> &anomalyNodes, const char mode) const {
    
    const vector<AnomalyEntry>* anomalies = 0;
    if (mode == 'a') {
        markAnomalyNodes(anomalyNodes, 'f');
        markAnomalyNodes(anomalyNodes, 'l');
        markAnomalyNodes(anomalyNodes, 'e');
        return;
    }
    else if (mode == 'f') anomalies = &freqAnomalies;
    else if (mode == 'l') anomalies = &lenAnomalies;
    else if (mode == 'e') anomalies = &entropyAnomalies;
    else {
        cout << "[ERROR] Unsupported mode: " << mode << endl;
        return;
    }
    
    string path;
    auto callback = [&] (const Node* node, const string &prefix) {
        if (prefix == path && node) anomalyNodes.insert(node);
    };

    for (const AnomalyEntry& e : *anomalies) {
        path = e.word;
        trie->traverse(path, callback);
    }

}



/* ==================== Export report, csv and json ====================*/

// void Analysis::report(const string directory) const {

//     cout << "All outputs are stored in " << directory << endl;

//     exportAnomaliesToCSV(directory);
//     exportCSV(directory+"/all_entries.csv");
//     exportReport(directory+"/overall_report.txt");

// }


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
                freqAnomaliesRate += entry.freqRate;
            }
            if (lenFreq[entry.depth] <= lenFreqThreshold) {
                lenAnomalies.push_back(entry);
                lenAnomalies.back().score = lenFreq[entry.depth];
                lenAnomaliesRate += entry.freqRate;
            }
        }
        else if (entry.entropy >= entropyThreshold) {
            entropyAnomalies.push_back(entry);
            entropyAnomalies.back().score = -entry.entropy;
            entropyAnomaliesRate += entry.freqRate;
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

// void Analysis::printAnomalies(const vector<AnomalyEntry> &anomalies) const {
//     // std::cout << "Word\tCount\tFreqRate\tEntropy\tDepth\tScore\n";
//     std::cout << "String\tCount\tFreqRate\tEntropy\tDepth\n";
//     std::cout << "---------------------------------------------------------------\n";
//     for (auto &a : anomalies) {
//         std::cout << a.word << "\t"
//                     << a.count << "\t"
//                     << std::fixed << std::setprecision(5) << a.freqRate << "\t"
//                     << a.entropy << "\t"
//                     << a.depth << "\n";
//                     // << a.score << "\n";
//     }
// }


/* ==================== Export to json ==================== */

// void Analysis::exportJSON(const string exportFile) const {
//     exportJSON(*trie, exportFile);
// }


// void Analysis::exportJSON (const StatTrie &_trie, const string exportFile) const {

//     ofstream file (exportFile, ios::trunc);
//     if (!file.is_open()) {
//         cout << "Failed to open " << exportFile << "to export json.";
//         return;
//     }
//     json jData, jRoot, jLabels, jIsAnomaly;
//     count_t id = 0;

//     auto collector = [&] (const StatTrie::Node* node, const string &prefix) {

//         if (prefix.empty()) jLabels[0] = "root";
//         else jLabels[id] = string(1, prefix.back());
//         json* j = &jRoot;
//         for (char c : prefix) j = &((*j)["children"][string(1, c)]);

//         (*j)["children"] = json::object();
//         for (const pair<const char, StatTrie::Node*> p : node->children) {
//             (*j)["children"][string(1, p.first)] = json::object();
//         }
//         (*j)["count"] = node->count;
//         // (*j)["isEnd"] = node->isEnd;
//         (*j)["ID"] = id;
//         jIsAnomaly[id] = (bool)anomalyNodes.count(node);
//         id++;
//     };

//     _trie.traverse(collector);
//     jData["root"] = jRoot;
//     jData["labels"] = jLabels;
//     jData["totalUnique"] = _trie.totalUniqueWords();
//     jData["isAnomaly"] = jIsAnomaly;
    
//     file << jData.dump(2);
//     file.close();
// }


// void Analysis::exportAnomaliesToJSON(const string directory) const {
//     StatTrie rareTrie;
//     for (const AnomalyEntry &entry : freqAnomalies) rareTrie.insert(entry.word, entry.count);
//     exportJSON(rareTrie, directory+"/rare_frequency.json");

//     rareTrie.clear();
//     for (const AnomalyEntry &entry : lenAnomalies) rareTrie.insert(entry.word, entry.count);
//     exportJSON(rareTrie, directory+"/rare_length.json");
    
//     rareTrie.clear();
//     for (const AnomalyEntry &entry : entropyAnomalies) rareTrie.insert(entry.word, entry.count);
//     exportJSON(rareTrie, directory+"/rare_entropy.json");   
// }


/* ==================== Export to csv ==================== */

// Helper: write csv content to file stream
void Analysis::writeCSVToFilestream (ofstream& file, const vector<AnomalyEntry>& anomalies)  const{
    file << "String,Kind,Frequency,Length,Length frequency,Entropy,Rate,Anomaly\n";
    for (const AnomalyEntry& entry : anomalies) {
        string status;
        if (entry.count <= freqThreshold) status += "frequency/";
        if (entry.isWord && lenFreq.at(entry.depth) <= lenFreqThreshold) status += "length/";
        if (!entry.isWord && entry.entropy <= entropyThreshold) status += "entropy/";
        if (!status.empty()) status.pop_back();
        file << escapeCSV(entry.word) << ','
             << (entry.isWord ? "word" : "prefix") << ','
             << entry.count << ',' 
             << entry.depth << ','
             << (entry.isWord ? lenFreq.at(entry.depth) : 0) << ','
             << entry.entropy << ','
             << entry.freqRate << ','
             << status << '\n';
    }
}


void Analysis::exportCSV(const string exportFile, char mode) const {
    ofstream fout;
    const vector<AnomalyEntry> *entries;

    if (mode == 'a') entries = &allEntries;
    else if (mode == 'f') entries = &freqAnomalies;
    else if (mode == 'l') entries = &lenAnomalies;
    else if (mode == 'e') entries = &entropyAnomalies;
    else cout << "[ERROR] Unsupported mode" << endl;

    fout.open(exportFile, ios::trunc);
    if (!fout.is_open()) {
        cout << "[ERROR] Failed to export anomalies to " << exportFile << endl;
        return;
    }

    writeCSVToFilestream(fout, *entries);
    fout.close();
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
    if (!file.is_open()) {
        cout << "Failed to open report file " << exportFile << endl;
        return;
    }

    file << "==================== TRIE ANALYSIS REPORT ====================\n"

         << "\n------------------------ Trie statistics -----------------------\n\n"
         << "- Total inserted words: " << totalInsertedWords << '\n'
         << "- Total unique words: " << totalUniqueWords << '\n'
         << "- Total unique-word characters: " << totalUniqueWordChar << '\n'
         << "- Total nodes: " << totalNodes << '\n'
         << "- Compressed rate (total unique-word characters / total nodes): " << (double)totalUniqueWordChar/totalNodes << '\n'
         << "\n----------------------- Extremum statistics ------------------------\n\n"
         << "Word frequency:\n"
         << "- Max frequency: " << maxFreq << '\n'
         << "- Min frequency: " << minFreq << "\n\n"
         << "Word length (depth):\n"
         << "- Max length: " << maxDepth << '\n'
         << "- Min length: " << minDepth << "\n\n"
        //  << "- Most popular length: " << mostPopLength << '\n'
        //  << "- Least popular length: " << leastPopLength << "\n\n"
         << "Entropy (local node entropy):\n"
         << "- Max entropy: " << maxEntropy << '\n'
         << "- Min entropy: " << minEntropy << '\n'

         << "\n-------------------- ANOMALIES DETECTION --------------------"
         << "\n\n-------------------------- Thresholds ----------------------------\n\n"
         << "- Word frequency threshold (" << freqPercentile << "% lower percentile): " << freqThreshold << '\n'
         << "- Length frequency threshold (" << lenPercentile << "% lower percentile): " << lenFreqThreshold << '\n'
         << "- Entropy threshold (" << entropyPercentile << "% lower percentile): " << entropyThreshold
         
         << "\n\n-------------------- Anomalies: frequency-based --------------------\n\n";

    size_t n = freqAnomalies.size() > 8 ? 8 : freqAnomalies.size();
    for (size_t i = 0; i < n; ++i) 
        file << freqAnomalies[i].word << ", frequency = " << freqAnomalies[i].count << '\n';
    if (n < freqAnomalies.size()) file << "...\n";
    file << "\nThere are " << freqAnomalies.size() << " frequency-based anomalies\n"
         << "Accounted for " << freqAnomaliesRate*100 << "% of the processed text"
    
         << "\n\n------------------ Anomalies: length-frequency-based -------------------\n\n";

    n = lenAnomalies.size() > 8 ? 8 : lenAnomalies.size();
    for (size_t i = 0; i < n; ++i) 
        file << lenAnomalies[i].word << ", length = " << lenAnomalies[i].depth
             << ", length frequency = " << lenFreq.at(lenAnomalies[i].depth) << ", frequency = " << lenAnomalies[i].count << '\n';
    if (n < lenAnomalies.size()) file << "...\n";
    file << "\nThere are " << lenAnomalies.size() << " length-frequency-based anomalies\n"
         << "Accounted for " << lenAnomaliesRate*100 << "% of the processed text"

         << "\n\n------------------ Anomalies: entropy-based -------------------\n\n";

    n = entropyAnomalies.size() > 8 ? 8 : entropyAnomalies.size();
    for (size_t i = 0; i < n; ++i) 
        file << entropyAnomalies[i].word << ", entropy = " << entropyAnomalies[i].entropy << ", frequency = " << entropyAnomalies[i].count << '\n';
    if (n < entropyAnomalies.size()) file << "...\n";
    file << "\nThere are " << entropyAnomalies.size() << " entropy-based anomalies\n"
         << "Accounted for " << entropyAnomaliesRate*100 << "% of the processed text"

         << "\n\n======================= END OF REPORT =============================";

    file.close();
}