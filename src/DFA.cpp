#include "DFA.h"

#include <algorithm>
#include <queue>
#include <stdexcept>

DFA::DFA(std::size_t stateCount,
         std::vector<char> alphabet,
         std::vector<std::vector<int>> delta,
         std::vector<bool> finalStates,
         int startState)
    : stateCount_(stateCount),
      alphabet_(std::move(alphabet)),
      delta_(std::move(delta)),
      final_(std::move(finalStates)),
      start_(startState) {
    validate();
}

void DFA::validate() const {
    if (stateCount_ == 0) throw std::invalid_argument("a DFA needs at least one state");
    if (alphabet_.empty()) throw std::invalid_argument("a DFA needs a non-empty alphabet");

    for (std::size_t i = 0; i < alphabet_.size(); ++i) {
        const char symbol = alphabet_[i];
        if (symbol < 'a' || symbol > 'z')
            throw std::invalid_argument("alphabet symbols must be lowercase latin letters");
        for (std::size_t j = i + 1; j < alphabet_.size(); ++j)
            if (alphabet_[j] == symbol)
                throw std::invalid_argument(std::string("repeated alphabet symbol '") + symbol + "'");
    }

    if (delta_.size() != stateCount_)
        throw std::invalid_argument("the transition table must have one row per state");
    if (final_.size() != stateCount_)
        throw std::invalid_argument("the set of final states is not consistent with |Q|");
    if (start_ < 0 || static_cast<std::size_t>(start_) >= stateCount_)
        throw std::invalid_argument("the start state is not a state of the automaton");

    for (std::size_t q = 0; q < stateCount_; ++q) {
        if (delta_[q].size() != alphabet_.size())
            throw std::invalid_argument("every row must have exactly |Sigma| transitions");
        for (const int target : delta_[q])
            if (target < 0 || static_cast<std::size_t>(target) >= stateCount_)
                throw std::invalid_argument("a transition points to a state that does not exist");
    }
}

bool DFA::isFinal(int q) const {
    if (q < 0 || static_cast<std::size_t>(q) >= stateCount_)
        throw std::out_of_range("isFinal: state out of range");
    return final_[static_cast<std::size_t>(q)];
}

int DFA::transition(int q, std::size_t symbolIndex) const {
    if (q < 0 || static_cast<std::size_t>(q) >= stateCount_)
        throw std::out_of_range("transition: state out of range");
    if (symbolIndex >= alphabet_.size())
        throw std::out_of_range("transition: symbol out of range");
    return delta_[static_cast<std::size_t>(q)][symbolIndex];
}

std::size_t DFA::indexOfSymbol(char symbol) const noexcept {
    const auto it = std::find(alphabet_.begin(), alphabet_.end(), symbol);
    if (it == alphabet_.end()) return npos;
    return static_cast<std::size_t>(it - alphabet_.begin());
}

std::vector<int> DFA::finalStates() const {
    std::vector<int> states;
    for (std::size_t q = 0; q < stateCount_; ++q)
        if (final_[q]) states.push_back(static_cast<int>(q));
    return states;
}

std::vector<int> DFA::inaccessibleStates() const {
    std::vector<bool> seen(stateCount_, false);
    std::queue<int> pending;
    seen[static_cast<std::size_t>(start_)] = true;
    pending.push(start_);

    while (!pending.empty()) {
        const int q = pending.front();
        pending.pop();
        for (std::size_t a = 0; a < alphabet_.size(); ++a) {
            const int target = delta_[static_cast<std::size_t>(q)][a];
            if (!seen[static_cast<std::size_t>(target)]) {
                seen[static_cast<std::size_t>(target)] = true;
                pending.push(target);
            }
        }
    }

    std::vector<int> unreachable;
    for (std::size_t q = 0; q < stateCount_; ++q)
        if (!seen[q]) unreachable.push_back(static_cast<int>(q));
    return unreachable;
}
