#include "DFAReader.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

#include "InputError.h"

namespace {

bool isBlank(const std::string &line) {
    return line.find_first_not_of(" \t\f\v") == std::string::npos;
}

std::string quoted(const std::string &token) { return "'" + token + "'"; }

}

bool DFAReader::readLine(std::string &line) {
    if (!std::getline(in_, line)) return false;
    ++lineNumber_;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
    return true;
}

std::string DFAReader::nextLine(const std::string &expected) {
    std::string line;
    if (!readLine(line))
        throw InputError(lineNumber_, "unexpected end of input, expected " + expected);
    return line;
}

std::string DFAReader::nextNonBlankLine(const std::string &expected) {
    std::string line;
    while (readLine(line))
        if (!isBlank(line)) return line;
    throw InputError(lineNumber_, "unexpected end of input, expected " + expected);
}

std::vector<std::string> DFAReader::tokenize(const std::string &line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) tokens.push_back(token);
    return tokens;
}

long DFAReader::parseInteger(const std::string &token, const std::string &what) const {
    if (token.empty() || token.find_first_not_of("0123456789") != std::string::npos)
        throw InputError(lineNumber_, what + " must be a non-negative integer, found " + quoted(token));
    errno = 0;
    char *end = nullptr;
    const long value = std::strtol(token.c_str(), &end, 10);
    if (errno == ERANGE || end == token.c_str() || *end != '\0')
        throw InputError(lineNumber_, what + " is not a representable integer: " + quoted(token));
    return value;
}

int DFAReader::parseState(const std::string &token, std::size_t stateCount,
                          const std::string &what) const {
    const long value = parseInteger(token, what);
    if (value >= static_cast<long>(stateCount))
        throw InputError(lineNumber_, what + " " + quoted(token) + " is not a state of the automaton (valid range: 0.." +
                                          std::to_string(stateCount - 1) + ")");
    return static_cast<int>(value);
}

std::size_t DFAReader::readCaseCount() {
    const std::string line = nextNonBlankLine("the number of cases");
    const auto tokens = tokenize(line);
    if (tokens.size() != 1)
        throw InputError(lineNumber_, "the first line must contain exactly one number (the number of cases), found " +
                                          std::to_string(tokens.size()) + " values");
    const long cases = parseInteger(tokens[0], "the number of cases");
    if (cases <= 0) throw InputError(lineNumber_, "the number of cases must be greater than zero");
    return static_cast<std::size_t>(cases);
}

DFA DFAReader::readCase(std::size_t caseNumber) {
    const std::string prefix = "case " + std::to_string(caseNumber) + ": ";

    // ---- numero de estados --------------------------------------------------
    std::size_t stateCount = 0;
    {
        const std::string line = nextNonBlankLine(prefix + "the number of states");
        const auto tokens = tokenize(line);
        if (tokens.size() != 1)
            throw InputError(lineNumber_, prefix + "expected exactly one number (the number of states)");
        const long n = parseInteger(tokens[0], prefix + "the number of states");
        if (n <= 0) throw InputError(lineNumber_, prefix + "the number of states must be greater than zero");
        stateCount = static_cast<std::size_t>(n);
    }

    // ---- alfabeto -----------------------------------------------------------
    std::vector<char> alphabet;
    {
        const std::string line = nextNonBlankLine(prefix + "the alphabet");
        const auto tokens = tokenize(line);
        if (tokens.empty()) throw InputError(lineNumber_, prefix + "the alphabet cannot be empty");
        for (const std::string &token : tokens) {
            if (token.size() != 1)
                throw InputError(lineNumber_, prefix + "every alphabet symbol must be a single character, found " + quoted(token));
            const char symbol = token[0];
            if (symbol < 'a' || symbol > 'z')
                throw InputError(lineNumber_, prefix + "alphabet symbols must be lowercase latin letters (a..z), found " + quoted(token));
            for (const char previous : alphabet)
                if (previous == symbol)
                    throw InputError(lineNumber_, prefix + "the symbol " + quoted(token) + " appears twice in the alphabet");
            alphabet.push_back(symbol);
        }
    }
    const std::size_t alphabetSize = alphabet.size();

    // ---- estados finales ----------------------------------------------------
    std::vector<bool> finalStates(stateCount, false);
    {
        const std::string line = nextLine(prefix + "the final states");
        for (const std::string &token : tokenize(line)) {
            const int state = parseState(token, stateCount, prefix + "the final state");
            if (finalStates[static_cast<std::size_t>(state)])
                throw InputError(lineNumber_, prefix + "the final state " + quoted(token) + " is repeated");
            finalStates[static_cast<std::size_t>(state)] = true;
        }
    }

    // ---- tabla de transiciones ----------------------------------------------
    std::vector<std::vector<int>> delta(stateCount, std::vector<int>(alphabetSize, 0));
    std::vector<bool> rowSeen(stateCount, false);
    for (std::size_t row = 0; row < stateCount; ++row) {
        const std::string line =
            nextNonBlankLine(prefix + "row " + std::to_string(row + 1) + " of the transition table");
        const auto tokens = tokenize(line);
        if (tokens.size() != alphabetSize + 1)
            throw InputError(lineNumber_, prefix + "every row of the table must have 1 + |Sigma| = " +
                                              std::to_string(alphabetSize + 1) + " values (the state and its transitions), found " +
                                              std::to_string(tokens.size()));

        const int state = parseState(tokens[0], stateCount, prefix + "the state of the row");
        if (rowSeen[static_cast<std::size_t>(state)])
            throw InputError(lineNumber_, prefix + "state " + quoted(tokens[0]) + " has more than one row in the table");
        rowSeen[static_cast<std::size_t>(state)] = true;

        for (std::size_t a = 0; a < alphabetSize; ++a) {
            const std::string what =
                prefix + "the transition delta(" + std::to_string(state) + ", " + alphabet[a] + ")";
            delta[static_cast<std::size_t>(state)][a] = parseState(tokens[a + 1], stateCount, what);
        }
    }
    for (std::size_t q = 0; q < stateCount; ++q)
        if (!rowSeen[q])
            throw InputError(lineNumber_, prefix + "state " + std::to_string(q) + " has no row in the transition table");

    try {
        return DFA(stateCount, std::move(alphabet), std::move(delta), std::move(finalStates), 0);
    } catch (const std::exception &error) {
        throw InputError(lineNumber_, prefix + error.what());
    }
}

bool DFAReader::hasTrailingContent() {
    std::string line;
    while (readLine(line))
        if (!isBlank(line)) return true;
    return false;
}
