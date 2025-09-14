
Welcome to inferno-cpp2v

Directories are organised thusly:

inferno-cpp2v/trunk (this directory)
    Everything lives in this directory. The idea of the "trunk" subdirectory
    is that branches can go in other subdirectories within inferno-synth/
    
src/
    This contains all the Inferno source code, most of which is organised into 
    further subdirectories under src/

docs/
    This contains documentation about inferno, its design and its steps. There 
    is more info about the docs in docs/readme.txt

test/
    This contains all the test harnesses and test input. Test results go in
    here too.

prototype/
    This is for standalone trial programs; the contents herein should not
    be compiled into the inferno executable.

llvm/
    A VERY old Clang/LLVM snapshot, with updates for modern compilers. Clang is 
    in llvm/tools/clang/. I think the revision is 61726 (Subversion) on 
    Sourceforge. 

eclipse/
    Inferno also supports the Eclipse IDE (with CDT C++ tools plugin installed). 
    There is a "standard makefile" C++ project in inferno-cpp2v/trunk/eclipse 
    Note that IDE integration is not officially supported for Inferno.

resources/
    Code needed to get SystemC to work for running test vectors etc, both in Inferno 
    and when compiling natively.

How to build:

Inferno works with gcc/g++ 13.3.0

On a Debian/Ubuntu system, you need approximately the following apt packages:
    
make
build-essential
bison         (parser generator)
graphviz      (only if you want to look at pretty pictures)
doxygen       (automated class documentation)
binutils-gold (installing this makes gcc use the gold linker which speeds up builds)
valgrind      (profiling and other handy stuff)
kcachegrind   (GUI for profiles)

You also need to install SystemC (version 3.1.0) and set up a symlink in this directory pointing to the installation. See docs/program/systemc_installation_notes.txt for more info.

And also RE/flex lexer generator (with unicode support):
 - go to https://github.com/Genivia/RE-flex/releases/tag/v6.0.0
 - download the source code .zip and unzip it and go in
 - ./clean.sh
 - ./build.sh
 - symlink `reflex` in inferno top level to the unzipped dir as with SystemC

Now type:

make all

This will:
 - clean up any old binaries       (make clean)
 - compile inferno and clang       (make inferno.exe or just make)
 - build the doxygen documentation (make docs)
 - run the tests                   (make test)

It should report "ALL TESTS PASSED". 

To run the tests by hand go into test/ and use runtests.sh. Look at the 
script for usage instructions. If you want to see a graph, try e.g.

./inferno.exe -itest/examples/add.cpp -gi | dotty -

To make git work with a GitHub access token:

git remote set-url origin https://<user>:<token>@github.com/jgraley/inferno-cpp2v.git
