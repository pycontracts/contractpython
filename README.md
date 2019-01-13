# Contract Python

Contract Python is a minimal Python interpreter that can be embedded into C++ programs. It features a strict "iron heap" and can impose strict limits for the execution of your program. As a matter of fact, numerous attacks such as the infamous "use after free" are mitigated by design.

<img src="https://raw.githubusercontent.com/pycontracts/contractpython/master/screen_cryptopython.png"></img>

## Features
* Contract python is based on the Python 3 flavor
* Supports an imperative subset of Python, currently there is no support for classes
* User defined constraints on memory size and application runtime
* More features than the leightweight COW language such as the possibility to define and call functions
* Python files can be compiled into compressed bytecode (snappy), which is ideal for space-constrained blockchains
* The interpreter comes with important blockchain functions integrated, such as access to the current blockchain's meta data and a deterministic cryptographically-secure random number generator

## Testing
* The code can be tested either using the integrated test suite
* ... or using the "American Fuzzer Loop"

## Dependencies
* The Meson Build System
* Google Test
* afl (in case you want to fuzzy-test)

## Building contractpython
```
meson build
cd build
ninja all
```

## Building contractpython for "American Fuzzer Loop"
```
./make_fuzzer.sh
cd build-fuzzer
ninja all
```
