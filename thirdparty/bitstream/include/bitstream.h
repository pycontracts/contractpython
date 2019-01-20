#pragma once

#include "pypa/ast/ast.hh"
#include <cowlang/InterpreterTypes.h>
#include <cowlang/NodeType.h>
#include <fstream>
#include <istream>
#include <math.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>

using namespace cow;
using namespace pypa;


namespace EndianSwapper
{
class SwapByteBase
{
public:
    static bool ShouldSwap()
    {
        static const uint16_t swapTest = 1;
        return (*((char *)&swapTest) == 1);
    }

    static void SwapBytes(uint8_t &v1, uint8_t &v2)
    {
        uint8_t tmp = v1;
        v1 = v2;
        v2 = tmp;
    }
};

template <class T, int S> class SwapByte : public SwapByteBase
{
public:
    static T Swap(T v) { throw std::runtime_error("SwapByte used incorrectly"); }
};

template <class T> class SwapByte<T, 1> : public SwapByteBase
{
public:
    static T Swap(T v) { return v; }
};

template <class T> class SwapByte<T, 2> : public SwapByteBase
{
public:
    static T Swap(T v)
    {
        if(ShouldSwap())
            return (v >> 8) | (v << 8);
        return v;
    }
};

template <class T> class SwapByte<T, 4> : public SwapByteBase
{
public:
    static T Swap(T v)
    {
        if(ShouldSwap())
        {
            return (SwapByte<uint16_t, 2>::Swap((uint32_t)v & 0xffff) << 16) |
                   (SwapByte<uint16_t, 2>::Swap(((uint32_t)v & 0xffff0000) >> 16));
        }
        return v;
    }
};

template <class T> class SwapByte<T, 8> : public SwapByteBase
{
public:
    static T Swap(T v)
    {
        if(ShouldSwap())
            return (((uint64_t)SwapByte<uint32_t, 4>::Swap((uint32_t)(v & 0xffffffffull))) << 32) |
                   (SwapByte<uint32_t, 4>::Swap((uint32_t)(v >> 32)));
        return v;
    }
};

template <> class SwapByte<float, 4> : public SwapByteBase
{
public:
    static float Swap(float v)
    {
        union {
            float f;
            uint8_t c[4];
        };
        f = v;
        if(ShouldSwap())
        {
            SwapBytes(c[0], c[3]);
            SwapBytes(c[1], c[2]);
        }
        return f;
    }
};

template <> class SwapByte<double, 8> : public SwapByteBase
{
public:
    static double Swap(double v)
    {
        union {
            double f;
            uint8_t c[8];
        };
        f = v;
        if(ShouldSwap())
        {
            SwapBytes(c[0], c[7]);
            SwapBytes(c[1], c[6]);
            SwapBytes(c[2], c[5]);
            SwapBytes(c[3], c[4]);
        }
        return f;
    }
};
}; // namespace EndianSwapper


#define SERIALIZER_FOR_UINT32TYPE(type)                                \
    inline bitstream &operator>>(bitstream &stream, type &v)           \
    {                                                                  \
        uint32_t typus = 0;                                            \
        stream.bytecode.read((char *)&typus, sizeof(uint32_t));        \
        if(stream.bytecode)                                            \
        {                                                              \
            typus = stream.Swap(typus);                                \
            v = (type)typus;                                           \
            return stream;                                             \
        }                                                              \
        else                                                           \
            throw std::runtime_error("Unexpected EOF");                \
    }                                                                  \
    inline bitstream &operator&(bitstream &stream, type &v)            \
    {                                                                  \
        uint32_t typus = 0;                                            \
        std::stringstream::pos_type pos = stream.bytecode.tellg();     \
        stream.bytecode.read((char *)&typus, sizeof(uint32_t));        \
        if(stream.bytecode)                                            \
        {                                                              \
            typus = stream.Swap(typus);                                \
            v = (type)typus;                                           \
            stream.bytecode.seekg(pos);                                \
            return stream;                                             \
        }                                                              \
        else                                                           \
            throw std::runtime_error("Unexpected EOF");                \
    }                                                                  \
    inline bitstream &operator<<(bitstream &stream, type v)            \
    {                                                                  \
        uint32_t typus = (uint32_t)v;                                  \
        typus = stream.Swap(typus);                                    \
        stream.bytecode.write((const char *)&typus, sizeof(uint32_t)); \
        return stream;                                                 \
    }


#define SERIALIZER_FOR_POD(type)                        \
    inline bitstream &operator>>(type &v)               \
    {                                                   \
        bytecode.read((char *)&v, sizeof(type));        \
        if(bytecode)                                    \
        {                                               \
            v = Swap(v);                                \
            return *this;                               \
        }                                               \
        else                                            \
            throw std::runtime_error("Unexpected EOF"); \
    }                                                   \
    inline bitstream &operator<<(type v)                \
    {                                                   \
        v = Swap(v);                                    \
        bytecode.write((char *)&v, sizeof(type));       \
        return *this;                                   \
    }

class bitstream
{
public:
    template <class T> T Swap(const T &v) const
    {
        return EndianSwapper::SwapByte<T, sizeof(T)>::Swap(v);
    }
    std::stringstream bytecode;

    bitstream() : bytecode() {}

    bitstream(bitstream &&b) : bytecode(b.bytecode.str()) {}

    bitstream(std::string &b) : bytecode(b) {}

    bitstream(const bitstream &b) : bytecode(b.bytecode.str()) {}

    void assign(uint8_t *buffer, uint32_t len)
    {
        bytecode.clear();
        bytecode.write((const char *)buffer, len);
    }

    const std::string store() const { return bytecode.str(); }

    std::string Read(uint32_t count)
    {
        std::string result(count, ' ');
        bytecode.read(&result[0], count);
        return result;
    }

    uint32_t pos()
    {
        std::stringstream::pos_type pos = bytecode.tellg();
        return (uint32_t)pos;
    }


    uint32_t pos_write()
    {
        std::stringstream::pos_type pos = bytecode.tellp();
        return (uint32_t)pos;
    }

    bool move_to(uint32_t pos)
    {
        bytecode.seekg(pos, bytecode.beg);
        return true;
    }

    bool move_to_write(uint32_t pos)
    {
        bytecode.seekp(pos, bytecode.beg);
        return true;
    }

    bool move_to_end()
    {
        bytecode.seekg(0, bytecode.end);
        return true;
    }

    bool move_to_end_write()
    {
        bytecode.seekp(0, bytecode.end);
        return true;
    }

    SERIALIZER_FOR_POD(bool)
    SERIALIZER_FOR_POD(char)
    SERIALIZER_FOR_POD(unsigned char)
    SERIALIZER_FOR_POD(short)
    SERIALIZER_FOR_POD(unsigned short)
    SERIALIZER_FOR_POD(int)
    SERIALIZER_FOR_POD(unsigned int)
    SERIALIZER_FOR_POD(long)
    SERIALIZER_FOR_POD(unsigned long)
    SERIALIZER_FOR_POD(long long)
    SERIALIZER_FOR_POD(unsigned long long)
    SERIALIZER_FOR_POD(float)
    SERIALIZER_FOR_POD(double)

    bitstream &operator>>(std::string &data)
    {
        data = "";
        uint32_t length;
        bytecode.read((char *)&length, sizeof(uint32_t));
        if(!bytecode)
            throw std::runtime_error("Unexpected EOF");
        length = Swap(length);
        if(length > 0)
        {
            data.resize(length);
            bytecode.read(&data[0], length);
            if(!bytecode)
                throw std::runtime_error("Unexpected EOF");
        }

        return *this;
    }

    bitstream &operator<<(std::string data)
    {
        uint32_t length = (uint32_t)data.size();

        length = Swap(length);
        bytecode.write((const char *)&length, sizeof(uint32_t));
        if(length > 0)
        {
            bytecode.write(data.c_str(), data.size());
        }

        return *this;
    }

    bitstream &operator<<(char *_data)
    {
        std::string data(_data);
        uint32_t length = Swap(data.size());
        bytecode.write((const char *)&length, sizeof(uint32_t));
        if(length > 0)
        {
            bytecode.write(data.c_str(), length);
        }

        return *this;
    }
};

SERIALIZER_FOR_UINT32TYPE(NodeType)
SERIALIZER_FOR_UINT32TYPE(ValueType)
SERIALIZER_FOR_UINT32TYPE(BinaryOpType)
SERIALIZER_FOR_UINT32TYPE(BoolOpType)
SERIALIZER_FOR_UINT32TYPE(UnaryOpType)
SERIALIZER_FOR_UINT32TYPE(CompareOpType)
SERIALIZER_FOR_UINT32TYPE(AstBinOpType)
SERIALIZER_FOR_UINT32TYPE(AstBoolOpType)
SERIALIZER_FOR_UINT32TYPE(AstUnaryOpType)
SERIALIZER_FOR_UINT32TYPE(AstCompareOpType)
