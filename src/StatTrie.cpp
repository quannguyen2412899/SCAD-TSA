#include "StatTrie.h"
using namespace std;


/* ---------- CONSTRUCTORS AND DESTRUCTORS ---------- */

StatTrie::StatTrie(double anomalyRate) {
    this->anomalyRate = anomalyRate;
    countInsertedWords = countUniqueWords = countInsertedChar = 0;
    countNodes = 1;
}


/* ---------- HELPERS ---------- */

void StatTrie::_traverse (function<void(const Node*, const string&)> &callback, const Node* currNode, string &prefix) const {
    callback (currNode, prefix);
    for (const pair<const char, Node*> &p : currNode->children) {
        prefix.push_back (p.first);
        _traverse (callback, p.second, prefix);
        prefix.pop_back();
    }
}


/* ---------- BASIC METHODS ---------- */

void StatTrie::insert (string word) {
    /*
    Propose:
    Use root.count to count number of words inserted instead of countInsertedWords
    Edge case: if (word.size() == 0) return;
    */
    if (word.size() == 0) return;
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

void StatTrie::remove (string word) {

    const size_t n = word.size();
    Node* ptr = &root;
    Node* stack[n+1];
    stack[0] = &root;
    for (size_t i = 0; i < n; ++i) {
        if (ptr->children.count(word[i])) {
            ptr = ptr->children[word[i]];
            stack[i+1] = ptr;
        }
        else return;
    }

    if (ptr->isEnd) {
        ptr->isEnd = false;
        count_t reduction = ptr->count;
        for (pair<const char, Node*>& p : ptr->children) reduction -= p.second->count;
        for (int i = n-1; i >= 0; --i) {
            stack[i+1]->count -= reduction;
            if (stack[i+1]->count == 0) {
                delete stack[i+1];
                --countNodes;
                stack[i]->children.erase(word[i]);
            }
        }
        --countUniqueWords;
        countInsertedWords -= reduction;
        countInsertedChar -= n * reduction;
    }
}

void StatTrie::clear() {
    for (pair<const char, Node*> p : root.children) delete p.second;
    root.children.clear();
    countInsertedWords = countUniqueWords = countInsertedChar = 0;
    countNodes = 1;
}


/* ---------- STATISTICAL METHODS ---------- */

count_t StatTrie::totalNodes() const {
    return countNodes;
}

count_t StatTrie::totalInsertedCharacters() const {
    return countInsertedChar;
}

count_t StatTrie::totalInsertedWords() const {
    return countInsertedWords;
}

count_t StatTrie::totalUniqueWords() const {
    return countUniqueWords;
}

void StatTrie::setAnomalyRate (double rate) {
    if (rate <= 1 && rate > 0) anomalyRate = rate;
}

double StatTrie::getAnomalyRate() const {
    return anomalyRate;
}

void StatTrie::traverse (function<void(const Node*, const string&)> callback) const {
    string prefix;
    _traverse (callback, &root, prefix);
}