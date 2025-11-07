#include "Visualize.h"
#include <iostream>


void Visualizer::exportJSON (const StatTrie &trie) {

    const string exportPath = "data/interim/trie.json";
    ofstream file (exportPath, ios::trunc);
    json jData, jRoot, jLabels;
    count_t id = 0;

    auto collector = [&jRoot, &jLabels, &id] (const Node* node, const string &prefix) {

        if (prefix.empty()) jLabels[0] = "root";
        else jLabels[id] = string(1, prefix.back());
        json* j = &jRoot;
        for (char c : prefix) j = &((*j)["children"][string(1, c)]);

        (*j)["children"] = json::object();
        for (const pair<const char, Node*> p : node->children) {
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
    
    file << jData.dump(4);
}

int main() {

    StatTrie trie(0.007);
    ifstream file ("data/input/text01.txt");
    string word;
    while (getline (file, word, ' ')) {
        trie.insert (word);
    }
    Visualizer::exportJSON(trie);


    return 0;
}
