#include <assert.h>
#include <cowlang/Callable.h>
#include <cowlang/CallableCFunction.h>
#include <cowlang/Dictionary.h>
#include <cowlang/Interpreter.h>
#include <cowlang/Scope.h>
#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <float.h>
#include <iostream>
#include <limits.h>
#include <math.h>
#include <memory>
#include <modules/blockchain_module.h>
#include <stddef.h>
#include <string.h>

bool addr_check(const char *address)
{

    if(address == 0)
        return false;

    auto mp_chainparams = &btc_chainparams_main;
    if(cow::net == cow::net_type::TEST)
        mp_chainparams = &btc_chainparams_test;
    else if(cow::net == cow::net_type::REGTEST)
        mp_chainparams = &btc_chainparams_regtest;

    uint8_t buf[strlen(address) * 2];
    bool valid = false;
    int r = btc_base58_decode_check(address, buf, sizeof(buf));
    if(r > 0 && buf[0] == mp_chainparams->b58prefix_pubkey_address)
    {
        valid = true;
    }
    else if(r > 0 && buf[0] == mp_chainparams->b58prefix_script_address)
    {
        valid = true;
    }
    else
    {
        // check for bech32
        int version = 0;
        unsigned char programm[40] = { 0 };
        size_t programmlen = 0;
        if(segwit_addr_decode(&version, programm, &programmlen, mp_chainparams->bech32_hrp, address) == 1)
        {
            if(programmlen == 20)
            {
                valid = true;
            }
        }
    }
    return valid;
}

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

const char *SuicideException::what() const throw()
{
    return "Contract has suicided and will go into hibernation mode now.";
}
const char *RevertException::what() const throw()
{
    return "Contract execution has been reverted!";
}
const char *OutOfGasException::what() const throw()
{
    return "Contract execution has been aborted because the VM ran out of gas!";
}
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
std::map<std::string, uint64_t> send_map;

BlockchainModule::BlockchainModule(MemoryManager &mem) : Module(mem)
{
    // preseed RC4
    for(int i = 0; i < 256; i++)
        s(i) = i;
    seeded = 0;

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
    ADD_FUNCTION("suicide", assert_args, suicide);
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
    if((value <= 0) | ((uint64_t)value > contract_balance))
    {
        throw std::runtime_error("sending nothing or more than the contract balance would allow");
    }

    // Store that data in the send map (outside of the mapped heap)
    std::map<std::string, uint64_t>::iterator it = send_map.find(a);
    if(it != send_map.end())
    {
        uint64_t overflow_check = it->second;
        it->second += value;
        if(it->second <= overflow_check)
        {
            throw std::runtime_error("sending produces overflow in receiver balance");
        }
    }
    else
        send_map[a] = value;

    // now decuct the remaining contract balance
    uint64_t overflow_check = it->second;
    contract_balance -= value;
    if(contract_balance <= overflow_check)
    {
        throw std::runtime_error("sending produces overflow in contract balance");
    }

    return wrap_value(new(mem) BoolVal(mem, true));
}

ValuePtr BlockchainModule::revert(Scope &scope) { throw RevertException(); }

ValuePtr BlockchainModule::suicide(Scope &scope)
{
    // First, we get the address from the arguments
    if(!scope.has_value("address"))
        throw std::runtime_error("address argument not present");
    ValuePtr p = scope.get_value("address");
    if(p == 0)
        throw std::runtime_error("pointer clash");
    std::string a = p->str();
    // Store that data in the send map (outside of the mapped heap)
    std::map<std::string, uint64_t>::iterator it = send_map.find(a);
    if(it != send_map.end())
    {
        uint64_t overflow_check = it->second;
        it->second += contract_balance;
        if(it->second <= overflow_check)
        {
            throw std::runtime_error("sending produces overflow in receiver balance");
        }
    }
    else
        send_map[a] = contract_balance;

    contract_balance = 0;
    throw SuicideException();
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
