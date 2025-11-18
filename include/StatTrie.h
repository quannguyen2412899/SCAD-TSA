#ifndef _STATTRIE_
#define _STATTRIE_


#include <unordered_map>
#include <string>
#include <functional>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

typedef unsigned int count_t;

using namespace std;

class Analysis;

class StatTrie {

    public:

    struct Node {
        unordered_map<char, Node*> children;
        count_t count;
        bool isEnd;

        Node() : count(0), isEnd(false) {}
        ~Node() {
            for (pair<const char, Node*> p : children) delete p.second;
        }
        count_t countEnd() const {
            count_t result = count;
            for (const pair<const char, Node*> &p : children) result -= p.second->count;
            return result;
        }
    };

    private:

    Node root;
    double anomalyRate;  // The occurence rate of one word below this rate => anomaly
    count_t countNodes; // Total number of Nodes currently in Trie
    count_t countUniqueWords;   // Total number of unique words currently stored in Trie
    count_t countInsertedChar;  // Total number of characters inserted to Trie (increased by inserting word's size for each insertion)
    count_t countInsertedWords; // Total number of words inserted to Trie (including duplications)

    void _traverse (function<void(const Node*, const string&)> &callback, const Node* currNode, string &prefix) const;
    nlohmann::json toJSON(const Node* root, const unordered_set<const Node*> &trimNodes, bool &containTrimNode, unsigned &id) const;

    friend class Analysis;

    public:

    StatTrie(double anomalyRate = 0.001);
    // ~StatTrie();
    
    void insert (string word);
    void insert (string word, count_t num);
    bool contains (string word) const;
    bool startWith (string prefix) const;
    void remove (string word);
    void clear();

    count_t totalNodes() const;
    count_t totalInsertedCharacters() const;
    count_t totalInsertedWords() const;
    count_t totalUniqueWords() const;

    void setAnomalyRate (double rate);
    double getAnomalyRate () const;
    
    void traverse (function<void(const Node*, const string&)> callback) const;
    void traverse (const string prefix, function<void(const Node*, const string&)> callback) const;

    void exportJSON(const string exportFile, const unordered_set<const Node*> &trimNodes) const;

};



#endif