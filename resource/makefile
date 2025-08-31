.PHONY: resource

RESOURCE_ARTEFACTS = resource/lib/sc_glue.o

resource : ${RESOURCE_ARTEFACTS}

resource/lib/sc_glue.o : resource/src/sc_glue.cpp makefile.common
	mkdir -p resource/lib
	$(RCC) -c $(OPTIONS) $(SC_OPTIONS) resource/src/sc_glue.cpp -o resource/lib/sc_glue.o -Isystemc/include

RESOURCE_SCRIPTS = resource/script/compile.sh resource/script/link.sh resource/script/exec.sh
RESOURCE_DEPS = $(RESOURCE_SCRIPTS) $(RESOURCE_ARTEFACTS) resource/makefile

# TODO make a .a file

