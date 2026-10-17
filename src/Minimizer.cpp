#include "Minimizer.h"

#include <queue>
#include <stdexcept>

Minimizer::Minimizer(const DFA &dfa)
    : dfa_(dfa),
      n_(dfa.stateCount()),
      distinguishable_(dfa.stateCount(), std::vector<char>(dfa.stateCount(), 0)),
      representative_(dfa.stateCount(), 0) {
    computeDistinguishablePairs();
    collectEquivalentPairs();
}

void Minimizer::computeDistinguishablePairs() {
    const std::size_t k = dfa_.alphabetSize();
    std::queue<std::pair<int, int>> pending;

    // Caso base: un estado final y uno no final se distinguen con la
    // cadena vacia.
    for (std::size_t p = 0; p < n_; ++p)
        for (std::size_t q = p + 1; q < n_; ++q)
            if (dfa_.isFinal(static_cast<int>(p)) != dfa_.isFinal(static_cast<int>(q))) {
                distinguishable_[p][q] = distinguishable_[q][p] = 1;
                pending.push({static_cast<int>(p), static_cast<int>(q)});
            }

    std::vector<std::vector<std::vector<int>>> predecessors(k, std::vector<std::vector<int>>(n_));
    for (std::size_t p = 0; p < n_; ++p)
        for (std::size_t a = 0; a < k; ++a) {
            const int target = dfa_.transition(static_cast<int>(p), a);
            predecessors[a][static_cast<std::size_t>(target)].push_back(static_cast<int>(p));
        }

    // Paso inductivo, propagado hacia atras.
    while (!pending.empty()) {
        const int r1 = pending.front().first;
        const int r2 = pending.front().second;
        pending.pop();
        for (std::size_t a = 0; a < k; ++a)
            for (const int p : predecessors[a][static_cast<std::size_t>(r1)])
                for (const int q : predecessors[a][static_cast<std::size_t>(r2)]) {
                    auto &mark = distinguishable_[static_cast<std::size_t>(p)][static_cast<std::size_t>(q)];
                    if (!mark) {
                        mark = 1;
                        distinguishable_[static_cast<std::size_t>(q)][static_cast<std::size_t>(p)] = 1;
                        pending.push({p < q ? p : q, p < q ? q : p});
                    }
                }
    }
}

void Minimizer::collectEquivalentPairs() {
    for (std::size_t p = 0; p < n_; ++p) {
        representative_[p] = static_cast<int>(p);
        for (std::size_t q = 0; q < p; ++q)
            if (!distinguishable_[p][q]) {
                representative_[p] = static_cast<int>(q);
                break;
            }
        for (std::size_t q = p + 1; q < n_; ++q)
            if (!distinguishable_[p][q]) pairs_.push_back({static_cast<int>(p), static_cast<int>(q)});
    }
}

bool Minimizer::areDistinguishable(int p, int q) const {
    if (p < 0 || q < 0 || static_cast<std::size_t>(p) >= n_ || static_cast<std::size_t>(q) >= n_)
        throw std::out_of_range("areDistinguishable: state out of range");
    return distinguishable_[static_cast<std::size_t>(p)][static_cast<std::size_t>(q)] != 0;
}

std::string Minimizer::formatPairs() const {
    std::string text;
    for (const auto &pair : pairs_) {
        if (!text.empty()) text += ' ';
        text += "(" + std::to_string(pair.first) + ", " + std::to_string(pair.second) + ")";
    }
    return text;
}

DFA Minimizer::collapse() const {
    const std::size_t k = dfa_.alphabetSize();

    // Cada clase se identifica por su representante; las clases se numeran en
    // orden creciente de representante, de modo que la clase del estado 0
    // recibe el numero 0 y sigue siendo el estado inicial.
    std::vector<int> classOf(n_, -1);
    std::vector<int> classRepresentative;
    for (std::size_t q = 0; q < n_; ++q) {
        const std::size_t rep = static_cast<std::size_t>(representative_[q]);
        if (classOf[rep] == -1) {
            classOf[rep] = static_cast<int>(classRepresentative.size());
            classRepresentative.push_back(static_cast<int>(rep));
        }
        classOf[q] = classOf[rep];
    }

    const std::size_t classCount = classRepresentative.size();
    std::vector<std::vector<int>> delta(classCount, std::vector<int>(k, 0));
    std::vector<bool> finalStates(classCount, false);
    for (std::size_t c = 0; c < classCount; ++c) {
        const int rep = classRepresentative[c];
        finalStates[c] = dfa_.isFinal(rep);
        for (std::size_t a = 0; a < k; ++a)
            delta[c][a] = classOf[static_cast<std::size_t>(dfa_.transition(rep, a))];
    }

    return DFA(classCount, dfa_.alphabet(), std::move(delta), std::move(finalStates),
               classOf[static_cast<std::size_t>(dfa_.startState())]);
}
