#ifndef MINIMIZER_H
#define MINIMIZER_H

#include <string>
#include <utility>
#include <vector>

#include "DFA.h"

/** Algoritmo de minimizacion. */
class Minimizer {
public:
    explicit Minimizer(const DFA &dfa);

    /** Pares equivalentes (p, q) con p < q, en orden lexicografico. */
    const std::vector<std::pair<int, int>> &equivalentPairs() const noexcept { return pairs_; }

    /** Verdadero si p y q son NO equivalentes. */
    bool areDistinguishable(int p, int q) const;

    /** Representante (el menor estado equivalente) de cada estado. */
    const std::vector<int> &representatives() const noexcept { return representative_; }

    /** El automata equivalente obtenido al colapsar los estados equivalentes. */
    DFA collapse() const;

    /** Los pares con el formato que se exige: "(1, 2) (3, 4)". */
    std::string formatPairs() const;

private:
    void computeDistinguishablePairs();
    void collectEquivalentPairs();

    const DFA &dfa_;
    std::size_t n_;
    std::vector<std::vector<char>> distinguishable_;  // tabla simetrica
    std::vector<std::pair<int, int>> pairs_;
    std::vector<int> representative_;
};

#endif  // MINIMIZER_H
