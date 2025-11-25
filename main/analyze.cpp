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
    // else if (argc == 2) {
    //     cout << "[ERROR] Not enough arguments\nRun 'analyze --help' for usage info" << endl;
    //     return 1;
    // }

    if (argc < 3) {
        cout << "[ERROR] Expect: analyze <input_file> <output_dir> [flags]\nRun 'analyze --help' for usage info" << endl;
        return 0;
    }

    string inputFile = argv[1];
    string outputDir = argv[2];

    /* Check paths' existence */
    if (!filesystem::exists(inputFile)) {
        cout << "[ERROR] Input file '" <<  inputFile << "' does not exist" << endl;
        return 1;
    }
    if (!filesystem::exists(outputDir)) {
        cout << "[ERROR] Output directory '" << outputDir << "' does not exist" << endl;
        return 1;
    }

    ifstream fin (inputFile);
    if (!fin.is_open()) {
        cout << "[ERROR] Cannot open input file at '" << inputFile << "'\n";
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
                cout << "[ERROR] Duplicate flag: " << flag << "\nRun 'analyze --help' for usage info" << endl;
                return 1;
            }
            if (path.empty()) {
                cout << "[ERROR] Missing output path for flag: " << flag << "\nRun 'analyze --help' for usage info\n";
                return 1;
            }
            jsonTargets[flag] = path;
        }
        else {
            cout << "[ERROR] Invalid flag: " << flag << "\nRun 'analyze --help' for usage info\n";
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


    // if (!jsonTargets.empty()) cout << "\nJSON files:\n";
    // json
    unordered_set<const Node*> allAbnormalNodes;
    unordered_set<const Node*> freqAbnormalNodes;
    unordered_set<const Node*> lengthAbnormalNodes;
    unordered_set<const Node*> entropyAbnormalNodes;
    if (jsonTargets.count("--json-complete")) {
        allAbnormalNodes.clear();
        a.markAnomalyNodes(allAbnormalNodes);
        trie.exportAllJSON(jsonTargets["--json-complete"], allAbnormalNodes);
    }
    if (jsonTargets.count("--json-partial")) {
        allAbnormalNodes.clear();
        a.markAnomalyNodes(allAbnormalNodes);
        trie.exportPartialJSON(jsonTargets["--json-partial"], allAbnormalNodes);
    }
    if (jsonTargets.count("--json-freq")) {
        freqAbnormalNodes.clear();
        a.markAnomalyNodes(freqAbnormalNodes, 'f');
        trie.exportPartialJSON(jsonTargets["--json-freq"], freqAbnormalNodes);
    }
    if (jsonTargets.count("--json-len")) {
        lengthAbnormalNodes.clear();
        a.markAnomalyNodes(lengthAbnormalNodes, 'l');
        trie.exportPartialJSON(jsonTargets["--json-len"], lengthAbnormalNodes);
    }
    if (jsonTargets.count("--json-entropy")) {
        entropyAbnormalNodes.clear();
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