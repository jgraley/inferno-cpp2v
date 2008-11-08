LEVEL=..
include ../makefile.common

INFERNO_SOURCES := inferno.cpp tree/tree.cpp common/refcount.cpp common/read_args.cpp common/trace.cpp parse/inferno_minimal_action.cpp common/type_info.cpp

inferno.a :
	$(ICC) -c $(OPTIONS) $(INFERNO_SOURCES) -ggdb
	$(AR) -r inferno.a *.o
	rm -r *.o
	
.PHONY: all clang llvm
