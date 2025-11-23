#include "Analysis.h"
#include <filesystem>
using namespace std;


int main(int argc, char *argv[]) {

    cout << "========== Analyze and report ==========" << endl;

    if (argc != 3) {
        cout << "[ERROR] Expect 2 argument (input and output path). Got " << argc-1 << " argument(s)" << endl;
        return 0;
    }

    string inputFile = argv[1];
    string outputDir = argv[2];

    /* Check paths' existence */
    if (!filesystem::exists(inputFile)) {
        cout << "[ERROR] Input file " <<  inputFile << " does not exist" << endl;
        return 0;
    }
    if (!filesystem::exists(outputDir)) {
        cout << "[ERROR] Output directory " << outputDir << " does not exist" << endl;
        return 0;
    }

    ifstream fin (inputFile);
    if (!fin.is_open()) {
        cout << "[ERROR] Cannot open input file at " << inputFile << endl;
        return 0;
    }

    /* Build trie */
    StatTrie trie;
    string line;
    while (getline(fin, line)) trie.insert(line);

    Analysis a;
    unordered_set<const Node*> allAbnormalNodes;
    unordered_set<const Node*> freqAbnormalNodes;
    unordered_set<const Node*> lengthAbnormalNodes;
    unordered_set<const Node*> entropyAbnormalNodes;

    /* Analyze trie */
    a.collectStatistics(&trie);

    /* Output */
    //  brief report
    a.exportReport(outputDir+"/report.txt");
    //  csv
    a.exportCSV(outputDir+"/all_entries.csv");
    a.exportCSV(outputDir+"/frequency_anomalies.csv", 'f');
    a.exportCSV(outputDir+"/length_anomalies.csv", 'l');
    a.exportCSV(outputDir+"/entropy_anomalies.csv", 'e');
    // json
    a.markAnomalyNodes(allAbnormalNodes);
    a.markAnomalyNodes(freqAbnormalNodes, 'f');
    a.markAnomalyNodes(lengthAbnormalNodes, 'l');
    a.markAnomalyNodes(entropyAbnormalNodes, 'e');
    trie.exportAllJSON(outputDir+"/complete_trie.json", allAbnormalNodes);
    trie.exportPartialJSON(outputDir+"/partial_trie.json", allAbnormalNodes);
    trie.exportPartialJSON(outputDir+"/frequency_anomalies_trie.json", freqAbnormalNodes);
    trie.exportPartialJSON(outputDir+"/length_anomalies_trie.json", lengthAbnormalNodes);
    trie.exportPartialJSON(outputDir+"/entropy_anomalies_trie.json", entropyAbnormalNodes);


    cout << "Outputs including a brief report, csv files and json files are stored in directory: " << outputDir << endl;

    return 0;
}