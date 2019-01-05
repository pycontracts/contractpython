#ifndef HELP_H
#define HELP_H
#define __VERSION__ "ContractPython v1.0"

int help_generic(int, int){

              std::cout << termcolor::bold << "+-----------------------------------------------------------------+-----------------------------------+" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| Welcome to ContractPython 1.0, an embeddable Python interpreter | Module 'blockchain'               |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 | -------------------               |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| specifically designed for the use in blockchain projects.       | txid() is the current tx id       |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 | block() is the hash of the block  |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 |   the current transaction was     |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| Generally, you can use ContractPython just like any other       |   included into                   |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| Python interpreter. Due to security reasons, only a very        | prevblock() is the hash of the    |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| limited number of importable modules are available. Please      |   current block's parent          |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| consult the table on the right side for the modules             | time() is the current block's     |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| included in this distribution.                                  |   unix timestamp                  |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 | prevtime() is the prev. block's   |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "+                                                                 +   unix timestamp                  +" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| ContractPython allows you to configure virtual                  | sender() is the address of the    |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| \"blockchain parameters\" (such as the current block or the       |   sender of the current tx        |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| current transaction id) using command line arguments.           | contract() is the address of the  |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| Run ./contractpython -h in order to find out more.              |   contract                        |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 | value() is the value (in the      |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| If you do not need an interactive shell, but rather want to     |   smallest denomination possible) |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| compile a python script into byte code in order to submit it    |   that was sent to this contract  |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| to the blockchain, make sure to run contractpython with the     | random() gives back a perfectly   |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| -x flag: ./contractpython -x file.py                            |   deterministic random integer    |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 |   based on RC4 seeded with        |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| The compiled byte code will be saved as: file.py.bitstream      |   the values from above           |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 | revert() aborts the contract      |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 |   execution and reestablishes the |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| For testing: after you exit this shell, the entire send         |   old state. GAS costs still apply|" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| table and the permanent storage dict (see right side)           | suicide(address) kills the        |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| are dumped. If you want to clear the entire state at any time,  |   contract and sends the balance  |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| you can just run the clear() command.                           |   to 'address'                    |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "|                                                                 | balance() contains the contract   |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| Each session has an instruction limit to avoid infinite         |   balance in the smallest denom.  |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| loops and resource intentsive iterations inside the contracts.  | send(address, value) allows the   |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| The limits are configurable via the -g flag, and can be also    |   to send 'value' to 'address'    |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| reset by either clear() [whole session is cleared]              | store [dict] is a dictionary which|" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "| or by clear_limits() [only the instruction limits are cleared]  |   is permanent accross calls      |" << termcolor::reset << std::endl;
              std::cout << termcolor::bold << "+-----------------------------------------------------------------+-----------------------------------+" << termcolor::reset << std::endl << std::endl;
}
#endif HELP_H
