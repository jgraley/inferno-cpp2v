.PHONY: resource

RESOURCE_ARTEFACTS=resource/lib/sc_glue.o

resource : ${RESOURCE_ARTEFACTS}

resource/lib/sc_glue.o : resource/src/sc_glue.cpp makefile.common
	mkdir -p resource/lib
	$(RCC) -c $(OPTIONS) $(SC_OPTIONS) resource/src/sc_glue.cpp -o resource/lib/sc_glue.o -Isystemc/include

# TODO make a .a file

