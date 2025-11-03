#include "Visualize.h"
#include <iostream>

void Visualizer::exportJSON (const StatTrie &trie) {

    const string exportPath = "data/trie.json";
    ofstream file (exportPath, ios::trunc);
    json jData;

    auto collector = [&jData] (const Node* node, const string &prefix) {
        json* j = &jData;
        for (char c : prefix) j = &((*j)["children"][string(1, c)]);

        (*j)["children"] = json::object();
        for (const pair<const char, Node*> p : node->children) {
            (*j)["children"][string(1, p.first)] = json::object();
        }
        (*j)["count"] = node->count;
        (*j)["isEnd"] = node->isEnd;
    };

    trie.traverse(collector);
    file << jData.dump(4);
}

// int main() {

//     StatTrie trie;
    
//     ifstream file ("data/text01");
//     cout << file.is_open();
//     string word;
//     while (getline(file, word, ' ')) {
//         cout << word << ' ';
//         trie.insert (word);
//     }

//     cout << trie.totalUniqueWords();
//     Visualizer::exportJSON(trie);

//     return 0;
// }