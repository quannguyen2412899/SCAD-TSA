#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>
using namespace std;    

class Preprocessor {
private:
    bool toLower;
    unordered_set<char> ignoredChars;
    unordered_set<char> delimiters;

    string normalizeWhitespace(const string& s) const;

public:
    Preprocessor(bool toLower = true);

    void setIgnoredCharacters(const string& chars);
    void setDelimiters(const string& chars);

    string cleanLine(const string& line) const;
    
    vector<string> processFile(
        const string& inputFile,
        const string& outputFile = ""
    );
    

    vector<string> filterByRegex(
        const string& inputFile,
        const string& pattern
    );

    void exportCollected(const string& outputFile, vector<string> data); 

};

#endif
