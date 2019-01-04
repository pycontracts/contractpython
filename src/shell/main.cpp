#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "pypa/parser/parser.hh"
#include "pypa/parser/error.hh"
#include <unistd.h>

using namespace cow;

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
const uint8_t attr[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, AT_SP, AT_SP, AT_SP, AT_SP, AT_SP, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    AT_SP, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR,
    AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR,
    AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_DI, AT_DI,
    AT_DI, AT_DI, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR,
    AT_PR, AT_UX, AT_UX, AT_UX, AT_UX, AT_UX, AT_UX, AT_UP,
    AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP,
    AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP, AT_UP,
    AT_UP, AT_UP, AT_UP, AT_PR, AT_PR, AT_PR, AT_PR, AT_PR,
    AT_PR, AT_LX, AT_LX, AT_LX, AT_LX, AT_LX, AT_LX, AT_LO,
    AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO,
    AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO, AT_LO,
    AT_LO, AT_LO, AT_LO, AT_PR, AT_PR, AT_PR, AT_PR, 0
};

bool unichar_isident(char c) {
    return c < 128 && ((attr[c] & (FL_ALPHA | FL_DIGIT)) != 0 || c == '_');
}


bool str_startswith_word(const char *str, const char *head) {
    size_t i;
    for (i = 0; str[i] && head[i]; i++) {
        if (str[i] != head[i]) {
            return false;
        }
    }
    return head[i] == '\0' && (str[i] == '\0' || !unichar_isident(str[i]));
}

bool mp_repl_continue_with_input(const char *input) {

    // check for blank input
    if (input[0] == '\0') {
        return false;
    }

    // check if input starts with a certain keyword
    bool starts_with_compound_keyword =
           input[0] == '@'
        || str_startswith_word(input, "if")
        || str_startswith_word(input, "while")
        || str_startswith_word(input, "for")
        || str_startswith_word(input, "try")
        || str_startswith_word(input, "with")
        || str_startswith_word(input, "def")
        || str_startswith_word(input, "class")
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
    for (i = input; *i; i++) {
        if (*i == '\'') {
            if ((in_quote == Q_NONE || in_quote == Q_3_SINGLE) && i[1] == '\'' && i[2] == '\'') {
                i += 2;
                in_quote = Q_3_SINGLE - in_quote;
            } else if (in_quote == Q_NONE || in_quote == Q_1_SINGLE) {
                in_quote = Q_1_SINGLE - in_quote;
            }
        } else if (*i == '"') {
            if ((in_quote == Q_NONE || in_quote == Q_3_DOUBLE) && i[1] == '"' && i[2] == '"') {
                i += 2;
                in_quote = Q_3_DOUBLE - in_quote;
            } else if (in_quote == Q_NONE || in_quote == Q_1_DOUBLE) {
                in_quote = Q_1_DOUBLE - in_quote;
            }
        } else if (*i == '\\' && (i[1] == '\'' || i[1] == '"' || i[1] == '\\')) {
            if (in_quote != Q_NONE) {
                i++;
            }
        } else if (in_quote == Q_NONE) {
            switch (*i) {
                case '(': n_paren += 1; break;
                case ')': n_paren -= 1; break;
                case '[': n_brack += 1; break;
                case ']': n_brack -= 1; break;
                case '{': n_brace += 1; break;
                case '}': n_brace -= 1; break;
                default: break;
            }
        }
    }

    // continue if unmatched 3-quotes
    if (in_quote == Q_3_SINGLE || in_quote == Q_3_DOUBLE) {
        return true;
    }

    // continue if unmatched brackets, but only if not in a 1-quote
    if ((n_paren > 0 || n_brack > 0 || n_brace > 0) && in_quote == Q_NONE) {
        return true;
    }

    // continue if last character was backslash (for line continuation)
    if (i[-1] == '\\') {
        return true;
    }

    // continue if compound keyword and last line was not empty
    if (starts_with_compound_keyword && i[-1] != '\n') {
        return true;
    }

    // otherwise, don't continue
    return false;
}

std::stringstream error_buffer;
int HANDLE_WITH_FULL_HEADER = 0;

void handle_error(pypa::Error e){

    // we only display the first error
    error_buffer.seekg(0, std::ios::end);
    int size = error_buffer.tellg();
    if(size > 0) return;

    unsigned long long cur_line = e.cur.line;

    if(HANDLE_WITH_FULL_HEADER)
        error_buffer << "File \"__stdin__\", line " << cur_line << "\n    " << e.line.c_str() << "\n    ";
    else
        error_buffer << "    ";

    if(e.cur.column == 0) ++e.cur.column;
    for(unsigned i = 0; i < e.cur.column - 1; ++i) {
        error_buffer << " ";
    }
    error_buffer << "^\n" << ( e.type == pypa::ErrorType::SyntaxError ? "SyntaxError" : e.type == pypa::ErrorType::SyntaxWarning ? "SyntaxWarning" : "IndentationError" ) << ": " << e.message.c_str();
    error_buffer << std::endl;
}
std::function<void(pypa::Error)> err_func(handle_error);

void handle_src_file(std::string& filename, Interpreter& pyint){
    HANDLE_WITH_FULL_HEADER = 1;
    try {
        auto doc = compile_file(filename, err_func);
        pyint.re_assign_bitstream(doc);
        pyint.execute();
    }
    catch(std::exception& e){
        error_buffer.seekg(0, std::ios::end);
        int size = error_buffer.tellg();
        if(size>0)
            std::cerr << error_buffer.str();
        else
            std::cerr << "RuntimeError: " << e.what() << std::endl;

        error_buffer.clear();
    }
}

void compile_src_file(std::string& filename){
    HANDLE_WITH_FULL_HEADER = 1;
    try {
        auto doc = compile_file(filename, err_func);
        std::ofstream fl (filename + ".bitstream", std::ios::out | std::ios::binary);
        fl.write((const char*)doc.data(), doc.size());
        fl.close();

    }
    catch(std::exception& e){
        error_buffer.seekg(0, std::ios::end);
        int size = error_buffer.tellg();
        if(size>0)
            std::cerr << error_buffer.str();
        else
            std::cerr << "CompileError: " << e.what() << std::endl;

        error_buffer.clear();
    }
}

void handle_readline(Interpreter& pyint) {

    rl_bind_key('\t', rl_insert);
    using_history();  /* initialize history */

    for (;;) {
        std::string line = "";

        // tunnel C Readline
        char* buf;
        if ((buf = readline(">>> ")) == nullptr) return;
        line += std::string(buf);
        free(buf);

        while (mp_repl_continue_with_input(line.c_str())) {
            char* buf;
            if ((buf = readline("... ")) == nullptr) return;
            line += "\n" + std::string(buf);
            free(buf);
        }

        if (strlen(line.c_str()) > 0) {
            add_history(line.c_str());
        }

        try {
            auto doc = compile_string(line, err_func);
            pyint.re_assign_bitstream(doc);
            pyint.execute();
            line.clear(); // clear
        }
        catch(std::exception& e){
            error_buffer.seekg(0, std::ios::end);
            int size = error_buffer.tellg();
            if(size>0)
                std::cerr << error_buffer.str();
            else
                std::cerr << "RuntimeError: " << e.what() << std::endl;

            error_buffer.clear();
        }
    }

}

void print_program_output(const std::string &str){
        std::cout << str;
}

int main (int argc, char *argv[]) {

    // scoped block for proper garbage collection
    {
        // initialize default mem mem_manager
        // this basically is a virtual, size-limited heap
        DefaultMemoryManager mem_manager;

        // Command line parsing
        bool only_compile = false;

        // Retrieve the (non-option) argument:
        std::string input = "";
        if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') ) {  // there is NO input...
        }
        else {  // there is an input...
            input = argv[argc-1];
        }

        // Shut GetOpt error messages down (return '?'):
        int opt;
        opterr = 0;

        // Retrieve the options:
        while ( (opt = getopt(argc, argv, "x")) != -1 ) {  // for each option...
            switch ( opt ) {
                case 'x':
                        only_compile = true;
                    break;
                case '?':  // unknown option...
                    std::cerr << "Unknown option: '" << char(optopt) << "'!" << std::endl;
                    exit(1);
            }
        }

        auto doc = compile_string("");
        Interpreter pyint(doc, mem_manager);
        pyint.execute();

        if(input=="")
            handle_readline(pyint);
        else{
            if(only_compile)
                compile_src_file(input);
            else
                handle_src_file(input, pyint);
        }


    }

}
