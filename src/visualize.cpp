#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

// compile: 

int main(int argc, char* argv[]) {

    cout << "========== Visualize trie ==========" << endl;

    if (argc < 3 || argc > 3) {
        cerr << "[ERROR] Expect: visualize <input_file> <output_file>" << endl;
        exit(EXIT_FAILURE);
    }

    string command = "python3 src/visualize.py ";
    for (int i = 1; i < argc; ++i) {
        command += argv[i];
        if (i < argc - 1) {
            command += " ";
        }
    }
    
    int status = system(command.c_str());
    if (status != 0) {
        cerr << "[ERROR] Failed to run script '" << command << '\'' << endl;
        return 1;
    }

    return 0;
}