#include "StatTrie.h"
using namespace std;
using json = nlohmann::json;


/* ---------- Node ---------- */

Node::Node() : count(0), isEnd(false) {}

unsigned Node::countEnd() const {
    unsigned result = count;
    for (const pair<const char, Node*> &p : children) result -= p.second->count;
    return result;
}


/* ---------- CONSTRUCTORS AND DESTRUCTORS ---------- */

StatTrie::StatTrie() :
    root(new Node),
    countNodes(1),
    countUniqueWordChar(0),
    countUniqueWords(0),
    countInsertedWords(0) {}

StatTrie::~StatTrie() {
    clear();
}


/* ---------- HELPERS ---------- */

void StatTrie::_clear (Node* node) {
    if (!node) return;
    for (pair<const char, Node*> p : node->children) _clear(p.second);
    delete node;
}

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
    if (word.size() == 0) return;
    Node* ptr = root;
    for (char c : word) {
        if (!ptr->children.count(c)) {
            ptr->children[c] = new Node;
            ++countNodes;
        }
        ptr = ptr->children[c];
        ++(ptr->count);
    }

    // countInsertedChar += word.size();
    ++countInsertedWords;
    if (!ptr->isEnd) {
        ptr->isEnd = true;
        ++countUniqueWords;
        countUniqueWordChar += word.size();
    }
}

void StatTrie::insert (string word, unsigned num) {
    if (word.size() == 0) return;
    Node* ptr = root;
    for (char c : word) {
        if (!ptr->children.count(c)) {
            ptr->children[c] = new Node;
            ++countNodes;
        }
        ptr = ptr->children[c];
        ptr->count += num;
    }

    // countInsertedChar += word.size() * num;
    countInsertedWords += num;
    if (!ptr->isEnd) {
        ptr->isEnd = true;
        ++countUniqueWords;
        countUniqueWordChar += word.size();
    }
}

bool StatTrie::contains (string word) const {
    const Node* ptr = root;
    for (char c : word) {
        unordered_map<char, Node*>::const_iterator it = ptr->children.find (c);
        if (it != ptr->children.end()) ptr = (*it).second;
        else return false;
    }
    if (ptr->isEnd) return true;
    return false;
}

bool StatTrie::startWith (string prefix) const {
    const Node* ptr = root;
    for (char c : prefix) {
        unordered_map<char, Node*>::const_iterator it = ptr->children.find (c);
        if (it != ptr->children.end()) ptr = (*it).second;
        else return false;
    }
    return true;
}

void StatTrie::remove (string word) {

    const size_t n = word.size();
    Node* ptr = root;
    Node* stack[n+1];
    stack[0] = root;
    for (size_t i = 0; i < n; ++i) {
        if (ptr->children.count(word[i])) {
            ptr = ptr->children[word[i]];
            stack[i+1] = ptr;
        }
        else return;
    }

    if (ptr->isEnd) {
        ptr->isEnd = false;
        unsigned reduction = ptr->countEnd();
        // for (pair<const char, Node*>& p : ptr->children) reduction -= p.second->count;
        for (int i = n-1; i >= 0; --i) {
            stack[i+1]->count -= reduction;
            if (stack[i+1]->count == 0) {
                delete stack[i+1];
                --countNodes;
                stack[i]->children.erase(word[i]);
            }
        }
        --countUniqueWords;
        countUniqueWordChar -= word.size();
        countInsertedWords -= reduction;
        // countInsertedChar -= n * reduction;
    }
}

void StatTrie::clear() {
    _clear(root);
    root = new Node;
    countInsertedWords = countUniqueWords = countUniqueWordChar = 0;
    countNodes = 1;
}


/* ---------- STATISTICAL METHODS ---------- */

unsigned StatTrie::totalNodes() const {
    return countNodes;
}

unsigned StatTrie::totalUniqueWordCharacters() const {
    return countUniqueWordChar;
}
// unsigned StatTrie::totalInsertedCharacters() const {
//     return countInsertedChar;
// }

unsigned StatTrie::totalInsertedWords() const {
    return countInsertedWords;
}

unsigned StatTrie::totalUniqueWords() const {
    return countUniqueWords;
}

void StatTrie::traverse (function<void(const Node*, const string&)> callback) const {
    string prefix;
    _traverse (callback, root, prefix);
}

void StatTrie::traverse (const string prefix, function<void(const Node*, const string&)> callback) const {
    const Node* ptr = root;
    callback(ptr, "");
    string _prefix;
    for (char c : prefix) {
        if (!ptr->children.count(c) || !ptr) return;
        _prefix.push_back(c);
        ptr = ptr->children.at(c);
        callback (ptr, _prefix);
    }
}

json StatTrie::toPartialJSON(const Node* root, const unordered_set<const Node*> &trimNodes, bool &containTrimNode, unsigned &id) const {
    
    if (!root) return json::object(); // guard
    json j;
    json jChildren = json::object();

    j["id"] = id++;
    bool subContain = false;

    for (auto& [ch, child] : root->children) {
        bool childContain = false;
        json cj = toPartialJSON(child, trimNodes, childContain, id);

        if (childContain) {
            cj["label"] = string(1, ch);
            subContain = true;
        }
        else {
            if (cj.contains("id")) id = cj["id"].get<unsigned>() + 1;
            cj["label"] = "...";
            cj["children"] = json::object();
        }

        jChildren[string(1, ch)] = move(cj);
    }

    bool isTrim = trimNodes.count(root);
    containTrimNode = isTrim || subContain;

    j["isEnd"] = root->isEnd;
    j["count"] = root->count;
    j["color"] = isTrim ? "red" : "black";
    j["children"] = move(jChildren);

    return j;
}

void StatTrie::exportPartialJSON(const string exportFile, const unordered_set<const Node*> &trimNodes) const {
    ofstream file (exportFile, ios::trunc);
    if (!file.is_open()) {
        cout << "[ERROR] Cannot open " << exportFile << " to export JSON" << endl;
        return;
    }
    unsigned id = 0;
    bool containTrimNode = false;
    json j;
    j["root"] = toPartialJSON(root, trimNodes, containTrimNode, id);
    j["root"]["label"] = "root";
    file << j.dump(2);
    file.close();

    cout << "Exported to: " << exportFile << endl;
}


json StatTrie::toJSON(const Node* root, const unordered_set<const Node*> &anomalyNodes, unsigned &id) const {
        
    if (!root) return json::object(); // guard
    json j;
    json jChildren = json::object();

    j["id"] = id++;

    for (auto& [ch, child] : root->children) {
        json cj = toJSON(child, anomalyNodes, id);
        cj["label"] = string(1, ch);
        jChildren[string(1, ch)] = move(cj);
    }

    bool isAnomaly = anomalyNodes.count(root);

    j["isEnd"] = root->isEnd;
    j["count"] = root->count;
    j["color"] = isAnomaly ? "red" : "black";
    j["children"] = move(jChildren);

    return j;
}

void StatTrie::exportAllJSON(const string exportFile, const unordered_set<const Node*> &anomalyNodes) const {
    ofstream file (exportFile, ios::trunc);
    if (!file.is_open()) {
        cout << "[ERROR] Cannot open " << exportFile << " to export JSON" << endl;
        return;
    }
    unsigned id = 0;
    json j;
    j["root"] = toJSON(root, anomalyNodes, id);
    j["root"]["label"] = "root";
    file << j;
    file.close();

    cout << "JSON exported to: " << exportFile << endl;
}