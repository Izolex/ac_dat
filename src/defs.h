#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


#define END_OF_TEXT 3
#define TRIE_POOL_INFO 0
#define TRIE_POOL_START 1


#define unused(...) (void)(0, __VA_ARGS__)
#define error(msg) do { perror((msg)); exit(EXIT_FAILURE); } while (0)

#ifdef VERBOSE
#define log(msg) puts((msg))
#else
#define log(msg)
#endif

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define prefetch(addr, rw, locality) __builtin_prefetch((addr), (rw), (locality))
#define add_overflow(a, b, result) __builtin_add_overflow((a), (b), (result))
#else
#define likely(x) (x)
#define unlikely(x) (x)
#define prefetch(addr, rw, locality) (void)
#define add_overflow(a, b, result) ({(*result) = (a) + (b); false;})
#endif

#endif
