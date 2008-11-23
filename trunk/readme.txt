
Welcome to the inferno-synth subversion module.

Directories are organised thusly:

inferno-synth/trunk (this directory)
    Everything lives in this directory. The idea of the "trunk" subdirectory
    is that branches can go in other subdirectories within inferno-synth/
    From now on, all the paths will be from this directory.

code/
    This contains all the sources, much of which is organised into further
    subdirectories under code/

test/
    This contains all the test harnesses and test input. Test results go in
    here too.

prototype/
    This is for standalone trial programs; the contents herein should not
    be compiled into inferno executables.

llvm/
    If you've done "make get_libs", a copy of llvm will go here. Clang goes 
    in llvm/tools/clang/

How to build:

First you have to get llvm and clang out of the llvm project's subversion
repository. To do this type

make get_libs

Now you can build and run the self-tests using

make tests

It should report "ALL TESTS PASSED". To build without testing just go

make

To run the tests by hand go into test/ and use runtests.sh. Look at the 
script for usage instructions.
