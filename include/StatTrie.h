#ifndef _STATTRIE_
#define _STATTRIE_


#include <unordered_map>
#include <string>
typedef unsigned int count_t;

using namespace std;

class StatTrie {

    private:

    struct Node {
        unordered_map<char, Node*> children;
        count_t count;
        bool isEnd;

        Node() : count(0), isEnd(false) {}
        ~Node() {
            for (pair<const char, Node*> p : children) delete p.second;
        }
    };

    Node root;
    double anomalyRate;  // The occurence rate of one word below this rate => anomaly
    count_t countNodes; // Total number of Nodes currently in Trie
    count_t countUniqueWords;   // Total number of unique words currently stored in Trie
    count_t countInsertedChar;  // Total number of characters inserted to Trie (increased by inserting word's size for each insertion)
    count_t countInsertedWords; // Total number of words inserted to Trie (including duplications)

    public:

    StatTrie(double anomalyRate = 0.001);
    // ~StatTrie();
    
    void insert (string word);
    bool contains (string word) const;
    bool startWith (string prefix) const;
    void remove (string word);
    void clear();

    count_t totalNodes() const;
    count_t totalInsertedCharacters() const;
    count_t totalInsertedWords() const;
    count_t totalUniqueWords() const;

    void setAnomalyRate (double rate);
};



#endif