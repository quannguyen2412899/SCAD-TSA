#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cstring> // For strncmp
#include <filesystem>

#ifdef _WIN32
const std::string BIN_PREPROCESS = "bin\\preprocess";
const std::string BIN_ANALYZE = "bin\\analyze";
const std::string BIN_VISUALIZE = "bin\\visualize";
#else
const std::string BIN_PREPROCESS = "bin/preprocess";
const std::string BIN_ANALYZE = "bin/analyze";
const std::string BIN_VISUALIZE = "bin/visualize";
#endif


// VisualTask struct removed since visualization step is deleted

// Function to run system commands and check for errors
void run_command(const std::string& step_name, const std::string& command) {
    // std::cout << "\n[PIPELINE] >>> Starting: " << step_name << "..." << std::endl;
    // std::cout << "  Command: " << command << std::endl; // Uncomment to debug command
    
    int status = std::system(command.c_str());
    
    if (status != 0) {
        std::cerr << "\n[ERROR] " << step_name << " failed (Exit code: " << status << ")." << std::endl;
        exit(EXIT_FAILURE);
    }
    // std::cout << "[PIPELINE] >>> Finished: " << step_name << ".\n" << std::endl;
}

// Helper function to check string prefix
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

void print_help(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " <input_text> <output_directory> [flags]\n\n"
              << "PREPROCESSING FLAGS:\n"
              << "  --regex=\"...\"       : Filter strings using Regex (Highest priority)\n"
              << "  --delim=\"...\"       : Delimiter characters (Default if no regex)\n"
              << "  --ignore=\"...\"      : Characters to ignore (Default if no regex)\n\n"
              << "ANALYZE FLAGS:\n"
              << "  --perc-freq=<val>      Percentile threshold for Frequency (Low, default: 5)\n"
              << "  --perc-len=<val>       Percentile threshold for Length (Low, default: 5)\n"
              << "  --perc-entropy=<val>   Percentile threshold for Entropy (High, default: 95)\n\n"
              << "JSON EXPORT FLAGS:\n"
              << "  --json-complete     : Export complete Trie structure JSON\n"
              << "  --json-partial      : Export partial Trie structure JSON (show anomalies only)\n"
              << "  --json-freq         : Export frequency anomalies JSON\n"
              << "  --json-len          : Export length anomalies JSON\n"
              << "  --json-entropy      : Export entropy anomalies JSON\n"
              << "\nOTHER FLAGS:\n"
              << "  --help              : Show this help message\n";
}

int main(int argc, char* argv[]) {
    // Basic argument check
    if (argc < 3) {
        print_help(argv[0]);
        return 1;
    }

    // Check for help flag
    for(int i=1; i<argc; i++) {
        if(std::string(argv[i]) == "--help") { print_help(argv[0]); return 0; }
    }

    std::string input_text = argv[1];
    std::string output_dir = argv[2];

    // Normalize output directory path (remove trailing slash)
    if (output_dir.back() == '/' || output_dir.back() == '\\') output_dir.pop_back();

    // Variables for Preprocess configuration
    std::string pp_regex = "";
    std::string pp_delim = "";
    std::string pp_ignore = "";

    // Variables for Analyze configuration
    std::string ana_perc_freq = "";
    std::string ana_perc_len = "";
    std::string ana_perc_entropy = "";

    // Variables for JSON configuration
    bool json_complete = false;
    bool json_partial = false;
    bool json_freq = false;
    bool json_len = false;
    bool json_entropy = false;

    // --- PARSING FLAGS ---
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];

        // 1. Capture Preprocess flags
        if (starts_with(arg, "--regex=")) {
            pp_regex = arg.substr(8); // Get value after '='
        }
        else if (starts_with(arg, "--delim=")) {
            pp_delim = arg.substr(8);
        }
        else if (starts_with(arg, "--ignore=")) {
            pp_ignore = arg.substr(9);
        }
        // 2. Capture Analyze flags
        else if (starts_with(arg, "--perc-freq=")) {
            ana_perc_freq = arg.substr(12);
        }
        else if (starts_with(arg, "--perc-len=")) {
            ana_perc_len = arg.substr(11);
        }
        else if (starts_with(arg, "--perc-entropy=")) {
            ana_perc_entropy= arg.substr(15);
        }
        // 3. Capture JSON flags
        else if (arg == "--json-complete") json_complete = true;
        else if (arg == "--json-partial") json_partial = true;
        else if (arg == "--json-freq") json_freq = true;
        else if (arg == "--json-len") json_len = true;
        else if (arg == "--json-entropy") json_entropy = true;

        else {
            std::cout << "[WARNING] Unknown flag: " << arg << std::endl;
        }
    }

    // --- STEP 0: CREATE OUTPUT DIRECTORY ---
    std::filesystem::create_directories(output_dir);

    // --- STEP 1: PREPROCESS ---
    std::string cleaned_input = output_dir + "/cleaned_data.txt";
    std::stringstream pp_cmd;
    
    // Base command
    pp_cmd << BIN_PREPROCESS << " \"" << input_text << "\" \"" << cleaned_input << "\"";

    // Priority logic: Regex > Delim/Ignore
    if (!pp_regex.empty()) {
        // Regex mode
        pp_cmd << " --regex=\"" << pp_regex << "\""; 
    } else {
        // Default mode (Delim/Ignore)
        if (!pp_delim.empty())  pp_cmd << " --delim=\"" << pp_delim << "\"";
        if (!pp_ignore.empty()) pp_cmd << " --ignore=\"" << pp_ignore << "\"";
    }

    // std::cout << pp_cmd.str() << std::endl;
    run_command("Preprocess Module", pp_cmd.str());

    // --- STEP 2: ANALYZE ---
    std::stringstream analyze_cmd;
    analyze_cmd << BIN_ANALYZE << " \"" << cleaned_input << "\" \"" << output_dir << "\"";

    // Percentile
    if (!ana_perc_freq.empty()) {
        analyze_cmd << " --perc-freq=" << ana_perc_freq;
    }
    if (!ana_perc_len.empty()) {
        analyze_cmd << " --perc-len=" << ana_perc_len;
    }
    if (!ana_perc_entropy.empty()) {
        analyze_cmd << " --perc-entropy=" << ana_perc_entropy;
    }
    
    if (json_complete) analyze_cmd << " --json-complete";
    if (json_partial)  analyze_cmd << " --json-partial";
    if (json_freq)     analyze_cmd << " --json-freq";
    if (json_len)      analyze_cmd << " --json-len";
    if (json_entropy)  analyze_cmd << " --json-entropy";

    // std::cout << analyze_cmd.str() << std::endl;
    run_command("Analyze Module", analyze_cmd.str());

    std::cout << "\nResults at: " << output_dir << std::endl;
    return 0;
}