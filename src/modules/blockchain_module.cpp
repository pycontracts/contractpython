#include <assert.h>
#include <cowlang/Callable.h>
#include <cowlang/CallableCFunction.h>
#include <cowlang/Dictionary.h>
#include <cowlang/Interpreter.h>
#include <cowlang/Scope.h>
#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <modules/blockchain_module.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#define s(i) _s[i]
/* Swap bytes that A and B point to. */
#define SWAP_BYTE(A, B)                 \
    do                                  \
    {                                   \
        unsigned char swap_temp = *(A); \
        *(A) = *(B);                    \
        *(B) = swap_temp;               \
    } while(0)


std::vector<std::string> empty;
#define EMPTY_ARGS empty


#define ADD_FUNCTION(x, m_args, y)    \
    function_map[x] = wrap_value(new( \
    mem) CallableCFunction(mem, m_args, std::bind(&BlockchainModule::y, this, std::placeholders::_1)));

namespace cow
{
net_type net = net_type::MAIN;

/* These are hardcoded default values for easy testing */
std::string txid = "a1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d";
std::string current_block = "00000000000000000015c23c0979270b91c26a562ae62463b85481f1d945bc21";
std::string previous_block = "0000000000000000002f3cb3939d8685c8976dc9e35ccec08c4e121b12688974";
uint32_t current_time = 1546659311;
uint32_t previous_time = 1546659307;
std::string sender = "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa";
std::string contract_address = "1XPTgDRhN8RFnzniWCddobD9iKZatrvH4";
uint64_t value = (uint64_t)50 * COIN;
uint64_t contract_balance = (uint64_t)300 * COIN;

BlockchainModule::BlockchainModule(MemoryManager &mem) : Module(mem)
{
    // preseed RC4
    for(int i = 0; i < 256; i++)
        s(i) = i;
    seeded = 0;

    // ADD_FUNCTION("fourtytwo", EMPTY_ARGS, fourtytwo);
    ADD_FUNCTION("txid", EMPTY_ARGS, get_txid);
    ADD_FUNCTION("current_block", EMPTY_ARGS, get_current_block);
    ADD_FUNCTION("previous_block", EMPTY_ARGS, get_previous_block);
    ADD_FUNCTION("current_time", EMPTY_ARGS, get_current_time);
    ADD_FUNCTION("previous_time", EMPTY_ARGS, get_previous_time);
    ADD_FUNCTION("sender", EMPTY_ARGS, get_sender);
    ADD_FUNCTION("contract_address", EMPTY_ARGS, get_contractaddress);
    ADD_FUNCTION("value", EMPTY_ARGS, get_value);
    ADD_FUNCTION("contract_balance", EMPTY_ARGS, get_contract_balance);
    ADD_FUNCTION("random", EMPTY_ARGS, get_random);

    std::vector<std::string> assert_args;
    assert_args.push_back("address");

    std::vector<std::string> send_args;
    send_args.push_back("address");
    send_args.push_back("value");

    ADD_FUNCTION("assert_address", assert_args, assert_address);
    ADD_FUNCTION("send", send_args, send);
    ADD_FUNCTION("revert", EMPTY_ARGS, revert);
}


ValuePtr BlockchainModule::get_current_block(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) StringVal(mem, current_block));
}

ValuePtr BlockchainModule::get_txid(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) StringVal(mem, txid));
}

ValuePtr BlockchainModule::assert_address(Scope &scope)
{
    auto &mem = memory_manager();

    if(!scope.has_value("address"))
        throw std::runtime_error("address argument not present");

    ValuePtr p = scope.get_value("address");
    if(p == 0)
        throw std::runtime_error("pointer clash");

    std::string a = p->str();
    if(a.size() == 0)
        throw std::runtime_error("address cannot be of length zero");

    if(!addr_check(a.c_str()))
        throw std::runtime_error("assert failed: argument is not a valid address");

    return wrap_value(new(mem) BoolVal(mem, true));
}

ValuePtr BlockchainModule::send(Scope &scope)
{
    auto &mem = memory_manager();

    // First, we get the address from the arguments
    if(!scope.has_value("address"))
        throw std::runtime_error("address argument not present");

    ValuePtr p = scope.get_value("address");
    if(p == 0)
        throw std::runtime_error("pointer clash");

    std::string a = p->str();
    if(a.size() == 0)
        throw std::runtime_error("address cannot be of length zero");

    if(!addr_check(a.c_str()))
        throw std::runtime_error("assert failed: argument is not a valid address");

    // and now, we get the value to be sent
    if(!scope.has_value("value"))
        throw std::runtime_error("value argument not present");

    ValuePtr v = scope.get_value("value");
    if(p == 0)
        throw std::runtime_error("pointer clash");

    int64_t value = unpack_integer(v);
    if(value<0 | value> contract_balance)
    {
        throw std::runtime_error("sending more than the contract has");
    }

    // TODO SEND
    return wrap_value(new(mem) BoolVal(mem, true));
}

ValuePtr BlockchainModule::revert(Scope &scope)
{
    throw std::runtime_error("contract execution has been reverted");
}


ValuePtr BlockchainModule::get_previous_block(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) StringVal(mem, previous_block));
}

ValuePtr BlockchainModule::get_sender(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) StringVal(mem, sender));
}

ValuePtr BlockchainModule::get_contractaddress(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) StringVal(mem, contract_address));
}

ValuePtr BlockchainModule::get_current_time(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) IntVal(mem, current_time));
}

ValuePtr BlockchainModule::get_previous_time(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) IntVal(mem, previous_time));
}

ValuePtr BlockchainModule::get_value(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) IntVal(mem, value));
}

ValuePtr BlockchainModule::get_contract_balance(Scope &scope)
{
    auto &mem = memory_manager();
    return wrap_value(new(mem) IntVal(mem, contract_balance));
}

ValuePtr BlockchainModule::get_random(Scope &scope)
{
    if(seeded == 0)
        seed();

    auto &mem = memory_manager();
    return wrap_value(new(mem) IntVal(mem, prng_get_uint()));
}

ValuePtr BlockchainModule::get_member(const std::string &name)
{

    if(function_map.find(name) != function_map.end())
    {
        return function_map[name];
    }
    // fallback
    throw std::runtime_error("no such "
                             "function '" +
                             name + "' in blockchain module");
}

void BlockchainModule::seed()
{
    // not seeding with integers because of the big / little endian dilemma
    prng_seed_bytes((unsigned char *)current_block.c_str(), strlen(current_block.c_str()));
    prng_seed_bytes((unsigned char *)previous_block.c_str(), strlen(previous_block.c_str()));
    prng_seed_bytes((unsigned char *)sender.c_str(), strlen(sender.c_str()));
    prng_seed_bytes((unsigned char *)contract_address.c_str(), strlen(contract_address.c_str()));
    prng_seed_bytes((unsigned char *)txid.c_str(), strlen(txid.c_str()));
}

void register_blockchain_module(Interpreter &i)
{
    auto v = wrap_value(new(i.memory_manager()) BlockchainModule(i.memory_manager()));
    i.set_module("blockchain", v);
};

} // namespace cow