#include <iostream>
#include <string>
#include <cstring>
#include "../include/Preprocessor.h"

using namespace std;

int main(int argc, char** argv) {
    cout << "========== Preprocess ==========" << endl;
    if (argc < 3) {
        std::cerr << "Usage:\n";
        std::cerr << "  preprocess <input file> <output file> [flags]\n\n";
        std::cerr << "Flags:\n";
        std::cerr << "  --regex=<pattern>       : filter using regex \n";
        std::cerr << "  --ignore=<chars>        : characters to ignore\n";
        std::cerr << "  --delim=<chars>         : delimiter characters\n";
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
        pp.filterByRegex(inputFile, outputFile, regexPattern);
        return 0;
    }



    if (!ignoreChars.empty()) {
        std::cout << "  + Ignore characters: " << ignoreChars << "\n";
        pp.setIgnoredCharacters(ignoreChars);
    }

    if (!delimChars.empty()) {
        std::cout << "  + Delimiter characters: " << delimChars << "\n";
        pp.setDelimiters(delimChars);
    }

    pp.processFile(inputFile, outputFile);
    return 0;
}
