#ifndef _priority_branching_propagator_hpp_INCLUDED
#define _priority_branching_propagator_hpp_INCLUDED

#include "cadical.hpp"
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

namespace CaDiCaL {

// Priority Branching Propagator
// Forces the solver to branch on variables in a specified order
class PriorityBranchingPropagator : public ExternalPropagator {
private:
  std::vector<int> branch_on_list;        // Priority branching variable list
  size_t branch_index;                     // Current index
  std::unordered_set<int> assigned_vars;   // Set of assigned variables
  std::vector<size_t> decision_stack;      // Decision stack for backtracking
  bool verbose;                            // Verbose output flag

public:
  PriorityBranchingPropagator(bool verbose = false) 
    : branch_index(0), verbose(verbose) {}

  // Load priority branching list from file
  // File format: one signed integer per line
  // Positive: branch variable to true, Negative: branch variable to false
  // Lines starting with '#' or 'c' are comments
  bool load_branch_list_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
      return false;
    }

    branch_on_list.clear();
    branch_index = 0;
    assigned_vars.clear();
    
    std::string line;
    int line_num = 0;
    
    while (std::getline(file, line)) {
      line_num++;
      
      // Remove leading/trailing whitespace
      size_t start = line.find_first_not_of(" \t\r\n");
      if (start == std::string::npos) continue;  // Empty line
      
      size_t end = line.find_last_not_of(" \t\r\n");
      line = line.substr(start, end - start + 1);
      
      // Skip comment lines
      if (line.empty() || line[0] == '#' || line[0] == 'c') {
        continue;
      }
      
      // Parse integer
      std::istringstream iss(line);
      int var;
      if (!(iss >> var)) {
        std::cerr << "Warning: Invalid integer at line " << line_num 
                  << ": '" << line << "'" << std::endl;
        continue;
      }
      
      if (var == 0) {
        std::cerr << "Warning: Variable 0 is invalid at line " << line_num << std::endl;
        continue;
      }
      
      branch_on_list.push_back(var);
    }
    
    file.close();
    
    if (verbose) {
      std::cout << "Loaded " << branch_on_list.size() 
                << " priority branching variables from '" << filename << "'" << std::endl;
      if (!branch_on_list.empty() && branch_on_list.size() <= 20) {
        std::cout << "Variables: ";
        for (int var : branch_on_list) {
          std::cout << var << " ";
        }
        std::cout << std::endl;
      }
    }
    
    return !branch_on_list.empty();
  }

  // Manually set priority branching list
  void set_branch_on_list(const std::vector<int>& vars) {
    branch_on_list = vars;
    branch_index = 0;
    assigned_vars.clear();
    
    if (verbose) {
      std::cout << "Set " << vars.size() << " priority branching variables" << std::endl;
    }
  }

  // Get current branch list
  const std::vector<int>& get_branch_on_list() const {
    return branch_on_list;
  }

  // Clear branch list
  void clear_branch_on_list() {
    branch_on_list.clear();
    branch_index = 0;
    assigned_vars.clear();
  }

  // Get number of processed variables
  size_t get_processed_count() const {
    return branch_index;
  }

  // Get number of remaining variables
  size_t get_remaining_count() const {
    return branch_on_list.size() > branch_index ? 
           branch_on_list.size() - branch_index : 0;
  }

  // ===== ExternalPropagator Interface =====

  // Notify assignment
  void notify_assignment(const std::vector<int>& lits) override {
    for (int lit : lits) {
      assigned_vars.insert(abs(lit));
    }
  }

  // Notify new decision level
  void notify_new_decision_level() override {
    decision_stack.push_back(branch_index);
  }

  // Notify backtrack
  void notify_backtrack(size_t new_level) override {
    // Clear assigned variables (simplified approach)
    assigned_vars.clear();
    
    // Restore branch_index
    if (new_level < decision_stack.size()) {
      if (new_level > 0)
        branch_index = decision_stack[new_level - 1];
      else
        branch_index = 0;
      decision_stack.resize(new_level);
    }
  }

  // Decision callback - return next unassigned variable from priority list
  int cb_decide() override {
    // Iterate through priority branching list
    while (branch_index < branch_on_list.size()) {
      int lit = branch_on_list[branch_index];
      int var = abs(lit);
      
      // If variable is unassigned, return it
      if (assigned_vars.find(var) == assigned_vars.end()) {
        branch_index++;
        
        if (verbose) {
          std::cout << "Priority branch: " << lit 
                    << " (index " << (branch_index - 1) << "/" 
                    << branch_on_list.size() << ")" << std::endl;
        }
        
        return lit;  // Return signed literal
      }
      
      // Already assigned, skip
      branch_index++;
    }
    
    // All variables in list processed, let solver decide
    return 0;
  }

  // Check found model - accept any model
  bool cb_check_found_model(const std::vector<int>& model) override {
    (void) model;
    return true;
  }

  // External propagation - not used
  int cb_propagate() override {
    return 0;
  }

  // External clause callbacks - not used
  bool cb_has_external_clause(bool& is_forgettable) override {
    (void) is_forgettable;
    return false;
  }

  int cb_add_external_clause_lit() override {
    return 0;
  }
};

} // namespace CaDiCaL

#endif // _priority_branching_propagator_hpp_INCLUDED
