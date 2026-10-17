#ifndef DOT_EXPORTER_H
#define DOT_EXPORTER_H

#include <string>

#include "DFA.h"

/** Funcion opcional: dibuja un automata en formato Graphviz DOT. */
class DotExporter {
public:
    static std::string toDot(const DFA &dfa, const std::string &name);

    /** Escribe el texto DOT en un archivo. Devuelve false si no se puede abrir. */
    static bool writeToFile(const DFA &dfa, const std::string &name, const std::string &path);
};

#endif  // DOT_EXPORTER_H
