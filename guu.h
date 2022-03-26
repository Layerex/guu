#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace guu {
template<typename T>
void error(T message);

template<typename T>
void assert(bool predicate, T message);

template<class T, class Container>
struct stack : public std::stack<T, Container>
{
    Container &container()
    {
        return this->c;
    }
};

typedef size_t Id;
typedef long long Number; // TODO: long numbers

enum class ValueType { Empty, Number, String, Variable };
struct Value
{
    std::variant<std::monostate, Number, std::string, Id> value;
    ValueType type();
};

enum class InstructionType { Set, Call, Print };
struct Instruction
{
    const InstructionType type;
    const Id id;
    const Value arg;
    Instruction(InstructionType type, Id id) : type(type), id(id) { }
    Instruction(InstructionType type, Id id, Value arg) : type(type), id(id), arg(arg) { }
};

struct Procedure
{
    std::vector<Instruction> instructions;
};

struct ProcedureFrame
{
    Id id;
    Id instruction = 0;
    ProcedureFrame() = delete;
    ProcedureFrame(Id id) : id(id) { }
};

class Program
{
protected:
    const std::string entryProcedureName = "main";

    Id entryPoint;
    std::vector<Procedure> procedures;
    std::vector<Value> variables;
    std::vector<std::string> procedureNames;
    std::vector<std::string> variableNames;
    std::unordered_map<std::string, Id> procedureIds;
    std::unordered_map<std::string, Id> variableIds;
    std::unordered_set<std::string> undefinedProcedures;
    std::unordered_set<std::string> undefinedVariables;

    Id getVariableId(const std::string &name, const bool definition);
    Id getProcedureId(const std::string &name, const bool definition);
    std::optional<Number> toNumber(const std::string &string);
    void strip(std::string &string);

    void setValue(const Id keyId, Value &value);
    void printValue(const Id valueId, std::ostream &out);

    std::string getDebugCommand(std::ostream &out, std::istream &in);

public:
    Program(std::istream &in);
    void run(std::ostream &out, std::ostream &err, std::istream &in, const bool debug,
             const bool log);
};
}
