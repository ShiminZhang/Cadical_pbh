# Priority Branching - Quick Start Guide

## ✨ What This Does

Force CaDiCaL SAT solver to branch on variables in a specific order that YOU specify!

## 🚀 Quick Examples

### 1. Basic Usage
```bash
./priority_branch test.cnf branch_list.txt
```

### 2. With Verbose Output
```bash
./priority_branch -v test.cnf branch_list.txt
```

### 3. Generate Proof
```bash
./priority_branch --proof=proof.drat test.cnf branch_list.txt
```

### 4. Set CaDiCaL Options
```bash
./priority_branch -o check=1 -o phase=0 test.cnf branch_list.txt
```

## 📝 Branch List Format

Create a file (e.g., `my_branches.txt`):
```
# Comments start with # or c
-3    # Branch variable 3 to false first
1     # Then variable 1 to true
-2    # Then variable 2 to false
4     # Then variable 4 to true
```

**Rules:**
- Positive number = set variable to TRUE
- Negative number = set variable to FALSE
- One variable per line

## 🎯 Common Use Cases

### Case 1: Explore Specific Search Path
```bash
# Create your branch order
echo "1" > branches.txt
echo "-2" >> branches.txt
echo "3" >> branches.txt

# Solve
./priority_branch problem.cnf branches.txt
```

### Case 2: Generate and Verify Proof
```bash
# Generate DRAT proof
./priority_branch --proof=proof.drat problem.cnf branches.txt

# Verify with drat-trim (if you have it)
drat-trim problem.cnf proof.drat
```

### Case 3: Debug with Verbose Mode
```bash
./priority_branch -v problem.cnf branches.txt
```

Shows:
```
Priority branch: -3 (index 0/10)
Priority branch: 1 (index 1/10)
...
```

## 📊 Understanding Output

### SAT Result
```
s SATISFIABLE
v -1 2 -3 4 0        ← Solution
c
c Priority branching statistics:
c   Total priority variables: 4
c   Processed: 3      ← Used 3 from your list
c   Remaining: 1      ← Didn't need the last one
```

### UNSAT Result
```
s UNSATISFIABLE
```

## 🔧 All Command Line Options

| Option | Description |
|--------|-------------|
| `-v, --verbose` | Show branching details |
| `-q, --quiet` | Minimal output |
| `-n, --no-witness` | Don't print solution |
| `--proof=FILE` | Write proof (DRAT) |
| `--lrat` | Use LRAT proof format |
| `--frat` | Use FRAT proof format |
| `--binary` | Binary proof format |
| `-o NAME=VAL` | Set CaDiCaL option |

## 🎓 Proof Output

### DRAT Proof (most common)
```bash
./priority_branch --proof=proof.drat problem.cnf branches.txt
```

### LRAT Proof (with hints)
```bash
./priority_branch --lrat --proof=proof.lrat problem.cnf branches.txt
```

### Binary Proof (compressed)
```bash
./priority_branch --binary --proof=proof.drat problem.cnf branches.txt
```

## 💡 Tips

1. **Don't specify all variables** - Only specify the important ones. The solver will decide the rest.

2. **Order matters** - Variables are branched in the order you list them.

3. **Positive vs Negative** - Controls the initial assignment direction:
   - `3` means "try variable 3 = true first"
   - `-3` means "try variable 3 = false first"

4. **Proof only for UNSAT** - Proofs are only meaningful for UNSAT instances. For SAT, the solution IS the proof!

## 📁 Files

| File | Purpose |
|------|---------|
| `priority_branch` | Main executable |
| `test.cnf` | Example CNF problem |
| `branch_list.txt` | Example branch list |
| `README_USAGE.md` | Detailed documentation |
| `PROOF_GUIDE.txt` | Proof output reference |

## ❓ Help

```bash
./priority_branch --help
```

## 🐛 Common Issues

**Problem**: Variables not branching in order
**Solution**: Make sure variables in branch list exist in CNF file

**Problem**: "can only start proof tracing right after initialization"
**Solution**: Already fixed! Proof is enabled before reading CNF.

**Problem**: Proof file empty
**Solution**: Already handled! Proof is flushed and closed automatically.

## 📚 More Info

- Full usage guide: `README_USAGE.md`
- Proof details: `PROOF_GUIDE.txt`
- Example code: `priority_branch.cpp`

---
**Happy Solving! 🎉**

