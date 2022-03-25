#include "guu.h"

namespace guu {
template<typename T>
void error(T message)
{
    throw std::runtime_error(message);
}

template<typename T>
void assert(bool predicate, T message)
{
    if (!predicate) {
        error(message);
    }
}

ValueType Value::type()
{
    return static_cast<ValueType>(value.index());
}

std::optional<Id> Program::getVariableId(const std::string &name)
{
    std::optional<Id> optional;
    try {
        optional = variableIds.at(name);
    } catch (const std::out_of_range &e) {
    }
    return optional;
}

std::optional<Id> Program::getProcedureId(const std::string &name)
{
    std::optional<Id> optional;
    try {
        optional = procedureIds.at(name);
    } catch (const std::out_of_range &e) {
    }
    return optional;
}

std::optional<Number> Program::toNumber(const std::string &string)
{
    std::optional<Number> number;
    if (string == "0") {
        number = 0;
    } else {
        number = std::atoll(string.data());
        if (number.value() == 0) {
            number.reset();
        }
    }
    return number;
}

void Program::strip(std::string &string)
{
    if (string.empty()) {
        return;
    }
    auto isEmptyChar = [](char c) { return c == ' ' || c == '\t' || c == '\n'; };
    size_t begin;
    for (begin = 0; begin < string.size() && isEmptyChar(string[begin]); ++begin)
        ;
    size_t end;
    for (end = string.size() - 1; end != begin && isEmptyChar(string[end]); --end)
        ;
    string = string.substr(begin, end);
}

Program::Program(std::istream &in)
{
    std::string line;
    uint line_number = 1;
    // Error reporting
    auto getLineMessage = [&line_number]() {
        return "on line " + std::to_string(line_number) + ": ";
    };
    auto getErrorMessage = [getLineMessage](auto message) { return getLineMessage() + message; };
    auto throwSyntaxError = [getErrorMessage](auto message) { error(getErrorMessage(message)); };
    auto assertSyntaxError = [getErrorMessage](bool predicate, auto message) {
        assert(predicate, getErrorMessage(message));
    };

#define ASSERT_OPTIONAL(error, ...)                                                                \
    [&]() {                                                                                        \
        auto $result = __VA_ARGS__;                                                                \
        assertSyntaxError($result.has_value(), error);                                             \
        return $result.value();                                                                    \
    }()

    // Helpers
    std::string procedureName = "";
    auto isProcedureSet = [&procedureName]() { return !procedureName.empty(); };

    auto makeValue = [this, assertSyntaxError](std::string stringValue) {
        guu::Value value;
        if (stringValue.front() == '"' && stringValue.back() == '"') {
            value.value = stringValue.substr(1, stringValue.size() - 1);
        } else {
            std::optional<Number> number = toNumber(stringValue);
            if (number.has_value()) {
                value.value = number.value();
            } else {
                value.value = ASSERT_OPTIONAL("set: value of unknown type: `" + stringValue + "'",
                                              getVariableId(stringValue));
            }
        }
        return value;
    };

    for (; std::getline(in, line); ++line_number) {
        std::istringstream is(line);
        std::string instruction;
        if (is >> instruction) {
            if (instruction == "sub") {
                assertSyntaxError(static_cast<bool>(is >> procedureName),
                                  "instruction `sub' requires an argument: procedure name.");
                procedureNames.push_back(procedureName);
                Id procedureId = procedures.size();
                procedures.emplace_back();
                if (procedureName == entryProcedureName) {
                    entryPoint = procedureId;
                }
                procedureIds[procedureName] = procedureId;
            } else if (instruction == "set") {
                assertSyntaxError(isProcedureSet(),
                                  "instruction `set' doesn't belong to any procedure");

                std::string variableName, variableValue;
                is >> variableName;
                std::getline(is, variableValue);
                strip(variableValue);
                assertSyntaxError(
                        !variableValue.empty(),
                        "instruction `set' requires two arguments: variable name and value.");
                Id variableId = variableNames.size();
                variableNames.push_back(variableName);
                variableIds[variableName] = variableId;
                procedures.back().instructions.emplace_back(InstructionType::Set, variableId,
                                                            makeValue(variableValue));
            } else if (instruction == "print") {
                assertSyntaxError(isProcedureSet(),
                                  "instruction `print' doesn't belong to any procedure");

                std::string variableName;
                assertSyntaxError(static_cast<bool>(is >> variableName),
                                  "instruction `print' requires an argument: variable name.");
                procedures.back().instructions.emplace_back(
                        InstructionType::Print,
                        ASSERT_OPTIONAL("undefined variable: `" + variableName + "'",
                                        getVariableId(variableName)));
            } else if (instruction == "call") {
                assertSyntaxError(isProcedureSet(),
                                  "instruction `call' doesn't belong to any procedure");

                std::string procedureName;
                assertSyntaxError(static_cast<bool>(is >> procedureName),
                                  "instruction `call' requires an argument: procedure name.");
                is >> procedureName;
                procedures.back().instructions.emplace_back(
                        InstructionType::Call,
                        ASSERT_OPTIONAL("undefined procedure: `" + procedureName + "'",
                                        getProcedureId(procedureName)));
            } else {
                throwSyntaxError("unknown instruction: `" + instruction + "'");
            }
        }
    }
    entryPoint = ASSERT_OPTIONAL("No entry procedure (procedure with name `" + entryProcedureName
                                         + "') found'",
                                 getProcedureId(entryProcedureName));
    // TODO: when nodebug mode: remove symbols
    for (auto procedure : procedures) {
        procedure.instructions.shrink_to_fit();
    }
    procedures.shrink_to_fit();
    procedureNames.shrink_to_fit();
    variableNames.shrink_to_fit();
#undef ASSERT_OPTIONAL
}
}
