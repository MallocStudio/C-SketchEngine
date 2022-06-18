#ifndef SESTRING_H
#define SESTRING_H

#include "sedefines.h"

#define SESTRING_MAX_SIZE 0xffffffff // from <limits.h> for UINT_MAX

typedef struct SE_String {
    char *buffer;
    u32 size;
    u32 capacity;
} SE_String;

/// Allocates memory
void sestring_init(SE_String *string_buffer, const char *buffer);
/// Frees potentially allocated memory
void sestring_deinit(SE_String *string_buffer);
/// Duplicates string1's content to string2's content.
void sestring_duplicate(SE_String *src, SE_String *dest);
/// Appends string's buffer. Potentially allocates more memory
void sestring_append(SE_String *string_buffer, const char *buffer);
/// Appends string's buffer by going through buffer 'length' amount
void sestring_append_length(SE_String *string_buffer, const char *buffer, u32 length);
/// Find the last index of a given character in the string.
/// Returns SESTRING_MAX_SIZE if not found
u32 sestring_lastof(SE_String *string_buffer, const char letter);
/// Clears the buffer (deinits then inits)
void sestring_clear(SE_String *string_buffer);
/// Deletes from the end of the buffer. Does not modify memory capacity
void sestring_delete_from_end(SE_String *string_buffer, u32 amount);

#define SESTRING_MAX_NUM_OF_DIGITS 10 // only 10 digits are allowed
/// Tries to parse out a floating point out of the given string.
/// Decimal place is used to determine how to convert a number (say 1234) to
/// a floating point number (in the case of decimal_place = 100 -> 1234 / 100 = 12.34)
f32 sestring_as_f32(SE_String *string_buffer);

/// returns true if the strings have the same value stored
b8 sestring_compare(const SE_String *string1, const SE_String *string2);
#endif // SESTRING_H