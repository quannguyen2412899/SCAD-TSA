#ifndef _STATTRIE_
#define _STATTRIE_


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include "nlohmann/json.hpp"



struct Node {
    std::unordered_map<char, Node*> children;
    unsigned count;
    bool isEnd;

    Node();
    // ~Node();
    unsigned countEnd() const;
};

class StatTrie {
    
    private:

    Node* root;
    unsigned countNodes; // Total number of Nodes currently in Trie
    unsigned countUniqueWordChar;
    unsigned countUniqueWords;   // Total number of unique words currently stored in Trie
    unsigned countInsertedWords; // Total number of words inserted to Trie (including duplications)

    void _clear (Node* node);
    void _traverse (std::function<void(const Node*, const std::string&)> &callback, const Node* currNode, std::string &prefix) const;
    
    nlohmann::json toPartialJSON(const Node* root, const std::unordered_set<const Node*> &trimNodes, bool &containTrimNode, unsigned &id) const;
    nlohmann::json toJSON(const Node* root, const std::unordered_set<const Node*> &anomalyNodes, unsigned &id) const;


    public:

    StatTrie();
    ~StatTrie();
    
    void insert (std::string word);
    void insert (std::string word, unsigned num);
    bool contains (std::string word) const;
    bool startWith (std::string prefix) const;
    void remove (std::string word);
    void clear();

    unsigned totalNodes() const;
    unsigned totalUniqueWordCharacters() const;
    unsigned totalInsertedWords() const;
    unsigned totalUniqueWords() const;

    void traverse (std::function<void(const Node*, const std::string&)> callback) const;
    void traverse (const std::string prefix, std::function<void(const Node*, const std::string&)> callback) const;

    void exportPartialJSON(const std::string exportFile, const std::unordered_set<const Node*> &trimNodes) const;
    void exportAllJSON(const std::string exportFile, const std::unordered_set<const Node*> &anomalyNodes) const;
};



#endif