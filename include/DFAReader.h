#ifndef DFA_READER_H
#define DFA_READER_H

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

#include "DFA.h"

/**
 * Lee el formato de entrada descrito en la tarea y lo valida.
 *
 * Formato esperado:
 *   linea 1       : c > 0, numero de casos
 *   por cada caso:
 *     linea       : n > 0, numero de estados
 *     linea       : el alfabeto, simbolos separados por espacios
 *     linea       : los estados finales separados por espacios (puede ir vacia)
 *     n lineas    : <estado> <delta(estado, a_1)> ... <delta(estado, a_k)>
 */
class DFAReader {
public:
    explicit DFAReader(std::istream &in) : in_(in) {}

    /** Lee la primera linea del archivo: el numero de casos. */
    std::size_t readCaseCount();

    /** Lee y valida un caso completo. */
    DFA readCase(std::size_t caseNumber);

    /** Contenido no vacio que queda tras el ultimo caso, si lo hay (para advertencias). */
    bool hasTrailingContent();

    std::size_t currentLine() const noexcept { return lineNumber_; }

private:
    bool readLine(std::string &line);
    std::string nextNonBlankLine(const std::string &expected);
    std::string nextLine(const std::string &expected);

    static std::vector<std::string> tokenize(const std::string &line);
    long parseInteger(const std::string &token, const std::string &what) const;
    int parseState(const std::string &token, std::size_t stateCount,
                   const std::string &what) const;

    std::istream &in_;
    std::size_t lineNumber_ = 0;
};

#endif  // DFA_READER_H
