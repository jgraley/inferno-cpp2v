
Welcome to the inferno-synth subversion module.

Directories are organised thusly:

inferno-cpp2v/trunk (this directory)
    Everything lives in this directory. The idea of the "trunk" subdirectory
    is that branches can go in other subdirectories within inferno-synth/
    
src/
    This contains all the Inferno source code, most of which is organised into 
    further subdirectories under src/

test/
    This contains all the test harnesses and test input. Test results go in
    here too.

prototype/
    This is for standalone trial programs; the contents herein should not
    be compiled into the inferno executable.

llvm/
    If you've done "make get_libs", a copy of llvm will go here. Clang goes 
    in llvm/tools/clang/

patches/
    Patches applied to the llvm/clang checkout by make get_libs.

eclipse/
    Inferno also supports the Eclipse IDE (with CDT C++ tools plugin installed). 
    There is a "standard makefile" C++ project in inferno-cpp2v/trunk/eclipse 
    Note that IDE integration is not officially supported for Inferno.

How to build:

On a Debian/Ubuntu system, you need approximately the following apt packages:
    
subversion
make
patch
g++
flex
bison
libboost-dev
libbbost-thread-dev
graphviz   [only if you want to look at pretty pictures] 
indent     [makes reading inferno C output easier]

First you have to get llvm and clang out of the llvm project's subversion
repository. To do this type

make get_libs

Now you can build and run the self-tests using

make test

It should report "ALL TESTS PASSED". To build without testing just go

make

To run the tests by hand go into test/ and use runtests.sh. Look at the 
script for usage instructions. If you want to see a graph, try e.g.

./inferno.exe -itest/examples/add.cpp -gi | dotty -

