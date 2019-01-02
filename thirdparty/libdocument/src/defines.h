#include <string>

inline uint8_t from_hex(char c)
{
    switch(c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
    default: return 42;
    }
}

static inline char to_hex(uint8_t val)
{
    //assert(val < 16);

    switch(val)
    {
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
    case 10: return 'A';
    case 11: return 'B';
    case 12: return 'C';
    case 13: return 'D';
    case 14: return 'E';
    case 15: return 'F';
    default: return 'x';
    }
}

inline std::string to_string(int i, int min)
{
    char const digit[] = "0123456789";
    std::string out = "";
    int32_t pos = -1;

    if(i < 0) {
        out = "-";
        pos += 1;
        i *= -1;
    }

    int shifter = i;

    do {
        pos += 1;
        shifter = shifter/10;
    } while(shifter || pos < min-1);

    out.resize(pos+1);

    do{ //Move back, inserting digits as u go
        out[pos] = digit[i%10];
        pos -= 1;
        i = i/10;
    }while(i);

    while(pos >= 0)
    {
        out[pos] = '0';
        pos -= 1;
    }

    return out;
}

