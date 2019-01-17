#include "pypa/parser/error.hh"
#include "pypa/parser/parser.hh"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <modules/blockchain_module.h>
#include <snappy.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

using namespace cow;


std::stringstream error_buffer;
std::stringstream stdout_buffer;

std::string get_errorbuf()
{
    std::string err = error_buffer.str();
    error_buffer.str("");
    return err;
}

std::string get_outbuf()
{
    std::string out = stdout_buffer.str();
    stdout_buffer.str("");
    return out;
}

void init_cryptopython()
{
    // deactivate devmode
    devmode = false;
    contractmode = false; // special flag if using the "contract interaction shell"
}

void print_program_output(const std::string &str) { stdout_buffer << str; }

int HANDLE_WITH_FULL_HEADER = 0;

void handle_error(pypa::Error e)
{
    // we only display the first error
    error_buffer.seekg(0, std::ios::end);
    int size = error_buffer.tellg();
    if(size > 0)
        return;

    unsigned long long cur_line = e.cur.line;

    if(HANDLE_WITH_FULL_HEADER)
        error_buffer << "File \"__stdin__\", line " << cur_line << "\n    " << e.line.c_str() << "\n    ";
    else
        error_buffer << "    ";

    if(e.cur.column == 0)
        ++e.cur.column;
    for(unsigned i = 0; i < e.cur.column - 1; ++i)
    {
        error_buffer << " ";
    }
    error_buffer << "^\n"
                 << (e.type == pypa::ErrorType::SyntaxError ?
                     "SyntaxError" :
                     e.type == pypa::ErrorType::SyntaxWarning ? "SyntaxWarning" : "IndentationError")
                 << ": " << e.message.c_str();
    error_buffer << std::endl;
}
std::function<void(pypa::Error)> err_func(handle_error);


std::string compile_src_file(std::string &filename)
{
    try
    {
        auto doc = compile_file(filename, err_func);
        std::string raw = doc.store();
        std::string compressed;
        snappy::Compress(raw.data(), raw.size(), &compressed);
        return compressed;
    }
    catch(std::exception &e)
    {
        std::string exp = error_buffer.str();
        error_buffer.str("");
        if(exp.size() > 0)
        {
            throw std::runtime_error(exp);
        }
        else
        {
            throw e;
        }
    }
}


int execute_program(std::string &raw,
                    net_type network,
                    blockchain_arguments blkchn,
                    uint64_t gas,
                    uint32_t gasprice,
                    uint64_t &used_g,
                    std::string &old_storage,
                    std::stringstream &s)
{

    // possibly throws early on syntax error

    // set those global fields for the mod blockchain module
    cow::txid = blkchn.txid;
    cow::current_block = blkchn.current_block;
    cow::previous_block = blkchn.previous_block;
    cow::current_time = blkchn.current_time;
    cow::previous_time = blkchn.previous_time;
    cow::sender = blkchn.sender;
    cow::contract_address = blkchn.contract_address;
    cow::value = blkchn.value;
    cow::contract_balance = blkchn.contract_balance;

    // make sure to select the correct net
    net = network;


    // init everything
    DefaultMemoryManager mem_manager;
    std::string decompressed;
    snappy::Uncompress(raw.data(), raw.size(), &decompressed);

    bitstream doc(decompressed);
    Interpreter pyint(doc, mem_manager);
    std::shared_ptr<PersistableDictionary> stpt = pyint.get_storage_pointer();

    if(old_storage.size() > 0)
    {
        // unserializa old storage
        /*
        std::map<std::string, std::string> m_elements_string;
        std::map<std::string, int64_t> m_elements_int;
        std::map<std::string, double> m_elements_double;
        std::map<std::string, bool> m_elements_bool;
        */
        std::stringstream ss;
        ss.str(old_storage);
        boost::archive::text_iarchive iarch(ss);
        iarch >> stpt->m_elements_string;
        iarch >> stpt->m_elements_int;
        iarch >> stpt->m_elements_double;
        iarch >> stpt->m_elements_bool;
    }


    uint64_t limit = gas;
    limit /= gasprice;
    pyint.set_execution_step_limit((uint32_t)limit);
    register_blockchain_module(pyint);

    try
    {
        // Go for it
        pyint.execute(); // make print also print to a buffer
        used_g = (pyint.num_execution_steps()) * gasprice;
        {
            boost::archive::text_oarchive oarch(s);
            oarch << stpt->m_elements_string;
            oarch << stpt->m_elements_int;
            oarch << stpt->m_elements_double;
            oarch << stpt->m_elements_bool;
        }
        return 0;
    }
    catch(OutOfGasException &e)
    {
        used_g = (pyint.num_execution_steps()) * gasprice;
        {
            boost::archive::text_oarchive oarch(s);
            oarch << stpt->m_elements_string;
            oarch << stpt->m_elements_int;
            oarch << stpt->m_elements_double;
            oarch << stpt->m_elements_bool;
        }
        error_buffer << "ContractError: " << e.what();
        error_buffer << std::endl;
        return 0x70;
    }
    catch(SuicideException &e)
    {
        used_g = (pyint.num_execution_steps()) * gasprice;
        {
            boost::archive::text_oarchive oarch(s);
            oarch << stpt->m_elements_string;
            oarch << stpt->m_elements_int;
            oarch << stpt->m_elements_double;
            oarch << stpt->m_elements_bool;
        }
        error_buffer << "ContractError: " << e.what();
        error_buffer << std::endl;
        return 0x69;
    }
    catch(RevertException &e)
    {
        used_g = (pyint.num_execution_steps()) * gasprice;
        {
            boost::archive::text_oarchive oarch(s);
            oarch << stpt->m_elements_string;
            oarch << stpt->m_elements_int;
            oarch << stpt->m_elements_double;
            oarch << stpt->m_elements_bool;
        }
        error_buffer << "ContractError: " << e.what();
        error_buffer << std::endl;
        return 0x71;
    }
    catch(std::exception &e)
    {
        used_g = (pyint.num_execution_steps()) * gasprice;
        {
            boost::archive::text_oarchive oarch(s);
            oarch << stpt->m_elements_string;
            oarch << stpt->m_elements_int;
            oarch << stpt->m_elements_double;
            oarch << stpt->m_elements_bool;
        }

        error_buffer << "ContractError: " << e.what();
        error_buffer << std::endl;
        return 0x80;
    }
}
