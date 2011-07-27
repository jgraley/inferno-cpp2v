LEVEL=..
include ../makefile.common

all : lib/sc_glue.o script/compile.sh script/link.sh

lib/sc_glue.o : src/sc_glue.cpp ../makefile.common
	$(ICC) -c $(OPTIONS) src/sc_glue.cpp -o lib/sc_glue.o -I../systemc/include

script/compile.sh : ../makefile.common makefile
	@echo $(ICC) -c \$$1 -o \$$2 $(OPTIONS) -Iresource/include -Isystemc/include > script/compile.sh
	@chmod 755 script/compile.sh

script/link.sh : ../makefile.common makefile
	@echo $(ICC) \$$1 -o \$$2 resource/lib/sc_glue.o -Lsystemc/lib-linux64/ -lsystemc > script/link.sh
	@chmod 755 script/link.sh

# TODO make a .a file

