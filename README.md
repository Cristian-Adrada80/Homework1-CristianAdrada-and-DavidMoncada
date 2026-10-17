# Formal Languages — Assignment 1: DFA Minimization

Implementation of the minimization algorithm of Kozen (1997), Lectures 13–14.
Given a DFA with no inaccessible states, the program prints the pairs of
equivalent states — the pairs that can be collapsed to obtain the minimal
automaton.

## Students

**Class number:** S2666-0509
- Cristian Alejandro Adrada Ordoñez
- David Moncada Correa

## Environment

| Item | Version |
| --- | --- |
| Operating system | Windows 11 Home Single Language (build 26200) |
| Language | C++17 |
| Compiler | MinGW GCC g++ 13.1.0 (bundled with CLion) |
| Build system | CMake 4.2.2 (bundled with CLion; 3.16 or newer works) |
| IDE | CLion 2026.1.1 |

## Build and run

### CLion

1. `File > Open...` and select the project folder (the one with `CMakeLists.txt`).
   CLion configures CMake automatically.
2. Build the `minimize` target (`Ctrl+F9`).
3. In `Run > Edit Configurations...` set **Program arguments** to an input file
   path and press `Shift+F10`.

### Command line

```bash
# with CMake
cmake -S . -B build
cmake --build build
./build/minimize examples/input.txt     # from a file
./build/minimize < examples/input.txt    # from standard input

# or a single compile command
g++ -std=c++17 -O2 -Iinclude -o minimize src/*.cpp
```

## Input

The format is the one in the assignment statement:

```
c                     number of cases
  n                   number of states
  a b ...             alphabet (blank-separated)
  f1 f2 ...           final states (blank-separated; may be empty)
  <state> <δ(state,a1)> ... <δ(state,ak)>   (n rows)
```

States are natural numbers, the start state is always `0`, and the alphabet
uses lowercase latin letters.

## Output

For each case the program prints one line with the equivalent pairs in
lexicographical order, separated by blanks:

```
(1, 2) (3, 4)
(1, 2) (3, 4) (3, 5) (4, 5)
(0, 3) (1, 4) (2, 5)
(0, 1)
```

After those result lines, and only as an extra aid, the program also prints for
each case the transition table of the original and the minimized automaton
(`->` marks the start state, `<-` a final state) plus the Graphviz source of
both, ready to paste into a viewer such as
<https://dreampuf.github.io/GraphvizOnline>:

```
========== Case 1 ==========
Original automaton:
        a  b
->  0   1  2
<-  1   3  4
...
Minimized automaton (6 -> 4 states):
...
To see the automata, paste the following code blocks into Graphviz Online:
  # Original automaton (case 1)
digraph original { ... }
  # Minimized automaton (case 1)
digraph minimized { ... }
```

Special cases: a case with no equivalent pairs prints only
`(Case N has no equivalent pairs)`; a case with inaccessible states (which the
statement rules out) prints only a warning on standard error.

### Option

`-d, --dot <prefix>` also saves the Graphviz source to files
`<prefix>_case<i>_original.dot` and `<prefix>_case<i>_minimized.dot`. Render one
with `dot -Tpng <file>.dot -o <file>.png` if Graphviz is installed, or paste it
into an online viewer.

## Algorithm

Two states `p` and `q` are **equivalent** when, for every string `x`, `δ̂(p, x)`
is final if and only if `δ̂(q, x)` is final; otherwise they are
**distinguishable**. The program (Kozen, Lecture 14) marks every distinguishable
pair; the pairs left unmarked are the equivalent ones.

1. **Base case.** A pair `{p, q}` is marked when exactly one of `p`, `q` is
   final: the empty string already tells them apart.
2. **Induction.** If `{δ(p, a), δ(q, a)}` is already marked for some symbol `a`,
   then `{p, q}` is marked too: if `x` distinguishes the successors, then `a·x`
   distinguishes `p` and `q`.
3. **Fixed point.** Step 2 repeats until nothing new is marked.

The implementation propagates marks backwards using a work list and the
predecessor sets `pred[a][r] = { p : δ(p, a) = r }`, so each pair is processed
once: `O(|Q|² · |Σ|)` time and `O(|Q|²)` space. The unmarked pairs come out in
lexicographical order directly from the double loop over `p < q`. Collapsing the
equivalent states (each identified by the smallest state in its class) yields
the minimal automaton, which recognizes the same language (Lecture 13).

The code is organized as: `DFAReader` (input parsing and validation), `DFA` (the
automaton and its invariants), `Minimizer` (the marking algorithm and the
collapse), and two helpers, `TablePrinter` and `DotExporter`, for the extra
output.