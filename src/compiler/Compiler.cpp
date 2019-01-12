#include <bitstream.h>
#include <cowlang/NodeType.h>

#include "pypa/ast/ast.hh"
#include "pypa/filebuf.hh"
#include "pypa/lexer/lexer.hh"
#include "pypa/parser/parser.hh"
#include "pypa/reader.hh"

#define SAFE_PARSE_NEXT(ptr)                          \
    if(ptr == nullptr)                                \
    {                                                 \
        throw std::runtime_error("mailicious input"); \
    }                                                 \
    parse_next(*(ptr))

namespace cow
{

class StringReader : public pypa::Reader
{
public:
    StringReader(const std::string &code) : m_line_pos(0)
    {
        size_t last_pos = 0;
        size_t pos = std::string::npos;

        while((pos = code.find_first_of('\n', last_pos)) != std::string::npos)
        {
            m_code.push_back(code.substr(last_pos, pos + 1 - last_pos));
            last_pos = pos + 1;
        }

        m_code.push_back(code.substr(last_pos, code.size() - last_pos));
    }

    uint32_t get_line_number() const override { return m_line_pos; }

    std::string get_line(size_t idx) override
    {
        if(m_code.size() - 1 < idx)
            throw std::runtime_error("mailicious input");
        return m_code[idx];
    }

    std::string next_line() override
    {
        if(eof())
        {
            return "";
        }

        auto res = m_code[m_line_pos];
        m_line_pos += 1;
        return res;
    }

    std::string get_filename() const override { return "code"; }

    bool set_encoding(const std::string &coding) override
    {
        (void)coding;
        return true;
    }

    bool eof() const override { return m_line_pos == m_code.size(); }

private:
    uint32_t m_line_pos;
    std::vector<std::string> m_code;
};

class Compiler
{
public:
    Compiler(const pypa::AstModulePtr ast) : m_ast(ast) {}

    void run() { SAFE_PARSE_NEXT((m_ast->body)); }

    bitstream get_result() { return std::move(m_result); }

private:
    void parse_next(const pypa::AstExpr &expr)
    {
        parse_next(reinterpret_cast<const pypa::Ast &>(expr));
    }

    void parse_expr_list(const pypa::AstExprList &list)
    {
        uint32_t size = list.size();
        m_result << size;

        for(auto item : list)
        {
            if(item == 0)
            {
                m_result << NodeType::Pass;
            }
            else
            {
                SAFE_PARSE_NEXT(item);
            }
        }
    }


    void parse_stmt_list(const pypa::AstStmtList &list)
    {
        uint32_t size = list.size();
        m_result << size;

        for(auto item : list)
        {
            SAFE_PARSE_NEXT(item);
        }
    }

    void parse_next(const pypa::Ast &stmt)
    {

        switch(stmt.type)
        {
        case pypa::AstType::ImportFrom:
        {
            auto &import = reinterpret_cast<const pypa::AstImportFrom &>(stmt);
            m_result << NodeType::ImportFrom;

            SAFE_PARSE_NEXT(import.module);
            SAFE_PARSE_NEXT(import.names);
            break;
        }
        case pypa::AstType::Import:
        {
            auto &import = reinterpret_cast<const pypa::AstImport &>(stmt);
            m_result << NodeType::Import;
            SAFE_PARSE_NEXT(import.names);
            break;
        }
        case pypa::AstType::Alias:
        {
            auto &alias = reinterpret_cast<const pypa::AstAlias &>(stmt);
            m_result << NodeType::Alias;
            const std::string name = reinterpret_cast<const pypa::AstName &>(*alias.name).id.c_str();

            if(alias.as_name)
            {
                const std::string as_name =
                reinterpret_cast<const pypa::AstName &>(*alias.as_name).id.c_str();
                m_result << name << as_name;
            }
            else
                m_result << name << std::string("");
            break;
        }
        case pypa::AstType::Name:
        {
            auto &exp = reinterpret_cast<const pypa::AstName &>(stmt);
            m_result << NodeType::Name;

            std::string str = exp.id.c_str();
            m_result << str;
            break;
        }
        case pypa::AstType::Assign:
        {
            auto &assign = reinterpret_cast<const pypa::AstAssign &>(stmt);
            m_result << NodeType::Assign;
            SAFE_PARSE_NEXT(assign.value);
            parse_expr_list(assign.targets);
            break;
        }
        case pypa::AstType::Suite:
        {
            auto &suite = reinterpret_cast<const pypa::AstSuite &>(stmt);
            m_result << NodeType::StatementList;
            parse_stmt_list(suite.items);
            break;
        }
        case pypa::AstType::Str:
        case pypa::AstType::DocString:
        {
            auto &str = reinterpret_cast<const pypa::AstStr &>(stmt);
            m_result << NodeType::String;
            m_result << std::string(str.value.c_str());
            break;
        }
        case pypa::AstType::Return:
        {
            auto &ret = reinterpret_cast<const pypa::AstReturn &>(stmt);
            m_result << NodeType::Return;
            SAFE_PARSE_NEXT(ret.value);
            break;
        }
        case pypa::AstType::Dict:
        {
            auto &dict = reinterpret_cast<const pypa::AstDict &>(stmt);

            uint32_t size = dict.keys.size();
            m_result << NodeType::Dictionary << size;
            for(uint32_t i = 0; i < size; ++i)
            {
                SAFE_PARSE_NEXT(dict.keys[i]);
                SAFE_PARSE_NEXT(dict.values[i]);
            }
            break;
        }
        case pypa::AstType::Compare:
        {
            auto &comp = reinterpret_cast<const pypa::AstCompare &>(stmt);
            m_result << NodeType::Compare;
            SAFE_PARSE_NEXT(comp.left);

            uint32_t size = comp.comparators.size();
            m_result << size;
            for(uint32_t i = 0; i < size; ++i)
            {
                m_result << comp.operators[i];
                SAFE_PARSE_NEXT(comp.comparators[i]);
            }

            break;
        }
        case pypa::AstType::Number:
        {
            auto &num = reinterpret_cast<const pypa::AstNumber &>(stmt);

            if(num.num_type == pypa::AstNumber::Integer)
            {
                m_result << NodeType::Integer;
                int32_t i = num.integer;
                m_result << i;
            }
            else
                throw std::runtime_error("Unknown number type!");
            break;
        }
        case pypa::AstType::If:
        {
            auto &ifclause = reinterpret_cast<const pypa::AstIf &>(stmt);
            if(ifclause.orelse)
            {
                m_result << NodeType::IfElse;
                SAFE_PARSE_NEXT(ifclause.test);
                SAFE_PARSE_NEXT(ifclause.body);
                SAFE_PARSE_NEXT(ifclause.orelse);
            }
            else
            {
                m_result << NodeType::If;
                SAFE_PARSE_NEXT(ifclause.test);
                SAFE_PARSE_NEXT(ifclause.body);
            }
            break;
        }
        case pypa::AstType::Call:
        {
            auto &call = reinterpret_cast<const pypa::AstCall &>(stmt);
            m_result << NodeType::Call;
            SAFE_PARSE_NEXT(call.function);
            parse_expr_list(call.arglist.arguments);
            break;
        }
        case pypa::AstType::Attribute:
        {
            auto &attr = reinterpret_cast<const pypa::AstAttribute &>(stmt);
            m_result << NodeType::Attribute;
            SAFE_PARSE_NEXT(attr.value);
            SAFE_PARSE_NEXT(attr.attribute);
            break;
        }
        case pypa::AstType::UnaryOp:
        {
            auto &op = reinterpret_cast<const pypa::AstUnaryOp &>(stmt);
            m_result << NodeType::UnaryOp;
            m_result << op.op;
            SAFE_PARSE_NEXT(op.operand);
            break;
        }
        case pypa::AstType::BoolOp:
        {
            auto &op = reinterpret_cast<const pypa::AstBoolOp &>(stmt);
            m_result << NodeType::BoolOp;
            m_result << op.op;
            m_result << static_cast<uint32_t>(op.values.size());

            for(auto v : op.values)
                parse_next(v);
            break;
        }
        case pypa::AstType::BinOp:
        {
            auto &op = reinterpret_cast<const pypa::AstBinOp &>(stmt);
            m_result << NodeType::BinaryOp;
            m_result << op.op;
            SAFE_PARSE_NEXT(op.left);
            SAFE_PARSE_NEXT(op.right);

            break;
        }
        case pypa::AstType::List:
        {
            auto &list = reinterpret_cast<const pypa::AstList &>(stmt);
            m_result << NodeType::List;
            parse_expr_list(list.elements);
            break;
        }
        case pypa::AstType::Subscript:
        {
            auto &subs = reinterpret_cast<const pypa::AstSubscript &>(stmt);
            m_result << NodeType::Subscript;
            SAFE_PARSE_NEXT(subs.slice);
            SAFE_PARSE_NEXT(subs.value);
            break;
        }
        case pypa::AstType::Index:
        {
            auto &idx = reinterpret_cast<const pypa::AstIndex &>(stmt);
            m_result << NodeType::Index;
            SAFE_PARSE_NEXT(idx.value);
            break;
        }
        case pypa::AstType::For:
        {
            auto &loop = reinterpret_cast<const pypa::AstFor &>(stmt);
            m_result << NodeType::ForLoop;
            SAFE_PARSE_NEXT(loop.target);
            SAFE_PARSE_NEXT(loop.iter);
            SAFE_PARSE_NEXT(loop.body);
            break;
        }
        case pypa::AstType::While:
        {
            auto &loop = reinterpret_cast<const pypa::AstWhile &>(stmt);
            m_result << NodeType::WhileLoop;
            SAFE_PARSE_NEXT(loop.test);
            SAFE_PARSE_NEXT(loop.body);
            break;
        }
        case pypa::AstType::AugAssign:
        {
            auto &ass = reinterpret_cast<const pypa::AstAugAssign &>(stmt);
            m_result << NodeType::AugmentedAssign;
            m_result << ass.op;
            SAFE_PARSE_NEXT(ass.target);
            SAFE_PARSE_NEXT(ass.value);
            break;
        }
        case pypa::AstType::ExpressionStatement:
        {
            auto &expr = reinterpret_cast<const pypa::AstExpressionStatement &>(stmt);
            SAFE_PARSE_NEXT(expr.expr);
            break;
        }
        case pypa::AstType::Continue:
        {
            m_result << NodeType::Continue;
            break;
        }
        case pypa::AstType::Break:
        {
            m_result << NodeType::Break;
            break;
        }
        case pypa::AstType::Tuple:
        {
            auto &t = reinterpret_cast<const pypa::AstTuple &>(stmt);
            m_result << NodeType::Tuple;
            m_result << static_cast<uint32_t>(t.elements.size());

            for(auto e : t.elements)
            {
                SAFE_PARSE_NEXT(e);
            }
            break;
        }
        case pypa::AstType::Pass:
        {
            m_result << NodeType::Pass;
            break;
        }
        case pypa::AstType::ListComp:
        {
            auto &c = reinterpret_cast<const pypa::AstListComp &>(stmt);
            m_result << NodeType::ListComp;

            SAFE_PARSE_NEXT(c.element);

            parse_expr_list(c.generators);
            break;
        }
        case pypa::AstType::Comprehension:
        {
            auto &c = reinterpret_cast<const pypa::AstComprehension &>(stmt);
            m_result << NodeType::Comprehension;

            SAFE_PARSE_NEXT(c.target);
            SAFE_PARSE_NEXT(c.iter);
            parse_expr_list(c.ifs);
            break;
        }
        case pypa::AstType::Global:
        {
            auto &c = reinterpret_cast<const pypa::AstGlobal &>(stmt);
            m_result << NodeType::Global;
            // store the name
            m_result << (uint32_t)c.names.size();
            for(auto &nm : c.names)
            {
                SAFE_PARSE_NEXT(nm);
            }
            break;
        }
        case pypa::AstType::FunctionDef:
        {
            auto &c = reinterpret_cast<const pypa::AstFunctionDef &>(stmt);
            m_result << NodeType::FunctionDef;

            // store the name
            SAFE_PARSE_NEXT(c.name);

            // store the stub with start and end markers
            uint32_t dummy_pos = m_result.pos_write();
            // ???? (write 0 and overwrite later??)
            m_result << (uint32_t)0;
            m_result << NodeType::FunctionStart;

            // save arguments
            parse_expr_list(c.args.arguments);
            m_result << NodeType::FunctionStartDefaults;

            // save defaults
            parse_expr_list(c.args.defaults);
            m_result << NodeType::FunctionStartStub;

            // now, we dump the length + whole body of the function,
            // followed by an end marker
            uint32_t size_start = m_result.pos_write();
            SAFE_PARSE_NEXT(c.body);
            uint32_t size_end = m_result.pos_write();

            uint32_t dummy_zero = size_end - size_start;

            m_result.move_to_write(dummy_pos);
            m_result << dummy_zero;
            m_result.move_to_end_write();

            m_result << NodeType::FunctionEnd;
            break;
        }
        default:
            throw std::runtime_error("Unknown statement type!");
        }
    }

    const pypa::AstModulePtr m_ast;

    bitstream m_result; // main execution module
};

bitstream compile_file(const std::string &filename, std::function<void(pypa::Error)> &e)
{
    pypa::AstModulePtr ast;
    pypa::SymbolTablePtr symbols;
    pypa::ParserOptions options;
    options.python3only = true;
    options.printerrors = false;
    options.printdbgerrors = false;
    options.error_handler = e;

    pypa::Lexer lexer(std::unique_ptr<pypa::Reader>{ new pypa::FileBufReader(filename) });

    if(!pypa::parse(lexer, ast, symbols, options))
    {
        throw std::runtime_error("Parsing failed");
    }

    Compiler compiler(ast);
    compiler.run();

    return compiler.get_result();
}

bitstream compile_string(const std::string &code)
{
    pypa::AstModulePtr ast;
    pypa::SymbolTablePtr symbols;
    pypa::ParserOptions options;
    options.python3only = true;
    options.printerrors = true;
    options.printdbgerrors = false;

    pypa::Lexer lexer(std::unique_ptr<pypa::Reader>{ new StringReader(code) });

    if(!pypa::parse(lexer, ast, symbols, options))
    {
        throw std::runtime_error("Parsing failed");
    }

    Compiler compiler(ast);
    compiler.run();

    return compiler.get_result();
}

bitstream compile_string(const std::string &code, std::function<void(pypa::Error)> &e)
{
    pypa::AstModulePtr ast;
    pypa::SymbolTablePtr symbols;
    pypa::ParserOptions options;
    options.python3only = true;
    options.printerrors = false;
    options.printdbgerrors = false;
    options.error_handler = e;

    pypa::Lexer lexer(std::unique_ptr<pypa::Reader>{ new StringReader(code) });

    if(!pypa::parse(lexer, ast, symbols, options))
    {
        throw std::runtime_error("Parsing failed");
    }

    Compiler compiler(ast);
    compiler.run();

    return compiler.get_result();
}

} // namespace cow
