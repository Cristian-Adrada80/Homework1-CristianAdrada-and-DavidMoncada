#include "TablePrinter.h"

#include <iomanip>
#include <string>
#include <vector>

namespace {

// Ancho de una columna: el mayor entre el encabezado y todos los valores.
std::size_t columnWidth(const std::string &header, const std::vector<std::string> &cells) {
    std::size_t width = header.size();
    for (const std::string &cell : cells)
        if (cell.size() > width) width = cell.size();
    return width;
}

} 

void TablePrinter::print(std::ostream &out, const DFA &dfa, const std::string &title) {
    const std::size_t n = dfa.stateCount();
    const std::size_t k = dfa.alphabetSize();
    const auto &alphabet = dfa.alphabet();

    // Textos de cada celda por columna, para calcular anchos y alinear.
    // Columna 0: marca (-> / <-). Columna 1: numero de estado. Resto: destinos.
    std::vector<std::string> markCells(n);
    std::vector<std::string> stateCells(n);
    std::vector<std::vector<std::string>> targetCells(k, std::vector<std::string>(n));

    for (std::size_t q = 0; q < n; ++q) {
        std::string mark;
        if (dfa.startState() == static_cast<int>(q)) mark += "->";
        if (dfa.isFinal(static_cast<int>(q))) mark += (mark.empty() ? "<-" : " <-");
        markCells[q] = mark;
        stateCells[q] = std::to_string(q);
        for (std::size_t a = 0; a < k; ++a)
            targetCells[a][q] = std::to_string(dfa.transition(static_cast<int>(q), a));
    }

    const std::size_t markW = columnWidth("", markCells);
    const std::size_t stateW = columnWidth("", stateCells);
    std::vector<std::size_t> targetW(k);
    for (std::size_t a = 0; a < k; ++a)
        targetW[a] = columnWidth(std::string(1, alphabet[a]), targetCells[a]);

    out << title << "\n";

    // Encabezado: espacios para marca y estado, luego los simbolos.
    out << std::string(markW, ' ') << "  " << std::string(stateW, ' ');
    for (std::size_t a = 0; a < k; ++a)
        out << "  " << std::setw(static_cast<int>(targetW[a])) << alphabet[a];
    out << "\n";

    // Una fila por estado.
    for (std::size_t q = 0; q < n; ++q) {
        out << std::left << std::setw(static_cast<int>(markW)) << markCells[q] << std::right
            << "  " << std::setw(static_cast<int>(stateW)) << stateCells[q];
        for (std::size_t a = 0; a < k; ++a)
            out << "  " << std::setw(static_cast<int>(targetW[a])) << targetCells[a][q];
        out << "\n";
    }
    out << "\n";
}
