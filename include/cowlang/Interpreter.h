#pragma once

#include <string>

#include "Dictionary.h"
#include "Module.h"
#include "NodeType.h"
#include "PersistableDictionary.h"
#include "Scope.h"
#include "Tuple.h"
#include "Value.h"

#include "execution_limits.h"
extern bool devmode;
extern bool contractmode;

#define ASSERT_LEFT_AND_RIGHT               \
    if(left == nullptr || right == nullptr) \
    throw std::runtime_error("VM was halted: bytecode is improperly formatted.")
#define ASSERT_LEFT     \
    if(left == nullptr) \
    throw std::runtime_error("VM was halted: bytecode is improperly formatted.")
#define ASSERT_RIGHT     \
    if(right == nullptr) \
    throw std::runtime_error("VM was halted: bytecode is improperly formatted.")
#define ASSERT_GENERIC(right) \
    if(right == nullptr)      \
    throw std::runtime_error("VM was halted: bytecode is improperly formatted.")
#define CHARGE_EXECUTION                                                              \
    m_num_execution_steps += 1;                                                       \
    if(m_execution_step_limit > 0 && m_num_execution_steps >= m_execution_step_limit) \
    {                                                                                 \
        throw OutOfGasException();                                                    \
    }

namespace cow
{

/**
 * Main class that takes care of running (=interpreting) the compiled code
 */
class Interpreter
{
public:
    /**
     * Construct the Python interpreter
     *
     * @param data
     *      The compiled syntax tree of the program
     */
    Interpreter(const bitstream &data, MemoryManager &mem);
    Interpreter(const bitstream &data, MemoryManager &mem, Interpreter &scope_borrower);
    ~Interpreter();

    void re_assign_bitstream(const bitstream &data);
    ValuePtr execute();
    ValuePtr execute_in_scope(Scope &scope);
    ValuePtr calldata(std::string &data);
    Scope &get_scope() { return *m_global_scope; };
    std::shared_ptr<PersistableDictionary> get_storage_pointer() { return store; }

    void set_value(const std::string &name, ValuePtr value);

    void set_module(const std::string &name, ModulePtr module);
    void set_list(const std::string &name, const std::vector<std::string> &list);
    void set_string(const std::string &name, const std::string &value);

    const uint32_t num_execution_steps() const;
    const uint32_t max_execution_steps() const;
    const uint32_t num_mem() const;
    const uint32_t max_mem() const;

    void set_execution_step_limit(uint32_t limit);
    void set_num_execution_steps(uint32_t current);

    MemoryManager &memory_manager() { return m_mem; }

private:
    enum class LoopState
    {
        None,
        TopLevel,
        Normal,
        Break,
        Continue,
        IgnoreAll
    };

    ModulePtr get_module(const std::string &name);

    ValuePtr execute_next(Scope &scope, LoopState &loop_state);
    void skip_next();

    void load_from_module(Scope &scope, const std::string &module, const std::string &name, const std::string &as_name);
    void load_module(Scope &scope, const std::string &name, const std::string &as_name);
    ValuePtr read_function_stub(Interpreter &i);
    std::string read_name();
    std::vector<std::string> read_names();

    MemoryManager &m_mem;
    bool do_not_free_scope;

    Scope *m_global_scope;

    std::unordered_map<std::string, ModulePtr> m_loaded_modules;

    uint32_t m_num_execution_steps;
    uint32_t m_execution_step_limit;
    bitstream m_data;

    std::shared_ptr<PersistableDictionary> store;
};

inline void Interpreter::set_value(const std::string &name, ValuePtr value)
{
    m_global_scope->set_value(name, value);
}

} // namespace cow
