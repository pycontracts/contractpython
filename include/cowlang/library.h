#ifndef LIB_CPTH
#define LIB_CPTH
#include <modules/blockchain_module.h>
#include <string.h>

bool execute_program(std::string raw, net_type network, blockchain_arguments blkchn, uint64_t gas, uint32_t gasprice);
void init_cryptopython();
std::string get_errorbuf();
std::string get_outbuf();

#endif