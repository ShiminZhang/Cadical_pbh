# Priority Branching for CaDiCaL - Usage Guide

## Quick Start

### Basic Usage
```bash
./priority_branch <cnf-file> <branch-list-file>
```

### With Options
```bash
./priority_branch [options] <cnf-file> <branch-list-file>
```

## Command Line Options

### General Options
| Option | Description |
|--------|-------------|
| `-h, --help` | Show help message |
| `-v, --verbose` | Enable verbose output (shows branching details) |
| `-q, --quiet` | Quiet mode (only show result) |
| `-n, --no-witness` | Don't print solution |

### Proof Output Options

#### Basic Proof Output
```bash
# DRAT format (default)
./priority_branch --proof=output.drat problem.cnf branches.txt

# LRAT format (with antecedents)
./priority_branch --lrat --proof=output.lrat problem.cnf branches.txt

# FRAT format
./priority_branch --frat --proof=output.frat problem.cnf branches.txt

# Binary format (compressed)
./priority_branch --binary --proof=output.drat problem.cnf branches.txt
```

#### Proof Format Comparison

| Format | Full Name | Features | File Size |
|--------|-----------|----------|-----------|
| **DRAT** | Deletion Resolution Asymmetric Tautology | Standard format, widely supported | Medium |
| **LRAT** | Literal Resolution Asymmetric Tautology | Includes hints (antecedents), easier to check | Large |
| **FRAT** | FRAT format | Modern format with finalization | Medium |
| **Binary** | Binary DRAT | Compressed binary format | Small |

### CaDiCaL Options

#### Setting Options
```bash
# Short form
./priority_branch -o <name>=<value> problem.cnf branches.txt

# Long form
./priority_branch --<name>=<value> problem.cnf branches.txt

# Boolean options
./priority_branch --<name> problem.cnf branches.txt        # Set to true
./priority_branch --no-<name> problem.cnf branches.txt     # Set to false
```

#### Common Options

| Option | Default | Description |
|--------|---------|-------------|
| `check=1` | 0 | Enable internal proof checking |
| `chrono=0` | 1 | Disable chronological backtracking |
| `phase=1` | 1 | Initial phase (0=false, 1=true) |
| `stable=0` | 1 | Disable stable mode |
| `verbose=1` | 0 | CaDiCaL verbose output |

## Branch List File Format

### Format Rules
- **One signed integer per line**
- **Positive integer**: Branch variable to `true`
- **Negative integer**: Branch variable to `false`
- **Comments**: Lines starting with `#` or `c`
- **Empty lines**: Ignored

### Example File (branches.txt)
```
# Priority branching list
# Branch on these variables in order

-3    # Set variable 3 to false first
1     # Then set variable 1 to true
-2    # Then set variable 2 to false
4     # Then set variable 4 to true
```

## Usage Examples

### Example 1: Basic Solving
```bash
./priority_branch problem.cnf branches.txt
```

Output:
```
c
c Priority Branching SAT Solver (CaDiCaL)
c
c CNF file:        problem.cnf
c Branch list:     branches.txt
c
c Parsed: 100 variables, 250 clauses
c
c Loaded 10 priority variables
c Added 10 observed variables
c
c Solving...
c
s SATISFIABLE
v 1 -2 3 -4 5 ... 0
c
c Priority branching statistics:
c   Total priority variables: 10
c   Processed: 8
c   Remaining: 2
```

### Example 2: Verbose Mode
```bash
./priority_branch -v problem.cnf branches.txt
```

Shows detailed branching information:
```
Priority branch: -3 (index 0/10)
Priority branch: 1 (index 1/10)
...
```

### Example 3: Generate DRAT Proof
```bash
./priority_branch --proof=proof.drat problem.cnf branches.txt
```

Output file `proof.drat` contains the proof trace in DRAT format.

### Example 4: Generate LRAT Proof with Checking
```bash
./priority_branch --lrat --proof=proof.lrat -o check=1 problem.cnf branches.txt
```

- Generates LRAT format proof
- Enables internal checking
- Output: `proof.lrat`

### Example 5: Multiple Options
```bash
./priority_branch \
  --proof=proof.drat \
  -o check=1 \
  -o verbose=1 \
  --no-stable \
  problem.cnf branches.txt
```

Sets multiple options:
- Proof output to `proof.drat`
- Enable checking
- Enable CaDiCaL verbose
- Disable stable mode

### Example 6: Quiet Mode (Minimal Output)
```bash
./priority_branch -q problem.cnf branches.txt
```

Output only:
```
s SATISFIABLE
v 1 -2 3 -4 ... 0
```

### Example 7: No Witness (Don't Print Solution)
```bash
./priority_branch -n problem.cnf branches.txt
```

Output:
```
s SATISFIABLE
```
(No `v` line)

## Verifying Proofs

### Using drat-trim (for DRAT proofs)
```bash
# Download drat-trim from https://github.com/marijnheule/drat-trim
./drat-trim problem.cnf proof.drat
```

### Using lrat-check (for LRAT proofs)
```bash
# For LRAT format proofs
./lrat-check problem.cnf proof.lrat
```

## Advanced Usage

### Example: Custom Configuration
```bash
./priority_branch \
  --proof=proof.lrat \
  --lrat \
  -o check=1 \
  -o chrono=0 \
  -o phase=0 \
  -v \
  problem.cnf branches.txt
```

### Example: Binary Compressed Proof
```bash
./priority_branch \
  --binary \
  --proof=proof.drat \
  problem.cnf branches.txt
```
(Much smaller file size)

### Example: Proof to stdout
```bash
./priority_branch --proof=/dev/stdout problem.cnf branches.txt > proof.drat
```

## Integration in C++ Code

### Example 1: Basic Usage
```cpp
#include "src/cadical.hpp"
#include "src/priority_branching_propagator.hpp"

int main() {
    CaDiCaL::Solver solver;
    CaDiCaL::PriorityBranchingPropagator propagator;
    
    // Enable proof BEFORE reading CNF
    solver.trace_proof("proof.drat");
    
    // Read CNF
    int vars = 0;
    solver.read_dimacs("problem.cnf", vars, 1);
    
    // Connect propagator
    solver.connect_external_propagator(&propagator);
    
    // Load branches
    propagator.load_branch_list_from_file("branches.txt");
    
    // Mark observed variables
    for (int lit : propagator.get_branch_on_list()) {
        solver.add_observed_var(abs(lit));
    }
    
    // Solve
    int result = solver.solve();
    
    // Clean up
    solver.flush_proof_trace();
    solver.close_proof_trace();
    solver.disconnect_external_propagator();
    
    return result == 10 ? 0 : 1;
}
```

### Example 2: With Options
```cpp
// Set options
solver.set("check", 1);
solver.set("verbose", 1);
solver.set("chrono", 0);

// Enable LRAT proof
solver.set("lrat", 1);
solver.trace_proof("proof.lrat");

// ... rest of code ...
```

## Troubleshooting

### Issue: "can only start proof tracing right after initialization"
**Solution**: Call `trace_proof()` BEFORE `read_dimacs()`.

Correct order:
```cpp
solver.trace_proof("proof.drat");  // First
solver.read_dimacs("problem.cnf");  // Then
```

### Issue: Variables not branching in order
**Solution**: Make sure to call `add_observed_var()` for each variable in the branch list.

```cpp
for (int lit : propagator.get_branch_on_list()) {
    solver.add_observed_var(abs(lit));  // Required!
}
```

### Issue: Proof file is empty
**Solution**: Call `flush_proof_trace()` and `close_proof_trace()` before exiting.

```cpp
solver.flush_proof_trace();
solver.close_proof_trace();
```

## Output Format

### SAT Result
```
s SATISFIABLE
v <signed literals> 0
```

### UNSAT Result
```
s UNSATISFIABLE
```

### With Statistics
```
c Priority branching statistics:
c   Total priority variables: N
c   Processed: M
c   Remaining: K
```

## Files Created

| File | Description |
|------|-------------|
| `priority_branch` | Main executable |
| `src/priority_branching_propagator.hpp` | Propagator implementation |
| `branch_list.txt` | Example branch list |
| `test.cnf` | Example CNF file |

## References

- CaDiCaL: https://github.com/arminbiere/cadical
- DRAT format: http://www.cs.utexas.edu/~marijn/drat-trim/
- LRAT format: https://www.cs.utexas.edu/~marijn/publications/
- External Propagator API: `src/cadical.hpp` (line 1276)

## Help

For more options:
```bash
./priority_branch --help
```

