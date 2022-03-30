#ifndef SESTRING_H
#define SESTRING_H

#include "sedefines.h"

#define SESTRING_MAX_SIZE 0xffffffff // from <limits.h> for UINT_MAX

typedef struct SE_String {
    char *buffer;
    u32 size;
} SE_String;

/// Allocates memory
void sestring_init(SE_String *string, const char *buffer);
/// Frees potentially allocated memory
void sestring_deinit(SE_String *string);
/// Duplicates string1's content to string2's content.
void sestring_duplicate(SE_String *string1, SE_String *string2);
/// Appends string's buffer. Potentially allocates more memory
void sestring_append(SE_String *string, const char *buffer);
/// Appends string's buffer by going through buffer 'length' amount
void sestring_append_length(SE_String *string, const char *buffer, u32 length);
/// Find the last index of a given character in the string.
/// Returns SESTRING_MAX_SIZE if not found
u32 sestring_lastof(SE_String *string, const char letter);
#endif // SESTRING_H