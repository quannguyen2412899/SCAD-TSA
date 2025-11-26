#include "Analysis.h"
#include <filesystem>
using namespace std;

void printHelp();

int main(int argc, char *argv[]) {

    cout << "========== Analyze and report ==========" << endl;

    if (argc == 2 && string(argv[1]) == "--help") {
        printHelp();
        return 0;
    }

    if (argc < 3) {
        cerr << "[ERROR] Expect: analyze <input_file> <output_dir> [flags]\nRun 'analyze --help' for usage info" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputDir = argv[2];

    /* Check paths' existence */
    if (!filesystem::exists(inputFile)) {
        cerr << "[ERROR] Input file '" <<  inputFile << "' does not exist" << endl;
        return 1;
    }
    if (!filesystem::exists(outputDir)) {
        cerr << "[ERROR] Output directory '" << outputDir << "' does not exist" << endl;
        return 1;
    }

    ifstream fin (inputFile);
    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open input file at '" << inputFile << "'\n";
        return 1;
    }

    /* --- Parse JSON flags --- */
    unordered_map<string,string> jsonTargets;
    for (int i = 3; i < argc; i++) {
        string arg = argv[i];
        auto pos = arg.find('=');
        if (pos == string::npos) continue;

        string flag = arg.substr(0, pos);
        string path = arg.substr(pos + 1);

        if (flag == "--json-complete" ||
            flag == "--json-partial"  ||
            flag == "--json-freq"     ||
            flag == "--json-len"      ||
            flag == "--json-entropy") 
        {
            if (jsonTargets.count(flag)) {
                cerr << "[ERROR] Duplicate flag: " << flag << "\nRun 'analyze --help' for usage info" << endl;
                return 1;
            }
            if (path.empty()) {
                cerr << "[ERROR] Missing output path for flag: " << flag << "\nRun 'analyze --help' for usage info\n";
                return 1;
            }
            jsonTargets[flag] = path;
        }
        else {
            cerr << "[ERROR] Invalid flag: " << flag << "\nRun 'analyze --help' for usage info\n";
            return 1;
        }
    }

    /* Build trie */
    StatTrie trie;
    string line;
    while (getline(fin, line)) trie.insert(line);

    /* Analyze trie */
    Analysis a;
    a.collectStatistics(&trie);


    /* Output */

    //  brief report
    a.exportReport(outputDir+"/overall_report.txt");

    //  csv
    a.exportCSV(outputDir+"/all_entries.csv");
    a.exportCSV(outputDir+"/frequency_anomalies.csv", 'f');
    a.exportCSV(outputDir+"/length_anomalies.csv", 'l');
    a.exportCSV(outputDir+"/entropy_anomalies.csv", 'e');
    
    if (jsonTargets.count("--json-complete")) {
        unordered_set<const Node*> allAbnormalNodes;
        a.markAnomalyNodes(allAbnormalNodes);
        trie.exportAllJSON(jsonTargets["--json-complete"], allAbnormalNodes);
    }
    if (jsonTargets.count("--json-partial")) {
        unordered_set<const Node*> allAbnormalNodes;
        a.markAnomalyNodes(allAbnormalNodes);
        trie.exportPartialJSON(jsonTargets["--json-partial"], allAbnormalNodes);
    }
    if (jsonTargets.count("--json-freq")) {
        unordered_set<const Node*> freqAbnormalNodes;
        a.markAnomalyNodes(freqAbnormalNodes, 'f');
        trie.exportPartialJSON(jsonTargets["--json-freq"], freqAbnormalNodes);
    }
    if (jsonTargets.count("--json-len")) {
        unordered_set<const Node*> lengthAbnormalNodes;
        a.markAnomalyNodes(lengthAbnormalNodes, 'l');
        trie.exportPartialJSON(jsonTargets["--json-len"], lengthAbnormalNodes);
    }
    if (jsonTargets.count("--json-entropy")) {
        unordered_set<const Node*> entropyAbnormalNodes;
        a.markAnomalyNodes(entropyAbnormalNodes, 'e');
        trie.exportPartialJSON(jsonTargets["--json-entropy"], entropyAbnormalNodes);
    }

    return 0;
}

void printHelp() {
    cout << "Usage: analyze <input_file> <output_dir> [flags]\n\n"
         << "JSON export flags (default: no JSON exported):\n"
         << "  --json-complete=<path>   Export full trie JSON\n"
         << "  --json-partial=<path>    Export trimmed trie JSON (only anomalies)\n"
         << "  --json-freq=<path>       Export JSON showing frequency anomalies\n"
         << "  --json-len=<path>        Export JSON showing length anomalies\n"
         << "  --json-entropy=<path>    Export JSON showing entropy anomalies\n"
         << "\nOther flags:\n"
         << "  --help                   Show this help message\n";
}