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

Id Program::getVariableId(const std::string &name, const bool definition = false)
{
    Id id;
    try {
        id = variableIds.at(name);
        if (definition) {
            undefinedVariables.erase(name);
        }
    } catch (const std::out_of_range) {
        variableNames.push_back(name);
        id = variables.size();
        variables.emplace_back();
        variableIds[name] = id;
        if (!definition) {
            undefinedVariables.insert(name);
        }
    }
    return id;
}

Id Program::getProcedureId(const std::string &name, const bool definition = false)
{
    Id id;
    try {
        id = procedureIds.at(name);
        if (definition) {
            undefinedProcedures.erase(name);
        }
    } catch (const std::out_of_range) {
        procedureNames.push_back(name);
        id = procedures.size();
        procedures.emplace_back();
        procedureIds[name] = id;
        if (name == entryProcedureName) {
            entryPoint = id;
        }
        if (!definition) {
            undefinedProcedures.insert(name);
        }
    }
    return id;
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
                // TODO: check if stringValue can be variable name
                value.value = getVariableId(stringValue);
            }
        }
        return value;
    };

    size_t currentProcedureId;
    for (; std::getline(in, line); ++line_number) {
        std::istringstream is(line);
        std::string instruction;
        if (is >> instruction) {
            if (instruction == "sub") {
                assertSyntaxError(static_cast<bool>(is >> procedureName),
                                  "instruction `sub' requires an argument: procedure name.");

                currentProcedureId = getProcedureId(procedureName, true);
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
                Id variableId = getVariableId(variableName, true);
                procedures[currentProcedureId].instructions.emplace_back(
                        InstructionType::Set, variableId, makeValue(variableValue));
            } else if (instruction == "print") {
                assertSyntaxError(isProcedureSet(),
                                  "instruction `print' doesn't belong to any procedure");

                std::string variableName;
                assertSyntaxError(static_cast<bool>(is >> variableName),
                                  "instruction `print' requires an argument: variable name.");
                Id variableId = getVariableId(variableName);
                procedures[currentProcedureId].instructions.emplace_back(InstructionType::Print,
                                                                         variableId);
            } else if (instruction == "call") {
                assertSyntaxError(isProcedureSet(),
                                  "instruction `call' doesn't belong to any procedure");

                std::string procedureName;
                assertSyntaxError(static_cast<bool>(is >> procedureName),
                                  "instruction `call' requires an argument: procedure name.");
                Id procedureId = getProcedureId(procedureName);
                procedures[currentProcedureId].instructions.emplace_back(InstructionType::Call,
                                                                         procedureId);
            } else {
                throwSyntaxError("unknown instruction: `" + instruction + "'");
            }
        }
    }

    try {
        entryPoint = procedureIds.at(entryProcedureName);
    } catch (const std::out_of_range) {
        throwSyntaxError("No entry procedure (procedure with name `" + entryProcedureName
                         + "') defined");
    }

    if (!undefinedProcedures.empty()) {
        std::string undefinedProceduresError = "Following procedures are used but not defined: ";
        for (auto procedure : undefinedProcedures) {
            undefinedProceduresError += "`";
            undefinedProceduresError += procedure;
            undefinedProceduresError += "' ";
        }
        undefinedProceduresError.back() = '.';
        throwSyntaxError(undefinedProceduresError);
    }

    if (!undefinedVariables.empty()) {
        std::string undefinedVariablesError = "Following variables are used but not defined:";
        for (auto variable : undefinedVariables) {
            undefinedVariablesError += "`";
            undefinedVariablesError += variable;
            undefinedVariablesError += "' ";
        }
        undefinedVariablesError.back() = '.';
        throwSyntaxError(undefinedVariablesError);
    }

    // TODO: when nodebug mode: remove symbols
    for (auto procedure : procedures) {
        procedure.instructions.shrink_to_fit();
    }
    procedures.shrink_to_fit();
    variables.resize(variableNames.size());
    procedureNames.shrink_to_fit();
    variableNames.shrink_to_fit();
}

void Program::setValue(const Id keyId, Value &value)
{
    switch (value.type()) {
    case ValueType::Empty:
        break;
    case ValueType::Number:
    case ValueType::String:
        variables[keyId] = value;
        break;
    case ValueType::Variable:
        const Id valueId = std::get<Id>(value.value);
        if (variables[valueId].type() == ValueType::Empty) {
            throw std::runtime_error("instruction `set': variable `" + variableNames[valueId]
                                     + "' undefined");
        } else {
            variables[keyId] = variables[valueId];
        }
        break;
    }
}

void Program::printValue(const Id valueId, std::ostream &out)
{
#define valueToPrint variables[valueId]
    switch (valueToPrint.type()) {
    case ValueType::Empty:
        throw std::runtime_error("instruction `print': variable `" + variableNames[valueId]
                                 + "' undefined");
        break;
    case ValueType::Number:
        out << std::get<Number>(valueToPrint.value) << '\n';
        break;
    case ValueType::String:
        out << std::get<std::string>(valueToPrint.value) << '\n';
        break;
    case ValueType::Variable:
        printValue(std::get<Id>(valueToPrint.value), out);
        break;
    }
#undef valueToPrint
}

std::string Program::getDebugCommand(std::ostream &out, std::istream &in)
{
    std::string command;
    out << "> ";
    in >> command;
    return command;
}

void Program::run(std::ostream &out, std::ostream &err, std::istream &in, const bool debug = false,
                  const bool log = false)
{
#define LOG(...)                                                                                   \
    if (log)                                                                                       \
    err << __VA_ARGS__ << '\n'

    LOG("run");
    stack<ProcedureFrame, std::vector<ProcedureFrame>> procedureStack;
    procedureStack.emplace(entryPoint);
    for (;;) {
        ProcedureFrame *currentProcedure = &procedureStack.top();
        while (procedures[currentProcedure->id].instructions.size()
               == currentProcedure->instruction) {
            procedureStack.pop();
            LOG("return");
            if (procedureStack.empty()) {
                goto end;
            } else {
                currentProcedure = &procedureStack.top();
                ++currentProcedure->instruction;
            }
        }
#define currentInstruction                                                                         \
    procedures[currentProcedure->id].instructions[currentProcedure->instruction]
        LOG("instruction " << currentProcedure->instruction + 1 << "/"
                           << procedures[currentProcedure->id].instructions.size());
        switch (currentInstruction.type) {
        case InstructionType::Set:
            LOG("set");
            setValue(currentInstruction.id, const_cast<Value &>(currentInstruction.arg));
            break;
        case InstructionType::Call:
            LOG("call");
            if (debug) {
            getCommand:
                std::string command = getDebugCommand(err, in);
                if (command == "i") {
                    // pass
                } else if (command == "o") {
                    break;
                } else if (command == "trace") {
                    for (Id i = 0; i < procedureStack.container().size(); ++i) {
                        err << i << ") " << procedureNames[procedureStack.container()[i].id]
                            << '\n';
                    }
                    goto getCommand;
                } else if (command == "var") {
                    for (Id i = 0; i < variables.size(); ++i) {
                        if (variables[i].type() != ValueType::Empty) {
                            err << variableNames[i] << "=";
                            printValue(i, err);
                        }
                    }
                    goto getCommand;
                }
            }
            procedureStack.emplace(currentInstruction.id);
            break;
        case InstructionType::Print:
            LOG("print");
            printValue(currentInstruction.id, out);
            break;
        }
        ++currentProcedure->instruction;
#undef currentInstruction
    }
end:
    LOG("end");
#undef LOG
}
}
