#pragma once

namespace cow
{

enum class NodeType
{
    Pass,
    StatementList,
    Name,
    Assign,
    Return,
    String,
    Compare,
    Dictionary,
    Integer,
    IfElse,
    If,
    Call,
    Attribute,
    UnaryOp,
    BinaryOp,
    BoolOp,
    List,
    Tuple,
    Subscript,
    Index,
    ForLoop,
    WhileLoop,
    AugmentedAssign,
    Continue,
    Break,
    Import,
    ImportFrom,
    Alias
};

}
