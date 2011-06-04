
        Documentation resources

docs/steps/

This directory contains detailed designs for the actual steps that the convertor
will go through. The layout may vary, but basically each step is covered
separately and invariants, ordering limitations etc are discussed.

docs/program/

This directory contains designs and documentation for the Inferno program, not 
including the transformation steps themselves. This includes search and replace, 
interfaces, program layout, common ultilities etc.

docs/philosophy

This contains general discussions about the approach taken. Some polemic here.

Graphs of trees

Install Graphviz, build Inferno (using just make in trunk/) and run:
./inferno.exe -i<file> -q0 -gi | dotty -      to see tree form of <file>
./inferno.exe -i<file> -q<n> -gi | dotty -    to see <file> after <n> steps
./inferno.exe -gp<n> | dotty -                to see the patterns for step <n>

Looking at the graphs for simple programs and simple steps can be 
fairly informative about what is going on.

Doxygen [WORK IN PROGRESS]

Install Doxygen and run the document generation makefile in docs/doxygen/
to get the generated docs.

Comments in source code

Some parts of the source code are worth reading as documantation. These include:
src/tree/tree.hpp        defines the tree nodes for C/C++ with comments
src/render/graph.hpp     comments explain what nodes look like in graph plots
src/main/inferno.hpp     shows the top level flow of the program

Auto sequencer

The only artifact relating to the intended automatic sequencer 
algorithm is in prototypes/style/style/dialect.hpp/cpp. 
This is a visual studio project.

