#ifndef DEFINES_H
#define DEFINES_H

typedef int i32;
typedef unsigned int u32;
// typedef uint_64 u64;
typedef short i16;
typedef char u8;
typedef float f32;
typedef long long f64;

typedef enum bool {
    false, true
} bool;

/// inlining
#ifdef _MSC_VER
#define SEINLINE __forceinline
#define SENOINLINE __declspec(noinline)
#else
#define SEINLINE static inline
#define SENOINLINE
#endif // inline

#endif // DEFINES_H