LEVEL=..
include ../makefile.common

all : lib/sc_glue.o script/compile.sh script/link.sh

lib/sc_glue.o : src/sc_glue.cpp ../makefile.common
	@mkdir -p lib
	$(ICC) -c $(OPTIONS) src/sc_glue.cpp -o lib/sc_glue.o -I../systemc/include

#JSG Due to SystemC bug discussed here: http://www.accellera.org/Discussion_Forums/systemc-forum/archive/msg?list_name=systemc-forum&monthdir=201002&msg=msg00026.html
# I have to use -fpermissive and cannot use -Werror (in $OPTIONS) even though I want to. Apparently a fix is in SystemC 2.3 which is in betaa presently. TODO ensure using SC2.3+ and revert bodge here.
script/compile.sh : ../makefile.common makefile
	@mkdir -p script
	@echo $(ICC) -c \$$1 -o \$$2 -fpermissive -Iresource/include -Isystemc/include > script/compile.sh
	@chmod 755 script/compile.sh

script/link.sh : ../makefile.common makefile
	@mkdir -p script
	@echo $(ICC) \$$1 -o \$$2 resource/lib/sc_glue.o -Lsystemc/lib-linux64/ -lsystemc > script/link.sh
	@chmod 755 script/link.sh

# TODO make a .a file

