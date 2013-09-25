
Welcome to the inferno-synth subversion module.

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
    If you've done "make get_libs", a copy of llvm will go here. Clang goes 
    in llvm/tools/clang/

patches/
    Patches applied to the llvm/clang checkout by make get_libs.

eclipse/
    Inferno also supports the Eclipse IDE (with CDT C++ tools plugin installed). 
    There is a "standard makefile" C++ project in inferno-cpp2v/trunk/eclipse 
    Note that IDE integration is not officially supported for Inferno.


How to check out:

Inferno is hosted at Sourceforge in a Subversion repository. You can get a modifiable checkout using:

svn checkout svn+ssh://jgraley@svn.code.sf.net/p/inferno-cpp2v/code/trunk inferno-cpp2v-code 

How to build:

On a Debian/Ubuntu system, you need approximately the following apt packages:
    
subversion
make
patch
g++
flex
bison
libboost-dev
libboost-thread-dev
graphviz      (only if you want to look at pretty pictures)
indent        (makes reading inferno C output easier)
doxygen       (automated class documentation)
imagemagick   (only for web page generation TODO don't need this now using svg)
binutils-gold (installing this makes gcc use the gold linker which speeds up builds)

You also need to install SystemC (version 2.2.0) and set up a symlink in this directory pointing to the installation. See docs/program/systemc_installation_notes.txt for more info.

Now type:

make all

This will:
 - clean up any old binaries       (make clean)
 - download LLVM and clang         (make get_libs)
 - compile inferno and clang       (make inferno.exe or just make)
 - build the doxygen documentation (make docs)
 - run the tests                   (make test)

It should report "ALL TESTS PASSED". 

To run the tests by hand go into test/ and use runtests.sh. Look at the 
script for usage instructions. If you want to see a graph, try e.g.

./inferno.exe -itest/examples/add.cpp -gi | dotty -

