// Priority Branching for CaDiCaL
// Allows forcing branching order from an external file
// Supports CaDiCaL options and proof output

#include "src/cadical.hpp"
#include "src/priority_branching_propagator.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

using namespace std;
using namespace CaDiCaL;

void print_usage(const char* program_name) {
  cout << "Usage: " << program_name << " [options] <cnf-file> <branch-list-file>" << endl;
  cout << endl;
  cout << "Options:" << endl;
  cout << "  -h, --help              Show this help message" << endl;
  cout << "  -v, --verbose           Enable verbose output" << endl;
  cout << "  -q, --quiet             Disable all output except result" << endl;
  cout << "  -n, --no-witness        Do not print witness (solution)" << endl;
  cout << "  --cadical-verbose       Show CaDiCaL's detailed output" << endl;
  cout << "  --cadical-quiet         Hide CaDiCaL's output (default)" << endl;
  cout << "  --proof=<file>          Write proof to file (DRAT format)" << endl;
  cout << "  --lrat                  Use LRAT proof format" << endl;
  cout << "  --frat                  Use FRAT proof format" << endl;
  cout << "  --binary                Use binary proof format (default)" << endl;
  cout << "  --no-binary             Use text proof format" << endl;
  cout << "  --plain                 Disable all preprocessing" << endl;
  cout << "  -o <name>=<val>         Set CaDiCaL option" << endl;
  cout << "  --<name>=<val>          Set CaDiCaL option (long form)" << endl;
  cout << "  --<name>                Set boolean option to true" << endl;
  cout << "  --no-<name>             Set boolean option to false" << endl;
  cout << endl;
  cout << "Branch list file format:" << endl;
  cout << "  - One signed integer per line" << endl;
  cout << "  - Positive: branch variable to true" << endl;
  cout << "  - Negative: branch variable to false" << endl;
  cout << "  - Lines starting with '#' or 'c' are comments" << endl;
  cout << "  - Empty lines are ignored" << endl;
  cout << endl;
  cout << "Examples:" << endl;
  cout << "  " << program_name << " problem.cnf branches.txt" << endl;
  cout << "  " << program_name << " -v problem.cnf branches.txt" << endl;
  cout << "  " << program_name << " --proof=proof.drat problem.cnf branches.txt" << endl;
  cout << "  " << program_name << " -o check=1 problem.cnf branches.txt" << endl;
  cout << "  " << program_name << " --lrat --proof=proof.lrat problem.cnf branches.txt" << endl;
  cout << endl;
  cout << "Common CaDiCaL options:" << endl;
  cout << "  --check=1               Enable proof checking" << endl;
  cout << "  --plain                 Disable all preprocessing" << endl;
  cout << "  --no-binary             Use text (ASCII) proof format" << endl;
  cout << "  --chrono=0              Disable chronological backtracking" << endl;
  cout << "  --phase=1               Set initial phase (0=false, 1=true)" << endl;
  cout << "  --stable=0              Disable stable mode" << endl;
  cout << endl;
}

int main(int argc, char** argv) {
  bool verbose = false;
  bool quiet = false;
  bool print_witness = true;
  bool cadical_verbose = false;
  bool cadical_quiet = true;  // Default: hide CaDiCaL output
  bool use_plain = false;
  string cnf_file;
  string branch_file;
  string proof_file;
  vector<string> options;
  
  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    
    if (arg == "-h" || arg == "--help") {
      print_usage(argv[0]);
      return 0;
    } else if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    } else if (arg == "-q" || arg == "--quiet") {
      quiet = true;
    } else if (arg == "-n" || arg == "--no-witness") {
      print_witness = false;
    } else if (arg == "--cadical-verbose") {
      cadical_verbose = true;
      cadical_quiet = false;
    } else if (arg == "--cadical-quiet") {
      cadical_verbose = false;
      cadical_quiet = true;
    } else if (arg.substr(0, 8) == "--proof=") {
      proof_file = arg.substr(8);
    } else if (arg == "-o" && i + 1 < argc) {
      options.push_back(argv[++i]);
    } else if (arg == "--plain") {
      use_plain = true;
    } else if (arg.substr(0, 2) == "--") {
      // Long option for CaDiCaL
      options.push_back(arg.substr(2));
    } else if (cnf_file.empty()) {
      cnf_file = arg;
    } else if (branch_file.empty()) {
      branch_file = arg;
    } else {
      cerr << "Error: Too many arguments: " << arg << endl;
      print_usage(argv[0]);
      return 1;
    }
  }
  
  if (cnf_file.empty() || branch_file.empty()) {
    cerr << "Error: Missing required arguments" << endl;
    print_usage(argv[0]);
    return 1;
  }
  
  // Create solver and propagator
  Solver solver;
  PriorityBranchingPropagator propagator(verbose);
  
  // Apply plain mode if requested
  if (use_plain) {
    solver.configure("plain");
    if (!quiet) {
      cout << "c Plain mode enabled (preprocessing disabled)" << endl;
      cout << "c" << endl;
    }
  }
  
  solver.set("verbose", 1);
  solver.set("quiet", 0);
  
  // Set CaDiCaL verbosity
  // if (cadical_verbose) {
  //   solver.set("verbose", 1);
  //   solver.set("quiet", 0);
  // } else if (cadical_quiet) {
  //   solver.set("verbose", 0);
  //   solver.set("quiet", 1);
  // }
  
  if (!quiet) {
    cout << "c" << endl;
    cout << "c Priority Branching SAT Solver (CaDiCaL)" << endl;
    cout << "c" << endl;
    cout << "c CNF file:        " << cnf_file << endl;
    cout << "c Branch list:     " << branch_file << endl;
    if (!proof_file.empty()) {
      cout << "c Proof output:    " << proof_file << endl;
    }
    cout << "c" << endl;
  }
  
  // Set CaDiCaL options
  if (!options.empty() && !quiet) {
    cout << "c Setting options:" << endl;
  }
  for (const string& opt : options) {
    if (opt.find('=') != string::npos) {
      // Option with value: name=value
      size_t pos = opt.find('=');
      string name = opt.substr(0, pos);
      int value = atoi(opt.substr(pos + 1).c_str());
      if (!solver.set(name.c_str(), value)) {
        cerr << "Warning: Failed to set option '" << name << "'" << endl;
      } else if (!quiet) {
        cout << "c   " << name << " = " << value << endl;
      }
    } else if (opt.substr(0, 3) == "no-") {
      // Boolean option: no-name
      string name = opt.substr(3);
      if (!solver.set(name.c_str(), 0)) {
        cerr << "Warning: Failed to set option '" << name << "'" << endl;
      } else if (!quiet) {
        cout << "c   " << name << " = 0" << endl;
      }
    } else {
      // Boolean option or format like "lrat", "frat", "binary"
      // Try setting to 1
      if (!solver.set(opt.c_str(), 1)) {
        cerr << "Warning: Failed to set option '" << opt << "'" << endl;
      } else if (!quiet) {
        cout << "c   " << opt << " = 1" << endl;
      }
    }
  }
  
  if (!options.empty() && !quiet) {
    cout << "c" << endl;
  }
  
  // Enable proof output BEFORE reading CNF (required by CaDiCaL API)
  if (!proof_file.empty()) {
    if (verbose) {
      cout << "c Enabling proof output to: " << proof_file << endl;
    }
    if (!solver.trace_proof(proof_file.c_str())) {
      cerr << "Error: Failed to open proof file: " << proof_file << endl;
      return 1;
    }
    if (!quiet) {
      cout << "c Proof tracing enabled" << endl;
      cout << "c" << endl;
    }
  }
  
  // Read CNF file
  if (verbose) {
    cout << "c Reading CNF file..." << endl;
  }
  
  int max_var = 0;
  const char* parse_error = solver.read_dimacs(cnf_file.c_str(), max_var, 1);
  if (parse_error) {
    cerr << "Error: Cannot parse CNF file: " << parse_error << endl;
    return 1;
  }
  
  max_var = solver.vars();
  if (!quiet) {
    cout << "c Parsed: " << max_var << " variables, " 
         << solver.active() << " clauses" << endl;
    cout << "c" << endl;
  }
  
  // Connect external propagator
  solver.connect_external_propagator(&propagator);
  
  // Load branch list from file
  if (verbose) {
    cout << "c Loading branch list..." << endl;
  }
  
  if (!propagator.load_branch_list_from_file(branch_file)) {
    cerr << "Error: Failed to load branch list from: " << branch_file << endl;
    solver.disconnect_external_propagator();
    return 1;
  }
  
  const vector<int>& branch_list = propagator.get_branch_on_list();
  if (!quiet) {
    cout << "c Loaded " << branch_list.size() << " priority variables" << endl;
  }
  
  // Mark all branching variables as observed
  if (verbose) {
    cout << "c Adding observed variables..." << endl;
  }
  
  int observed_count = 0;
  for (int lit : branch_list) {
    int var = abs(lit);
    if (var > max_var) {
      cerr << "Warning: Variable " << var 
           << " in branch list exceeds max variable " << max_var << endl;
    } else {
      solver.add_observed_var(var);
      observed_count++;
      if (verbose) {
        cout << "c   Observed variable: " << var << endl;
      }
    }
  }
  
  if (!quiet) {
    cout << "c Added " << observed_count << " observed variables" << endl;
    cout << "c" << endl;
  }
  
  // Solve
  if (!quiet) {
    cout << "c Solving..." << endl;
    if (verbose) {
      cout << "c =============================" << endl;
    }
  }
  
  int result = solver.solve();
  
  if (verbose && !quiet) {
    cout << "c =============================" << endl;
  }
  
  if (!quiet) {
    cout << "c" << endl;
    cout << "c Solving finished" << endl;
    cout << "c" << endl;
  }
  
  // Flush and close proof if enabled
  if (!proof_file.empty()) {
    solver.flush_proof_trace();
    solver.close_proof_trace();
    if (!quiet) {
      cout << "c Proof written to: " << proof_file << endl;
      cout << "c" << endl;
    }
  }
  
  // Output result
  if (result == 10) {
    cout << "s SATISFIABLE" << endl;
    
    // Print witness (solution)
    if (print_witness) {
      cout << "v ";
      for (int i = 1; i <= max_var; i++) {
        int val = solver.val(i);
        if (val > 0) {
          cout << i << " ";
        } else if (val < 0) {
          cout << -i << " ";
        }
      }
      cout << "0" << endl;
    }
    
    if (!quiet) {
      cout << "c" << endl;
      cout << "c Priority branching statistics:" << endl;
      cout << "c   Total priority variables: " << branch_list.size() << endl;
      cout << "c   Processed: " << propagator.get_processed_count() << endl;
      cout << "c   Remaining: " << propagator.get_remaining_count() << endl;
    }
    
  } else if (result == 20) {
    cout << "s UNSATISFIABLE" << endl;
  } else {
    cout << "s UNKNOWN" << endl;
  }
  
  // Disconnect propagator
  solver.disconnect_external_propagator();
  
  return result == 10 ? 10 : result == 20 ? 20 : 0;
}

