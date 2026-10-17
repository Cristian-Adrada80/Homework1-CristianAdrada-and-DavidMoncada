#include "DotExporter.h"

#include <fstream>
#include <map>
#include <sstream>

std::string DotExporter::toDot(const DFA &dfa, const std::string &name) {
    std::ostringstream out;
    out << "digraph " << name << " {\n";
    out << "  rankdir=LR;\n";
    out << "  __start [shape=point];\n";

    for (std::size_t q = 0; q < dfa.stateCount(); ++q)
        out << "  " << q << " [shape=" << (dfa.isFinal(static_cast<int>(q)) ? "doublecircle" : "circle")
            << "];\n";

    out << "  __start -> " << dfa.startState() << ";\n";

    // Fusiona los simbolos que comparten el mismo origen y destino.
    std::map<std::pair<int, int>, std::string> labels;
    for (std::size_t q = 0; q < dfa.stateCount(); ++q)
        for (std::size_t a = 0; a < dfa.alphabetSize(); ++a) {
            const int target = dfa.transition(static_cast<int>(q), a);
            std::string &label = labels[{static_cast<int>(q), target}];
            if (!label.empty()) label += ", ";
            label += dfa.alphabet()[a];
        }

    for (const auto &edge : labels)
        out << "  " << edge.first.first << " -> " << edge.first.second << " [label=\"" << edge.second
            << "\"];\n";

    out << "}\n";
    return out.str();
}

bool DotExporter::writeToFile(const DFA &dfa, const std::string &name, const std::string &path) {
    std::ofstream file(path);
    if (!file) return false;
    file << toDot(dfa, name);
    return static_cast<bool>(file);
}
