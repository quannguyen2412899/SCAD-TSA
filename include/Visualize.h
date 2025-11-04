#ifndef _VISUALIZE_
#define _VISUALIZE_

#include <fstream>
#include "StatTrie.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using Node = StatTrie::Node;
using namespace std;

class Visualizer {

    public:
    static void exportJSON (const StatTrie &trie);

};

#endif