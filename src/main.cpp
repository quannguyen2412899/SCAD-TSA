#include <iostream>
#include "../include/Preprocessor.h"
#include "../include/StatTrie.h"
#include "../include/Analysis.h"
using namespace std;

int main() {
    string filepath = "data/Android_2k.log";
    
    cout << "=== Test ===" << endl;

    Preprocessor prep(filepath, false, false); 
    prep.setIgnoredCharacters(",;:[](){}<>\"'=-");
    prep.setDelimiters(".!?"); 
    prep.run(DataType::LOG);

    const vector<string>& data = prep.getProcessedData();
    cout << "Total processed entries: " << data.size() << endl;

    StatTrie trie(0.0006);

    for (const string& s : data) {
        trie.insert(s);
    }

    cout << "\n=== Trie Statistics ===" << endl;
    cout << "Total nodes: " << trie.totalNodes() << endl;
    cout << "Total unique sequences: " << trie.totalUniqueWords() << endl;
    cout << "Total inserted sequences: " << trie.totalInsertedWords() << endl;
    cout << "Total inserted characters: " << trie.totalInsertedCharacters() << endl;
    cout << "Anomaly rate: " << trie.getAnomalyRate() << endl;


    cout << "\n=== Sample check ===" << endl;
    string testStr = ""; 
    cout << "Contains '" << testStr << "' ? "
         << (trie.contains(testStr) ? "YES" : "NO") << endl;

    cout <<"\nCheck startwith" << endl;
    string str = "";
    cout << "Startwith '" << str << "' ? "
         << (trie.startWith(str) ? "YES" : "NO") << endl;


    cout << "\n=== Analyzing Trie... ===" << endl;
    Analysis analyzer(trie);
    analyzer.detectAnomalies();

    cout << "\n=== Analysis Summary ===" << endl;
    analyzer.generateReport("analysis_report.txt");

    return 0;
}
