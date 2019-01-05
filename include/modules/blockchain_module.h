#ifndef BLOCKCHAIN_MODULE_H
#define BLOCKCHAIN_MODULE_H

#include <cowlang/Module.h>
#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <cowlang/Interpreter.h>
#define COIN 100000000

namespace cow {
    extern std::string txid;
    extern std::string current_block;
    extern std::string previous_block;
    extern uint32_t current_time;
    extern uint32_t previous_time;
    extern std::string sender;
    extern std::string contract_address;
    extern uint64_t value;
    extern uint64_t contract_balance;
}
#define s(i) _s[i]

#define SWAP_BYTE(A, B)                         \
        do {                                    \
                unsigned char swap_temp = *(A); \
                *(A) = *(B);                    \
                *(B) = swap_temp;               \
        } while (0)

namespace cow {
class BlockchainModule : public Module
{
public:
    BlockchainModule(MemoryManager &mem);
    using Module::Module;
    ValuePtr get_member(const std::string &name);

private:
    ValuePtr fourtytwo(Scope& scope);
    ValuePtr get_txid(Scope& scope);
    ValuePtr get_current_block(Scope& scope);
    ValuePtr get_previous_block(Scope& scope);
    ValuePtr get_sender(Scope& scope);
    ValuePtr get_contractaddress(Scope& scope);
    ValuePtr get_current_time(Scope& scope);
    ValuePtr get_previous_time(Scope& scope);
    ValuePtr get_value(Scope& scope);
    ValuePtr get_random(Scope& scope);
    ValuePtr get_contract_balance(Scope& scope);
    std::map<std::string, ValuePtr> function_map;

    void seed();

        // RC$
        /* Retrieves one octet from the array BYTES, which is N_BYTES in
       size, starting at an offset of OCTET_IDX octets.  BYTES is
       treated as a circular array, so that accesses past the first
       N_BYTES bytes wrap around to the beginning. */
    static unsigned char
    get_octet (const unsigned char *bytes_, size_t n_bytes, size_t octet_idx)
    {
      const unsigned char *bytes = bytes_;
        return bytes[octet_idx % n_bytes];
    };

    /* Seeds the pseudo-random number based on the SIZE bytes in
       KEY.  At most the first 2048 bits in KEY are used. */
    void
    prng_seed_bytes (const unsigned char *key, size_t size)
    {
      int i, j;


      for (i = j = 0; i < 256; i++)
        {
          j = (j + s(i) + get_octet (key, size, i)) & 255;
          SWAP_BYTE (_s + i, _s + j);
        }

      s_i = s_j = 0;
      seeded = 1;
    };

    /* Returns a pseudo-random integer in the range [0, 255]. */
    unsigned char
    prng_get_octet (void)
    {
      s_i = (s_i + 1) & 255;
      s_j = (s_j + s(s_i)) & 255;
      SWAP_BYTE (_s + s_i, _s + s_j);

      return s((s(s_i) + s(s_j)) & 255);
    };

    /* Returns a pseudo-random integer in the range [0, UCHAR_MAX]. */
    unsigned char
    prng_get_byte (void)
    {
      unsigned byte;
      byte = prng_get_octet ();
      return byte;
    };

    /* Fills BUF with SIZE pseudo-random bytes. */
    void
    prng_get_bytes (unsigned char* buf_, size_t size)
    {
      unsigned char *buf;
      for (buf = buf_; size-- > 0; buf++)
        *buf = prng_get_byte ();
    };



    /* Returns a pseudo-random unsigned int in the range [0,
       UINT_MAX]. */
    unsigned
    prng_get_uint (void)
    {
      unsigned uint;
      size_t bits;

      uint = prng_get_octet ();
      for (bits = 1; bits < sizeof uint; bits += 1)
        uint = (uint << 8) | prng_get_octet ();
      return uint;
    };

    unsigned char _s[256];
    int s_i, s_j;
    int seeded;

};


void register_blockchain_module(cow::Interpreter& i);
};
#endif /* end of include guard: BLOCKCHAIN_MODULE_H */
