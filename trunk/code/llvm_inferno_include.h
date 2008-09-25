
#ifndef LLVM_HEADER_H
#define LLVM_HEADER_H

#include <stdio.h>

#ifdef assert
#undef assert
#endif

#define assert( X ) do { if(!(X)) { \
    fprintf(stderr, "Assertion failed %s:%d\n%s\n", __FILE__, __LINE__, #X); \
    *((int *)-1)=0; \
} } while(0)

#define _assert( X ) do { if(!(X)) { \
    fprintf(stderr, "Assertion failed %s:%d _\n%s\n", __FILE__, __LINE__, #X); \
    *((int *)-1)=0; \
} } while(0)

#endif

