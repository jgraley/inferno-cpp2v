.PHONY: resource

RESOURCE_INCLUDES = resource/include/isystemc.h resource/include/systemc_extensions.h 
RESOURCE_ARTEFACTS = resource/lib/systemc_extensions.o

resource : ${RESOURCE_ARTEFACTS}

resource/lib/systemc_extensions.o : resource/src/systemc_extensions.cpp makefile.common $(RESOURCE_INCLUDES)
	mkdir -p resource/lib
	$(RCC) -c $(OPTIONS) $(SC_OPTIONS) resource/src/systemc_extensions.cpp -o resource/lib/systemc_extensions.o -Isystemc/include -Iresource/include

RESOURCE_SCRIPTS = resource/script/compile.sh resource/script/link.sh resource/script/exec.sh
RESOURCE_DEPS = $(RESOURCE_INCLUDES) $(RESOURCE_SCRIPTS) $(RESOURCE_ARTEFACTS) resource/makefile

# TODO make a .a file

