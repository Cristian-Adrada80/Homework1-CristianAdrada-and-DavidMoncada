#ifndef DFA_INPUT_ERROR_H
#define DFA_INPUT_ERROR_H

#include <cstddef>
#include <stdexcept>
#include <string>

/** Error que se lanza al leer o validar la entrada de un caso. */
class InputError : public std::runtime_error {
public:
    InputError(std::size_t line, const std::string &message)
        : std::runtime_error(build(line, message)), line_(line) {}

    std::size_t line() const noexcept { return line_; }

private:
    static std::string build(std::size_t line, const std::string &message) {
        return "input line " + std::to_string(line) + ": " + message;
    }

    std::size_t line_;
};

#endif  // DFA_INPUT_ERROR_H
