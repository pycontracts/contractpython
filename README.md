# Contract Python

Contract Python is a minimal Python interpreter that can be embedded into C++ programs. It features a strict "iron heap" and can impose strict limits for the execution of your program. As a matter of fact, numerous attacks such as the infamous "use after free" are mitigated by design.

## Features
* Supports an imperative subset of Python, currently there is no support for classes
* User defined constraints on memory size and application runtime
* More features than the COW language, i.e., a special set of blockchain functions and the possibility to define and call functions

## Testing
* The code can be tested either using the integrated test suite
* ... or using the "Americal Fuzzer Loop"

## Dependencies
* The Meson Build System
* Google Test

## Building contractpython
```
meson build
cd build
ninja
sudo ninja install
```

## Building contractpython for "American Fuzzer Loop"
```
./make_fuzzer.sh
cd build-fuzzer
ninja all
```
