#include "pypa/parser/error.hh"
#include "pypa/parser/parser.hh"
#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <fstream>
#include <helpmessages.h>
#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <modules/blockchain_module.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <snappy.h>
#include <sstream>
#include <stdio.h>
#include <termcolor.h>
#include <unistd.h>

using namespace cow;

namespace cow
{
extern size_t DEFAULT_MAXIMUM_HEAP_PAGES;
}

// attribute flags
#define FL_PRINT (0x01)
#define FL_SPACE (0x02)
#define FL_DIGIT (0x04)
#define FL_ALPHA (0x08)
#define FL_UPPER (0x10)
#define FL_LOWER (0x20)
#define FL_XDIGIT (0x40)


// shorthand character attributes
#define AT_PR (FL_PRINT)
#define AT_SP (FL_SPACE | FL_PRINT)
#define AT_DI (FL_DIGIT | FL_PRINT | FL_XDIGIT)
#define AT_AL (FL_ALPHA | FL_PRINT)
#define AT_UP (FL_UPPER | FL_ALPHA | FL_PRINT)
#define AT_LO (FL_LOWER | FL_ALPHA | FL_PRINT)
#define AT_UX (FL_UPPER | FL_ALPHA | FL_PRINT | FL_XDIGIT)
#define AT_LX (FL_LOWER | FL_ALPHA | FL_PRINT | FL_XDIGIT)

// table of attributes for ascii characters
const uint8_t attr[] = { 0,     0,     0,     0,     0,     0,     0,     0,     0,     AT_SP,
                         AT_SP, AT_SP, AT_SP, AT_SP, 0,     0,     0,     0,     0,     0,
                         0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                         0,     0,     AT_SP, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR,
                         AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_DI, AT_DI,
                         AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_PR, AT_PR,
                         AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_UX, AT_UX, AT_UX, AT_UX, AT_UX,
                         AT_UX, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP,
                         AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP,
                         AT_UP, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_LX, AT_LX, AT_LX,
                         AT_LX, AT_LX, AT_LX, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO,
                         AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO,
                         AT_LO, AT_LO, AT_LO, AT_PR, AT_PR, AT_PR, AT_PR, 0 };

// Command line parsing
bool only_compile = false;
bool unsnappy = false;
uint32_t gas = 5000000;
uint32_t gasprice = 100;
uint32_t pagelimit = DEFAULT_MAXIMUM_HEAP_PAGES;


std::string printsize(uint32_t size, bool bytes = true)
{
    static const char *SIZES[] = { "B", "K", "M", "G" };
    size_t div = 0;
    size_t rem = 0;
    while(size >= 1024 && div < (sizeof SIZES / sizeof *SIZES))
    {
        rem = (size % 1024);
        div++;
        size /= 1024;
    }
    char result[1024];
    sprintf(result, "%.1f%s", (float)size + (float)rem / 1024.0, (div == 0 && !bytes) ? "" : SIZES[div]);
    return result;
}


bool unichar_isident(char c)
{
    return c < 128 && ((attr[(int)c] & (FL_ALPHA | FL_DIGIT)) != 0 || c == '_');
}


bool str_startswith_word(const char *str, const char *head)
{
    size_t i;
    for(i = 0; str[i] && head[i]; i++)
    {
        if(str[i] != head[i])
        {
            return false;
        }
    }
    return head[i] == '\0' && (str[i] == '\0' || !unichar_isident(str[i]));
}

bool mp_repl_continue_with_input(const char *input)
{

    // check for blank input
    if(input[0] == '\0')
    {
        return false;
    }

    // check if input starts with a certain keyword
    bool starts_with_compound_keyword =
    input[0] == '@' || str_startswith_word(input, "if") || str_startswith_word(input, "while") ||
    str_startswith_word(input, "for") || str_startswith_word(input, "try") ||
    str_startswith_word(input, "with") || str_startswith_word(input, "def") ||
    str_startswith_word(input, "class")
#if MICROPY_PY_ASYNC_AWAIT
    || str_startswith_word(input, "async")
#endif
    ;

// check for unmatched open bracket, quote or escape quote
#define Q_NONE (0)
#define Q_1_SINGLE (1)
#define Q_1_DOUBLE (2)
#define Q_3_SINGLE (3)
#define Q_3_DOUBLE (4)
    int n_paren = 0;
    int n_brack = 0;
    int n_brace = 0;
    int in_quote = Q_NONE;
    const char *i;
    for(i = input; *i; i++)
    {
        if(*i == '\'')
        {
            if((in_quote == Q_NONE || in_quote == Q_3_SINGLE) && i[1] == '\'' && i[2] == '\'')
            {
                i += 2;
                in_quote = Q_3_SINGLE - in_quote;
            }
            else if(in_quote == Q_NONE || in_quote == Q_1_SINGLE)
            {
                in_quote = Q_1_SINGLE - in_quote;
            }
        }
        else if(*i == '"')
        {
            if((in_quote == Q_NONE || in_quote == Q_3_DOUBLE) && i[1] == '"' && i[2] == '"')
            {
                i += 2;
                in_quote = Q_3_DOUBLE - in_quote;
            }
            else if(in_quote == Q_NONE || in_quote == Q_1_DOUBLE)
            {
                in_quote = Q_1_DOUBLE - in_quote;
            }
        }
        else if(*i == '\\' && (i[1] == '\'' || i[1] == '"' || i[1] == '\\'))
        {
            if(in_quote != Q_NONE)
            {
                i++;
            }
        }
        else if(in_quote == Q_NONE)
        {
            switch(*i)
            {
            case '(':
                n_paren += 1;
                break;
            case ')':
                n_paren -= 1;
                break;
            case '[':
                n_brack += 1;
                break;
            case ']':
                n_brack -= 1;
                break;
            case '{':
                n_brace += 1;
                break;
            case '}':
                n_brace -= 1;
                break;
            default:
                break;
            }
        }
    }

    // continue if unmatched 3-quotes
    if(in_quote == Q_3_SINGLE || in_quote == Q_3_DOUBLE)
    {
        return true;
    }

    // continue if unmatched brackets, but only if not in a 1-quote
    if((n_paren > 0 || n_brack > 0 || n_brace > 0) && in_quote == Q_NONE)
    {
        return true;
    }

    // continue if last character was backslash (for line continuation)
    if(i[-1] == '\\')
    {
        return true;
    }

    // continue if compound keyword and last line was not empty
    if(starts_with_compound_keyword && i[-1] != '\n')
    {
        return true;
    }

    // otherwise, don't continue
    return false;
}

std::stringstream error_buffer;
std::stringstream stdout_buffer;

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


void print_instruction_limit(Interpreter &pyint)
{
    const uint32_t current_ins = pyint.num_execution_steps();
    const uint32_t max_ins = pyint.max_execution_steps();
    ;
    const uint32_t current_mem = pyint.num_mem();
    const uint32_t max_mem = pyint.max_mem();

    std::cout << termcolor::on_grey << termcolor::yellow << termcolor::bold << "limits: "
              << "[" << termcolor::magenta << "instruction limit: " << printsize(current_ins, false)
              << " / " << printsize(max_ins, false) << termcolor::yellow << " | " << termcolor::blue
              << "memory used " << printsize(current_mem) << " / " << printsize(max_mem) << " "
              << termcolor::yellow << "] " << termcolor::reset << std::endl;
}

inline bool ends_with(std::string const &value, std::string const &ending)
{
    if(ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void handle_src_file(std::string &filename, Interpreter &pyint)
{
    HANDLE_WITH_FULL_HEADER = 1;
    try
    {
        if(ends_with(filename, ".bitstream.unsnappy") || unsnappy)
        {
            std::ifstream input(filename, std::ios::binary);
            std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            pyint.re_assign_bitstream(str);
            pyint.execute();
        }
        else if(ends_with(filename, ".bitstream"))
        {
            std::ifstream input(filename, std::ios::binary);
            std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            std::string decompressed;
            snappy::Uncompress(str.data(), str.size(), &decompressed);
            pyint.re_assign_bitstream(decompressed);
            pyint.execute();
        }
        else
        {
            auto doc = compile_file(filename, err_func);
            pyint.re_assign_bitstream(doc);
            pyint.execute();
        }

        stdout_buffer.seekg(0, std::ios::end);
        int size = stdout_buffer.tellg();
        if(size > 0)
            std::cout << termcolor::green << stdout_buffer.str() << termcolor::reset;
        stdout_buffer.str("");
        ;
    }
    catch(RevertException &e)
    {
        stdout_buffer.seekg(0, std::ios::end);
        int size = stdout_buffer.tellg();
        if(size > 0)
            std::cout << termcolor::green << stdout_buffer.str() << termcolor::reset;
        stdout_buffer.str("");
        std::cerr << termcolor::on_blue << termcolor::green
                  << "** the contract execution has been reverted gracefully **" << termcolor::reset
                  << std::endl;
    }
    catch(SuicideException &e)
    {
        stdout_buffer.seekg(0, std::ios::end);
        int size = stdout_buffer.tellg();
        if(size > 0)
            std::cout << termcolor::green << stdout_buffer.str() << termcolor::reset;
        stdout_buffer.str("");
        std::cerr << termcolor::on_blue << termcolor::green
                  << "** the contract has suicided, and will go into hibernation mode now **"
                  << termcolor::reset << std::endl;
    }
    catch(std::exception &e)
    {
        stdout_buffer.seekg(0, std::ios::end);
        int size = stdout_buffer.tellg();
        if(size > 0)
            std::cout << termcolor::green << stdout_buffer.str() << termcolor::reset;
        stdout_buffer.str("");
        error_buffer.seekg(0, std::ios::end);
        size = error_buffer.tellg();
        if(size > 0)
            std::cerr << termcolor::on_white << termcolor::red << error_buffer.str() << termcolor::reset;
        else
            std::cerr << termcolor::on_white << termcolor::red << "RuntimeError: " << e.what()
                      << termcolor::reset << std::endl;

        error_buffer.str("");
    }
    print_instruction_limit(pyint);
}

void compile_src_file(std::string &filename)
{
    HANDLE_WITH_FULL_HEADER = 1;
    try
    {
        auto doc = compile_file(filename, err_func);
        std::ofstream fl(filename + ".bitstream", std::ios::out | std::ios::binary);
        std::string raw = doc.store();
        std::string compressed;
        snappy::Compress(raw.data(), raw.size(), &compressed);

        fl.write((const char *)compressed.data(), compressed.size());
        fl.close();
    }
    catch(std::exception &e)
    {
        error_buffer.seekg(0, std::ios::end);
        int size = error_buffer.tellg();
        if(size > 0)
            std::cerr << termcolor::on_white << termcolor::red << error_buffer.str() << termcolor::reset;
        else
            std::cerr << termcolor::on_white << termcolor::red << "CompileError: " << e.what()
                      << termcolor::reset << std::endl;

        error_buffer.str("");
    }
}

void compile_src_file_unsnappy(std::string &filename)
{
    HANDLE_WITH_FULL_HEADER = 1;
    try
    {
        auto doc = compile_file(filename, err_func);
        std::ofstream fl(filename + ".bitstream.unsnappy", std::ios::out | std::ios::binary);
        std::string compressed = doc.store();

        fl.write((const char *)compressed.data(), compressed.size());
        fl.close();
    }
    catch(std::exception &e)
    {
        error_buffer.seekg(0, std::ios::end);
        int size = error_buffer.tellg();
        if(size > 0)
            std::cerr << termcolor::on_white << termcolor::red << error_buffer.str() << termcolor::reset;
        else
            std::cerr << termcolor::on_white << termcolor::red << "CompileError: " << e.what()
                      << termcolor::reset << std::endl;

        error_buffer.str("");
    }
}

std::string get_prompt(const char *prompt) { return prompt; }

void handle_readline(Interpreter &pyint)
{

    rl_bind_key('\t', rl_insert);
    using_history(); /* initialize history */

    for(;;)
    {
        std::string line = "";

        // tunnel C Readline
        char *buf;
        print_instruction_limit(pyint);
        if((buf = readline(get_prompt(">>> ").c_str())) == nullptr)
            return;
        line += std::string(buf);
        free(buf);

        while(mp_repl_continue_with_input(line.c_str()))
        {
            char *buf;
            print_instruction_limit(pyint);
            if((buf = readline(get_prompt("... ").c_str())) == nullptr)
                return;
            line += "\n" + std::string(buf);
            free(buf);
        }

        if(strlen(line.c_str()) > 0)
        {
            add_history(line.c_str());
        }

        try
        {
            auto doc = compile_string(line, err_func);
            pyint.re_assign_bitstream(doc);
            pyint.execute(); // make print also print to a buffer

            stdout_buffer.seekg(0, std::ios::end);
            int size = stdout_buffer.tellg();
            if(size > 0)
                std::cout << termcolor::green << stdout_buffer.str() << termcolor::reset;
            stdout_buffer.str("");

            line.clear(); // clear
        }
        catch(RevertException &e)
        {
            std::cerr << termcolor::on_blue << termcolor::green
                      << "** the contract execution has been reverted gracefully **"
                      << termcolor::reset << std::endl;
        }
        catch(SuicideException &e)
        {
            std::cerr << termcolor::on_blue << termcolor::green
                      << "** the contract has suicided, and will go into hibernation mode now **"
                      << termcolor::reset << std::endl;
        }
        catch(std::exception &e)
        {
            error_buffer.seekg(0, std::ios::end);
            int size = error_buffer.tellg();
            if(size > 0)
                std::cerr << termcolor::on_white << termcolor::red << error_buffer.str() << termcolor::reset;
            else
                std::cerr << termcolor::on_white << termcolor::red << "RuntimeError: " << e.what()
                          << termcolor::reset << std::endl;

            error_buffer.str("");
        }
    }
}


int usage(char **argv)
{
    printf("usage: %s [<opts>] [<filename> | <contract address> | <empty>]\n"
           "\nOptions:\n\n"
           "-x             : compile python file into bitstream\n"
           "-x             : compile python file into bitstream\n"
           "-S             : (unsnappy) compiles the bitstream without compression (debug only)\n"
           "-G [N]         : set gasprice, maximum instructions will be gas / gasprice [default: "
           "100]\n"
           "-n [N]         : set network type (0=main, 1=testnet, 2=regtest) [default: 0]\n"
           "-m [N]         : memory limit in PAGES (page size is 1MB) [default: 3]\n"
           "\nBlockchain parameters (only used when last parameter is not a contract address):\n\n"
           "-t [<hash>]    : current txid\n"
           "                 [default: %s]\n"
           "-b [<hash>]    : current block's hash in hex format\n"
           "                 [default: %s]\n"
           "-B [<hash>]    : previous block's hash in hex format\n"
           "                 [default: %s]\n"
           "-t [N]         : current block's UNIX timestamp [default: %u]\n"
           "-T [N]         : previous block's UNIX timestamp [default: %u]\n"
           "-H [N]         : current block's height [default: %u]\n"
           "-s [<address>] : the sender of the current transaction\n"
           "                 [default: %s]\n"
           "-a [<address>] : the current address of the contract\n"
           "                 [default: %s]\n"
           "-v [N]         : the amount that was sent in this transaction [default: %" PRIu64 "]\n"
           "-V [N]         : the contract's current balance [default: %" PRIu64 "]\n"
           "\n",
           argv[0], cow::txid.c_str(), cow::current_block.c_str(), cow::previous_block.c_str(),
           cow::current_time, cow::previous_time, cow::current_height, cow::sender.c_str(),
           cow::contract_address.c_str(), cow::value, cow::contract_balance);

    return 1;
}

int pre_process_options(int argc, char **argv)
{
    int skip = 0;
    for(int a = 1; a < argc; a++)
    {
        if(argv[a][0] == '-')
        {
            skip++;
            if(strcmp(argv[a], "-h") == 0)
            {
                exit(usage(argv));
            }
            else if(strcmp(argv[a], "-b") == 0)
            {
                cow::current_block = argv[a + 1];
                skip++;
            }
            else if(strcmp(argv[a], "-x") == 0)
            {
                only_compile = true;
            }
            else if(strcmp(argv[a], "-S") == 0)
            {
                unsnappy = true;
            }
            else if(strcmp(argv[a], "-B") == 0)
            {
                cow::previous_block = argv[a + 1];
                skip++;
            }
            else if(strcmp(argv[a], "-t") == 0)
            {
                cow::current_time = atoi(argv[a + 1]);
                skip++;
            }
            else if(strcmp(argv[a], "-T") == 0)
            {
                cow::previous_time = atoi(argv[a + 1]);
                skip++;
            }
            else if(strcmp(argv[a], "-H") == 0)
            {
                cow::current_height = atoi(argv[a + 1]);
                skip++;
            }
            else if(strcmp(argv[a], "-s") == 0)
            {
                cow::sender = argv[a + 1];
            }
            else if(strcmp(argv[a], "-a") == 0)
            {
                cow::contract_address = argv[a + 1];
                skip++;
            }
            else if(strcmp(argv[a], "-v") == 0)
            {
                cow::value = atol(argv[a + 1]);
                skip++;
            }
            else if(strcmp(argv[a], "-V") == 0)
            {
                cow::contract_balance = atol(argv[a + 1]);
                skip++;
            }
            else if(strcmp(argv[a], "-g") == 0)
            {
                gas = atoi(argv[a + 1]);
            }
            else if(strcmp(argv[a], "-p") == 0)
            {
                gasprice = atoi(argv[a + 1]);
            }
            else if(strcmp(argv[a], "-n") == 0)
            {
                uint32_t _net = atoi(argv[a + 1]);
                skip++;
                if(_net > 2)
                {
                    exit(usage(argv));
                }
                if(_net == 0)
                    net = net_type::MAIN;
                if(_net == 1)
                    net = net_type::TEST;
                if(_net == 2)
                    net = net_type::REGTEST;
            }
            else if(strcmp(argv[a], "-m") == 0)
            {
                pagelimit = atoi(argv[a + 1]);
                skip++;
                DEFAULT_MAXIMUM_HEAP_PAGES = pagelimit;
            }
        }
    }
    return skip;
}

int main(int argc, char *argv[])
{

    // scoped block for proper garbage collection
    {

        // activate devmode, e.g., certain interactive features like Clear() and ClearLimits()
        devmode = true;
        contractmode = false; // special flag if using the "contract interaction shell"

        // initialize default mem mem_manager
        // this basically is a virtual, size-limited heap
        DefaultMemoryManager mem_manager;

        // parte options
        int skip = pre_process_options(argc, argv);

        // Retrieve the (non-option) argument:
        std::string input = "";
        if((argc <= 1) || (argc - 1 == skip))
        { // there is NO input...
        }
        else
        { // there is an input...
            input = argv[argc - 1];
        }

        auto doc = compile_string("");
        Interpreter pyint(doc, mem_manager);
        uint64_t limit = gas;
        limit /= gasprice;
        pyint.set_execution_step_limit((uint32_t)limit);
        register_blockchain_module(pyint);


        if(input == "")
        {
            std::cout << termcolor::bold << "\nWelcome to " << __VERSIONSTRING__ << std::endl;
            std::cout << termcolor::reset << termcolor::cyan << "Press CTRL+D to exit"
                      << termcolor::reset << std::endl
                      << std::endl;

            handle_readline(pyint);
        }
        else
        {
            if(only_compile)
                if(unsnappy)
                    compile_src_file_unsnappy(input);
                else
                    compile_src_file(input);
            else
            {
                handle_src_file(input, pyint);
            }
        }
        if(!only_compile)
        {
            std::cout << termcolor::bold
                      << "\nGood bye! We will now dump the send table for you:" << std::endl
                      << termcolor::reset << std::endl;
            std::cout << termcolor::cyan << "| " << std::setw(40) << "Address"
                      << " | " << std::setw(20) << "Value (in Satoshi)"
                      << " |" << termcolor::reset << std::endl;
            std::cout << termcolor::reset << termcolor::cyan
                      << "-------------------------------------------------------------------"
                      << termcolor::reset << std::endl;

            std::map<std::string, uint64_t>::iterator it = send_map.begin();
            if(it == send_map.end())
            {
                std::cout << termcolor::reset << termcolor::cyan << "| " << termcolor::reset
                          << std::setw(63) << "The send table has no entries" << termcolor::reset
                          << termcolor::cyan << " |" << termcolor::reset << std::endl;
            }
            while(it != send_map.end())
            {
                std::cout << termcolor::reset << termcolor::cyan << "| " << termcolor::reset
                          << std::setw(40) << it->first << termcolor::reset << termcolor::cyan << " | "
                          << termcolor::reset << std::setw(20) << it->second << termcolor::reset
                          << termcolor::cyan << " |" << termcolor::reset << std::endl;
                ++it;
            }

            std::cout << termcolor::reset << termcolor::cyan
                      << "-------------------------------------------------------------------"
                      << termcolor::reset << std::endl;
        }
    }
}
