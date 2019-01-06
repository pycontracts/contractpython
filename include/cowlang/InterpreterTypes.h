#ifndef ITYPES
#define ITYPES

namespace cow
{

enum class ValueType
{
    None,
    Bool,
    String,
    Integer,
    Float,
    DictItems,
    CppObject,
    Attribute,
    Builtin,
    List,
    Dictionary,
    Iterator,
    Tuple,
    Alias,
    Module,
    Function,
    geo_Vector2,
    Custom
};

enum class CompareOpType
{
    Undefined,
    Equals,
    In,
    Is,
    IsNot,
    Less,
    LessEqual,
    More,
    MoreEqual,
    NotEqual,
    NotIn,
};

enum class BoolOpType
{
    Undefined,
    And,
    Or
};

enum class BinaryOpType
{
    Undefined,
    Add,
    BitAnd,
    BitOr,
    BitXor,
    Div,
    FloorDiv,
    LeftShift,
    Mod,
    Mult,
    Power,
    RightShift,
    Sub
};

enum class UnaryOpType
{
    Undefined,
    Add,
    Invert,
    Not,
    Sub,
};
} // namespace cow

#endif
