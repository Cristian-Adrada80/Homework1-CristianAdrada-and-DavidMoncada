#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "DFA.h"
#include "DFAReader.h"
#include "DotExporter.h"
#include "InputError.h"
#include "Minimizer.h"
#include "TablePrinter.h"

namespace {

constexpr int kExitOk = 0;
constexpr int kExitInputError = 1;
constexpr int kExitUsageError = 2;

struct Options {
    std::string inputFile;
    std::string dotPrefix;
    bool showHelp = false;
};

void printUsage(const char *program) {
    std::cout << "Usage: " << program << " [options] [input-file]\n\n"
              << "  Reads the automata from <input-file>, or from standard input when no\n"
              << "  file is given.\n\n"
              << "Options:\n"
              << "  -h, --help          Show this message.\n"
              << "  -d, --dot <prefix>  Write the Graphviz source (.dot) of each case as\n"
              << "                      <prefix>_case<i>_original.dot and\n"
              << "                      <prefix>_case<i>_minimized.dot\n"
              << "                      (viewable in Graphviz, local or online).\n\n"
              << "  On every run, the transition table of the original and the minimized\n"
              << "  automaton is printed for each case, followed by their Graphviz source.\n";
}

/** Analiza la linea de comandos. */
Options parseOptions(int argc, char *argv[]) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "-h" || argument == "--help") {
            options.showHelp = true;
        } else if (argument == "-d" || argument == "--dot") {
            if (i + 1 >= argc) throw std::invalid_argument("option " + argument + " needs a prefix");
            options.dotPrefix = argv[++i];
        } else if (!argument.empty() && argument[0] == '-') {
            throw std::invalid_argument("unknown option: " + argument);
        } else if (options.inputFile.empty()) {
            options.inputFile = argument;
        } else {
            throw std::invalid_argument("only one input file can be given");
        }
    }
    return options;
}

void exportDiagrams(const Options &options, std::size_t caseNumber, const DFA &original,
                    const DFA &minimized) {
    const std::string base = options.dotPrefix + "_case" + std::to_string(caseNumber);
    if (!DotExporter::writeToFile(original, "original", base + "_original.dot"))
        std::cerr << "warning: could not write " << base << "_original.dot\n";
    if (!DotExporter::writeToFile(minimized, "minimized", base + "_minimized.dot"))
        std::cerr << "warning: could not write " << base << "_minimized.dot\n";
}

}  // namespace

int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);

    Options options;
    try {
        options = parseOptions(argc, argv);
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << "\n\n";
        printUsage(argv[0]);
        return kExitUsageError;
    }

    if (options.showHelp) {
        printUsage(argv[0]);
        return kExitOk;
    }

    std::ifstream file;
    if (!options.inputFile.empty()) {
        file.open(options.inputFile);
        if (!file) {
            std::cerr << "error: cannot open the input file '" << options.inputFile << "'\n";
            return kExitUsageError;
        }
    }
    std::istream &input = options.inputFile.empty() ? std::cin : file;

    try {
        DFAReader reader(input);
        const std::size_t cases = reader.readCaseCount();

        // Primero se leen y procesan todos los casos; los resultados se guardan
        // para imprimirlos juntos al final.
        std::vector<std::string> results;
        results.reserve(cases);

        // La informacion adicional (tablas y codigo Graphviz) se acumula aqui y
        // se imprime después de todos los pares equivalentes, para que la salida
        // principal aparezca primero.
        std::ostringstream extra;

        for (std::size_t c = 1; c <= cases; ++c) {
            const DFA automaton = reader.readCase(c);

            const std::vector<int> unreachable = automaton.inaccessibleStates();
            if (!unreachable.empty()) {
                // El enunciado garantiza que no hay estados inaccesibles. Si
                // aparecen, solo se emite la advertencia y se omite todo lo
                // demas de este caso (pares y bloque adicional).
                std::cerr << "warning: case " << c << " has inaccessible states (";
                for (std::size_t i = 0; i < unreachable.size(); ++i)
                    std::cerr << (i ? ", " : "") << unreachable[i];
                std::cerr << "); the assignment assumes there are none\n";
                continue;
            }

            const Minimizer minimizer(automaton);
            const std::string pairs = minimizer.formatPairs();
            // Si el caso no tiene pares equivalentes, en vez de una linea vacia
            // se muestra un aviso claro indicando el numero de caso.
            results.push_back(pairs.empty() ? "(Case " + std::to_string(c) + " has no equivalent pairs)"
                                            : pairs);
            // Si el caso no tiene pares equivalentes, no se imprime el bloque
            // adicional (tablas y codigo Graphviz); basta con el aviso ya
            // guardado en 'results'.
            if (pairs.empty()) continue;

            const DFA minimized = minimizer.collapse();

            // Informacion adicional (tablas y codigo Graphviz), etiquetada por
            // caso. Se acumula en 'extra' y se imprime al final, despues de
            // todos los pares equivalentes.
            extra << "========== Case " << c << " ==========\n";

            TablePrinter::print(extra, automaton, "Original automaton:");
            TablePrinter::print(extra, minimized, "Minimized automaton ("
                                    + std::to_string(automaton.stateCount()) + " -> "
                                    + std::to_string(minimized.stateCount()) + " states):");

            // En vez de repetir los pares equivalentes (que ya salen en stdout),
            // se ofrece el codigo Graphviz para visualizar los automatas. Se
            // puede pegar en https://dreampuf.github.io/GraphvizOnline u otro
            // visor de Graphviz.
            extra << "To see the automata, paste the following code blocks into Graphviz Online:\n\n";
            extra << "  # Original automaton (case " << c << ")\n";
            extra << DotExporter::toDot(automaton, "original") << "\n";
            extra << "  # Minimized automaton (case " << c << ")\n";
            extra << DotExporter::toDot(minimized, "minimized") << "\n";

            if (!options.dotPrefix.empty())
                exportDiagrams(options, c, automaton, minimized);
        }

        if (!options.inputFile.empty() && reader.hasTrailingContent())
            std::cerr << "warning: there is content after the last case; it was ignored\n";

        // 1) Salida principal: los pares equivalentes, una linea por caso.
        for (const std::string &line : results)
            std::cout << line << '\n';

        // 2) Informacion adicional (tablas y codigo Graphviz), a continuacion,
        // por la misma salida estandar para garantizar que aparezca despues de
        // los pares en cualquier terminal. Se separa con una linea en blanco.
        std::cout << '\n' << extra.str();
    } catch (const InputError &error) {
        std::cerr << "error: " << error.what() << "\n";
        return kExitInputError;
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << "\n";
        return kExitInputError;
    }

    std::cout.flush();
    return kExitOk;
}