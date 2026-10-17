#ifndef DFA_H
#define DFA_H

#include <cstddef>
#include <string>
#include <vector>


class DFA {
public:
    DFA(std::size_t stateCount,
        std::vector<char> alphabet,
        std::vector<std::vector<int>> delta,
        std::vector<bool> finalStates,
        int startState = 0);

    std::size_t stateCount() const noexcept { return stateCount_; }
    std::size_t alphabetSize() const noexcept { return alphabet_.size(); }
    const std::vector<char> &alphabet() const noexcept { return alphabet_; }
    int startState() const noexcept { return start_; }

    /** Verdadero si q pertenece a F. Lanza std::out_of_range si q no es un estado. */
    bool isFinal(int q) const;

    /** delta(q, a_i) donde a_i es el simbolo en la posicion i del alfabeto. */
    int transition(int q, std::size_t symbolIndex) const;

    /** Indice de la letra dada dentro del alfabeto, o npos si no esta. */
    std::size_t indexOfSymbol(char symbol) const noexcept;

    std::vector<int> finalStates() const;

    /**
     * Estados que no se pueden alcanzar desde el estado inicial con ninguna
     * cadena. */
    std::vector<int> inaccessibleStates() const;

    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

private:
    void validate() const;

    std::size_t stateCount_;
    std::vector<char> alphabet_;
    std::vector<std::vector<int>> delta_;
    std::vector<bool> final_;
    int start_;
};

#endif  // DFA_H
