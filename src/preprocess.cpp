#include <iostream>
#include <string>
#include <cstring>
#include "Preprocessor.h"

using namespace std;

void displayUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  ./preprocess <input file> <output file> [flags]\n\n";
    std::cerr << "Flags:\n";
    std::cerr << "  --regex=<pattern>       : filter using regex \n";
    std::cerr << "  --ignore=<chars>        : characters to ignore\n";
    std::cerr << "  --delim=<chars>         : delimiter characters\n";
    std::cerr << "  --help                  : display this help message\n";
}

int main(int argc, char** argv) {
    cout << "========== Preprocess ==========" << endl;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            displayUsage();
            return 0; 
        }
    }

    if (argc < 3) {
        displayUsage();
        return 1;
    }

    std::string inputFile  = argv[1];
    std::string outputFile = argv[2];

    std::string regexPattern = "";
    std::string ignoreChars  = "";
    std::string delimChars   = "";


    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];

        if (arg.rfind("--regex=", 0) == 0) {
            regexPattern = arg.substr(8);
        }
        else if (arg.rfind("--ignore=", 0) == 0) {
            ignoreChars = arg.substr(9);
        }
        else if (arg.rfind("--delim=", 0) == 0) {
            delimChars = arg.substr(8);
        }
    }


    Preprocessor pp(true);

    if (!regexPattern.empty()) {
        vector<string> cleaned = pp.filterByRegex(inputFile,  regexPattern);
        pp.exportCollected(outputFile, cleaned);
        cout << "Cleaned data exported to " << outputFile << endl;
        return 0;
    }


    if (!ignoreChars.empty()) {
        pp.setIgnoredCharacters(ignoreChars);
    }

    if (!delimChars.empty()) {
        pp.setDelimiters(delimChars);
    }

    pp.processFile(inputFile, outputFile);
    cout << "Cleaned data exported to: " << outputFile << endl;
    return 0;
}