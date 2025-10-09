#include "StatTrie.h"
using namespace std;


/* ---------- CONSTRUCTORS AND DESTRUCTORS ---------- */

StatTrie::StatTrie(double anomalyRate = 0.001) {
    this->anomalyRate = anomalyRate;
    countInsertedWords = countUniqueWords = countInsertedChar = 0;
    countNodes = 1;
}


/* ---------- BASIC METHODS ---------- */

void StatTrie::insert (string word) {
    Node* ptr = &root;
    for (char c : word) {
        if (!ptr->children.count(c)) {
            ptr->children[c] = new Node;
            ++countNodes;
        }
        ptr = ptr->children[c];
        ++(ptr->count);
    }

    countInsertedChar += word.size();
    ++countInsertedWords;
    if (!ptr->isEnd) {
        ptr->isEnd = true;
        ++countUniqueWords;
    }
}

bool StatTrie::contains (string word) const {
    const Node* ptr = &root;
    for (char c : word) {
        unordered_map<char, Node*>::const_iterator it = ptr->children.find (c);
        if (it != ptr->children.end()) ptr = (*it).second;
        else return false;
    }
    if (ptr->isEnd) return true;
    return false;
}

bool StatTrie::startWith (string prefix) const {
    const Node* ptr = &root;
    for (char c : prefix) {
        unordered_map<char, Node*>::const_iterator it = ptr->children.find (c);
        if (it != ptr->children.end()) ptr = (*it).second;
        else return false;
    }
    return true;
}

void StatTrie::clear() {
    for (pair<const char, Node*> p : root.children) delete p.second;
    countInsertedWords = countUniqueWords = countInsertedChar = 0;
    countNodes = 1;
}