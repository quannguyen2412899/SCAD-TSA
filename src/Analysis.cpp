#include "Analysis.h"
using namespace std;
using json = nlohmann::json;


/* ==================== Constructor ==================== */

Analysis::Analysis(double freqPercentile, double lenPercentile, double entropyPercentile) : 
    // trie(trie),
    freqPercentile(freqPercentile), entropyPercentile(entropyPercentile), lenPercentile(lenPercentile),
    freqThreshold(0), entropyThreshold(0), lenFreqThreshold(0),
    totalInsertedWords(0), totalUniqueWords(0), totalNodes(0), totalUniqueWordChar(0),
    maxFreq(0), minFreq(0),
    maxDepth(0), minDepth(0),
    maxEntropy(0), minEntropy(0),
    freqAnomaliesRate(0), lenAnomaliesRate(0), entropyAnomaliesRate(0) {}

    
/* ==================== Helper: Compute entropy ==================== */

double Analysis::computeLocalEntropy(const Node* node) {
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
    totalUniqueWordChar = trie->totalUniqueWordCharacters();

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

            if (lenFreq.count(entry.depth)) lenFreq[entry.depth] += entry.count;
            else lenFreq[entry.depth] = entry.count;

            // totalUniqueWordChar += entry.depth;
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

    size_t fIdx = (size_t)((freqPercentile/100.0) * freqs.size());
    size_t eIdx = (size_t)((entropyPercentile/100.0) * entropies.size());
    size_t lIdx = (size_t)((lenPercentile/100.0) * lenFreqs.size());

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
        cerr << "[ERROR] Unsupported mode: " << mode << endl;
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
        if (a.score == b.score) return a.word.compare(b.word) < 0;
        return a.score < b.score;
    };
    // sort anomalies theo score tăng dần (score càng nhỏ càng hiếm)
    sort(freqAnomalies.begin(), freqAnomalies.end(), compare);
    sort(lenAnomalies.begin(), lenAnomalies.end(), compare);
    sort(entropyAnomalies.begin(), entropyAnomalies.end(), compare);

    // return anomalies;
}


/* ==================== Export to csv ==================== */

// Helper: write csv content to file stream
void Analysis::writeCSVToFilestream (ofstream& file, const vector<AnomalyEntry>& anomalies)  const{
    file << "String,Kind,Frequency,Length,Length frequency,Entropy,Rate,Anomaly\n";
    for (const AnomalyEntry& entry : anomalies) {
        string status;
        if (entry.count <= freqThreshold) status += "frequency/";
        if (entry.isWord && lenFreq.at(entry.depth) <= lenFreqThreshold) status += "length/";
        if (!entry.isWord && entry.entropy >= entropyThreshold) status += "entropy/";
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
    fout.open(exportFile, ios::trunc);
    if (!fout.is_open()) {
        cerr << "[ERROR] Failed to export anomalies to " << exportFile << endl;
        return;
    }
    const vector<AnomalyEntry> *entries = nullptr;

    if (mode == 'a') {
        entries = &allEntries;
    }
    else if (mode == 'f') {
        entries = &freqAnomalies;
    }
    else if (mode == 'l') {
        entries = &lenAnomalies;
    }
    else if (mode == 'e') {
        entries = &entropyAnomalies;
    }
    else cerr << "[ERROR] Unsupported mode" << endl;

    writeCSVToFilestream(fout, *entries);
    fout.close();

    cout << "CSV is saved at: " << exportFile << endl;
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
        cerr << "[ERROR] Failed to open report file " << exportFile << endl;
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
         << "- Entropy threshold (" << entropyPercentile << "% upper percentile): " << entropyThreshold
         
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

    cout << "Report is saved at: " << exportFile << endl;
}