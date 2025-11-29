#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cstring> // For strncmp

// Structure to store visualization tasks
struct VisualTask {
    std::string json_path;
    std::string png_path;
};

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
              << "VISUALIZATION FLAGS:\n"
              << "  --visual-complete   : Visualize the complete Trie\n"
              << "  --visual-partial    : Visualize partial Trie (show anomalies only)\n"
              << "  --visual-freq       : Visualize frequency anomalies\n"
              << "  --visual-len        : Visualize length anomalies\n"
              << "  --visual-entropy    : Visualize entropy anomalies\n"
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

    // Variables for Visualize configuration
    bool vis_complete = false;
    bool vis_partial = false;
    bool vis_freq = false;
    bool vis_len = false;
    bool vis_entropy = false;

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
        // 3. Capture Visualize flags
        else if (arg == "--visual-complete") vis_complete = true;
        else if (arg == "--visual-partial") vis_partial = true;
        else if (arg == "--visual-freq") vis_freq = true;
        else if (arg == "--visual-len") vis_len = true;
        else if (arg == "--visual-entropy") vis_entropy = true;

        else {
            std::cout << "[WARNING] Unknown flag: " << arg << std::endl;
        }
    }

    // --- STEP 0: CREATE OUTPUT DIRECTORY ---
    std::string mkdir_cmd = "mkdir -p \"" + output_dir + "\"";
    std::system(mkdir_cmd.c_str());

    // --- STEP 1: PREPROCESS ---
    std::string cleaned_input = output_dir + "/cleaned_data.txt";
    std::stringstream pp_cmd;
    
    // Base command
    pp_cmd << "bin/preprocess \"" << input_text << "\" \"" << cleaned_input << "\"";

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
    analyze_cmd << "bin/analyze \"" << cleaned_input << "\" \"" << output_dir << "\"";

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
    
    std::vector<VisualTask> tasks;

    // Helper lambda to add tasks
    auto addTask = [&](const std::string& flag, const std::string& name) {
        analyze_cmd << " " << flag;
        std::string json = output_dir + "/" + name + ".json";
        std::string png = output_dir + "/" + name + ".png";
        tasks.push_back({json, png});
    };

    if (vis_complete) addTask("--json-complete", "complete_trie");
    if (vis_partial)  addTask("--json-partial", "partial_trie");
    if (vis_freq)     addTask("--json-freq", "frequency_anomalies");
    if (vis_len)      addTask("--json-len", "length_anomalies");
    if (vis_entropy)  addTask("--json-entropy", "entropy_anomalies");

    // std::cout << analyze_cmd.str() << std::endl;
    run_command("Analyze Module", analyze_cmd.str());

    // --- STEP 3: VISUALIZE ---
    if (tasks.empty()) {
        std::cout << "[INFO] No visualization requested." << std::endl;
    } else {
        for (const auto& task : tasks) {
            std::stringstream vis_cmd;
            vis_cmd << "bin/visualize \"" << task.json_path << "\" \"" << task.png_path << "\"";
            // std::cout << vis_cmd.str() << std::endl;
            run_command("Visualize (" + task.json_path + ")", vis_cmd.str());
        }
    }

    std::cout << "\nResults at: " << output_dir << std::endl;
    return 0;
}