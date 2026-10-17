#ifndef TABLE_PRINTER_H
#define TABLE_PRINTER_H

#include <ostream>
#include <string>

#include "DFA.h"

/**
 * Funcion opcional: imprime la tabla de transiciones de un automata en formato
 * de texto (ASCII).
 */
class TablePrinter {
public:
    /** Escribe la tabla del automata precedida por un titulo. */
    static void print(std::ostream &out, const DFA &dfa, const std::string &title);
};

#endif  // TABLE_PRINTER_H
