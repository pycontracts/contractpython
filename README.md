# The Cow Language

[![Build Status](https://travis-ci.com/kaimast/cowlang.svg?branch=master)](https://travis-ci.com/kaimast/cowlang)

Cow is a minimal Python interpreter that can be embedded into C++ programs.

## Features
* Supports an imperative subset of Python, currently there is no support for classes
* Custom C++ objects can be hooked into the interpreter 
* User defined constraints on memory size and application runtime

## Dependencies
* The Meson Build System
* Google Test
* libpypa (https://github.com/vinzenz/libpypa): A python parser, currently used in the compiler 
* libdocument (https://github.com/kaimast/libdocument): A super fast JSON library
       
## Building cowlang
```
meson build
cd build
ninja
sudo ninja install
```
