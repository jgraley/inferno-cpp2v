.PHONY: resource

RESOURCE_ARTEFACTS=resource/lib/sc_glue.o resource/script/compile.sh resource/script/link.sh

resource : ${RESOURCE_ARTEFACTS}

resource/lib/sc_glue.o : resource/src/sc_glue.cpp makefile.common
	mkdir -p resource/lib
	$(RCC) -c $(OPTIONS) resource/src/sc_glue.cpp -o resource/lib/sc_glue.o -Isystemc/include

#JSG Due to SystemC bug discussed here: http://www.accellera.org/Discussion_Forums/systemc-forum/archive/msg?list_name=systemc-forum&monthdir=201002&msg=msg00026.html
# I have to use -fpermissive and cannot use -Werror (in $OPTIONS) even though I want to. Apparently a fix is in SystemC 2.3 which is in betaa presently. TODO ensure using SC2.3+ and revert bodge here.
resource/script/compile.sh : makefile.common makefile
	mkdir -p resource/script
	echo $(ICC) -c \$$1 -o \$$2 -fpermissive -Iresource/include -Isystemc/include -fPIC > resource/script/compile.sh
	chmod 755 $@

resource/script/link.sh : makefile.common makefile
	mkdir -p resource/script
	echo $(ICC) \$$1 -o \$$2 resource/lib/sc_glue.o -Lsystemc/lib-linux64/ -lsystemc -no-pie > resource/script/link.sh
	chmod 755 $@

# TODO make a .a file

