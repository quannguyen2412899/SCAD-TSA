#include <iostream>
#include "../include/Preprocessor.h"
#include "../include/StatTrie.h"
using namespace std;

int main() {
    string filepath = "data/data.log";
    
    cout << "=== Trie Anomaly Detection Test ===" << endl;

    // Bước 1: Tiền xử lý file log
    Preprocessor prep(filepath, true, false); // không chia chuỗi
    prep.setIgnoredCharacters(",;:[](){}<>\"'-= ");
    prep.setDelimiters(".!?"); // chỉ dùng cho text, nhưng log cũng OK
    prep.run(DataType::LOG);

    const vector<string>& data = prep.getProcessedData();
    cout << "Total processed entries: " << data.size() << endl;

    // Bước 2: Khởi tạo Trie thống kê
    StatTrie trie(0.001);

    // Bước 3: Chèn toàn bộ chuỗi vào Trie
    for (const string& s : data) {
        trie.insert(s);
    }

    // Bước 4: Xuất thống kê cơ bản
    cout << "\n=== Trie Statistics ===" << endl;
    cout << "Total nodes: " << trie.totalNodes() << endl;
    cout << "Total unique sequences: " << trie.totalUniqueWords() << endl;
    cout << "Total inserted sequences: " << trie.totalInsertedWords() << endl;
    cout << "Total inserted characters: " << trie.totalInsertedCharacters() << endl;
    cout << "Anomaly rate threshold: " << trie.getAnomalyRate() << endl;

    // Bước 5: Kiểm tra thử vài chuỗi
    cout << "\n=== Sample check ===" << endl;
    string testStr = "conchocon"; 
    cout << "Contains '" << testStr << "' ? "
         << (trie.contains(testStr) ? "YES" : "NO") << endl;

    cout <<"\nCheck startwith" << endl;
    string str = "conm";
    cout << "Startwith '" << str << "' ? "
         << (trie.startWith(str) ? "YES" : "NO") << endl;

    trie.remove("conchonbk");
    cout << "\nPrint Tree" << endl;
    //trie.printTrie();
    return 0;
}
