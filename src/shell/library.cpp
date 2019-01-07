#include "pypa/parser/error.hh"
#include "pypa/parser/parser.hh"
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

bool execute_program(std::string raw, net_type network, blockchain_arguments blkchn, uint64_t gas, uint32_t gasprice)
{
    try
    {

        // make sure to select the correct net
        net = network;

        // init everything
        DefaultMemoryManager mem_manager;
        std::string decompressed;
        snappy::Uncompress(raw.data(), raw.size(), &decompressed);

        bitstream doc(decompressed);
        Interpreter pyint(doc, mem_manager);
        uint64_t limit = gas;
        limit /= gasprice;
        pyint.set_execution_step_limit((uint32_t)limit);
        register_blockchain_module(pyint);

        // Go for it
        pyint.execute(); // make print also print to a buffer

        return true;
    }
    catch(std::exception &e)
    {
        error_buffer << "ExecutionException: " << e.what();
        error_buffer << std::endl;
        return false;
    }
}