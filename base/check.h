#ifndef BASE_CHECK_H_
#define BASE_CHECK_H_

#include <stdlib.h>
#include <cstdio>

#include <execinfo.h>

static inline void printBacktrace() {
  void *stackBuffer[64]; 
  int numAddresses = backtrace((void**) &stackBuffer, 64); 
  char **addresses = backtrace_symbols(stackBuffer, numAddresses); 
  for( int i = 0 ; i < numAddresses ; ++i ) { 
    fprintf(stderr, "[%2d]: %s\n", i, addresses[i]); 
  } 
  free(addresses);
} 

#define CHECK(cond)                                   \
 do {if(!(cond)) {                                    \
  fprintf(stderr, "%s:%i\n", __FILE__, __LINE__);     \
  fprintf(stderr, "Error: " #cond "\n");              \
 }} while(0)

#define CHECK_EQ(A, B) \
 CHECK((A) == (B))

#define CHECK_NE(A, B) \
 CHECK((A) != (B))

#define MCHECK(cond, ...) \
 do {if(!(cond)){printf(__VA_ARGS__); puts(""); exit(1);}} while(0)

#define MCHECK_EQ(A, B, ...) \
 do {if((A) != (B)){printf(__VA_ARGS__); puts(""); exit(1);}} while(0)

#define MCHECK_NE(A, B, ...) \
 do {if((A) == (B)){printf(__VA_ARGS__); puts(""); exit(1);}} while(0)

#define NOTREACHED() \
 MCHECK(false, "Unreached code point")

#endif  // BASE_CHECK_H_
