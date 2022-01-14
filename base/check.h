#ifndef BASE_CHECK_H_
#define BASE_CHECK_H_

#include <stdlib.h>
#include <cstdio>

#define CHECK(cond)                               \
 do {if(!(cond)) {                                \
  fprintf(stderr, "%s:%i\n", __FILE__, __LINE__); \
  fprintf(stderr, "Error: " #cond "\n");          \
  exit(1);                                        \
 }} while(0)

#define CHECK_EQ(A, B) \
 do {if((A) != (B)) exit(1);} while(0)

#define CHECK_NE(A, B) \
 do {if((A) == (B)) exit(1);} while(0)

#define MCHECK(cond, ...) \
 do {if(!(cond)){printf(__VA_ARGS__); puts(""); exit(1);}} while(0)

#define MCHECK_EQ(A, B, ...) \
 do {if((A) != (B)){printf(__VA_ARGS__); puts(""); exit(1);}} while(0)

#define MCHECK_NE(A, B, ...) \
 do {if((A) == (B)){printf(__VA_ARGS__); puts(""); exit(1);}} while(0)

#define NOTREACHED() \
 MCHECK(false, "Unreached code point")

#endif  // BASE_CHECK_H_
