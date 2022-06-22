#include "sestring.h"
#include "memory.h"
#include "semath.h"
void sestring_init(SE_String *sestring, const char *buffer) {
    if (buffer != NULL) {
        sestring->size = SDL_strlen(buffer);
        sestring->capacity = sestring->size + 1; // +1 to accomidate '\0'

        sestring->buffer = malloc(sizeof(char) * sestring->capacity);

        for (u32 i = 0; i < sestring->size+1; ++i) {
            sestring->buffer[i] = buffer[i];
        }
    } else {
        sestring->size = 0;
        sestring->capacity = 0;
        sestring->buffer = NULL;
    }
}

void sestring_init_i32(SE_String *sestring, i32 value) {
    char buffer[32] = {0};
    SDL_snprintf(buffer, 32, "%i", value);

    sestring->size = SDL_strlen(buffer);
    sestring->capacity = sestring->size + 1; // +1 to accomidate '\0'

    sestring->buffer = malloc(sizeof(char) * sestring->capacity);

    for (u32 i = 0; i < sestring->size+1; ++i) {
        sestring->buffer[i] = buffer[i];
    }
}

void sestring_init_f32(SE_String *sestring, f32 value) {
    char buffer[32] = {0};
    SDL_snprintf(buffer, 32, "%f", value);

    sestring->size = SDL_strlen(buffer);
    sestring->capacity = sestring->size + 1; // +1 to accomidate '\0'

    sestring->buffer = malloc(sizeof(char) * sestring->capacity);

    for (u32 i = 0; i < sestring->size+1; ++i) {
        sestring->buffer[i] = buffer[i];
    }
}

void sestring_deinit(SE_String *sestring) {
    if (sestring->buffer != NULL) {
        free(sestring->buffer);
        sestring->buffer = NULL;
        sestring->size = 0;
        sestring->capacity = 0;
    }
}

void sestring_duplicate(SE_String *string1, SE_String *string2) {
    sestring_deinit(string2);
    if (string1->buffer != NULL) {
        sestring_init(string2, string1->buffer);
    }
}

void sestring_append(SE_String *sestring, const char *buffer) {
    sestring_append_length(sestring, buffer, SDL_strlen(buffer));
}

void sestring_append_length(SE_String *sestring, const char *src, u32 length) {
    if (sestring->buffer == NULL) {
        // if this sestring is empty, initialise it with the given src
        sestring_init(sestring, src);
    } else {
        u32 start_index = sestring->size;

        sestring->size += length;

        if (sestring->size > sestring->capacity) {
            sestring->capacity = sestring->size+1; // +1 to accomidate '\0'
            sestring->buffer = realloc(sestring->buffer, sizeof(char) * sestring->capacity);
        }

        for (u32 i = start_index; i < sestring->size+1; ++i) { // +1 to inlcude '\0'
            sestring->buffer[i] = src[i - start_index];
        }
    }
}

u32 sestring_lastof(SE_String *sestring, const char letter) {
    if (sestring->buffer == NULL || sestring->size == 0) {
        return SESTRING_MAX_SIZE;
    }

    u32 result = SESTRING_MAX_SIZE;
    for (u32 i = sestring->size; i > 0; --i) {
        if (sestring->buffer[i] == letter) {
            result = i;
            break;
        }
    }
    return result;
}

void sestring_clear(SE_String *sestring) {
    if (sestring->buffer == NULL || sestring->size == 0) {
        return;
    }
    sestring->size = 0;
}

void sestring_delete_from_end(SE_String *sestring, u32 amount) {
    if (sestring->size >= amount) {
        sestring->size -= amount;
        sestring->buffer[sestring->size] = '\0';
    }
}

f32 sestring_as_f32(SE_String *sestring) {
    f32 result = 0; // default
    if (sestring->buffer == NULL || sestring->size == 0) return result;

    u32 decimal_place = 1;
    b8 decimal_place_set = false;

    i32 digits[SESTRING_MAX_NUM_OF_DIGITS] = {0};
    i32 digits_count = 0;
    i32 sign = 1;

    for (i32 i = sestring->size; i >= 0 ; --i) {
        char potential_number = sestring->buffer[i];
        // printf("%c is %i\n", potential_number, (i32) potential_number);

        // '0' is 48 and '9' is 57
        if (potential_number >= '0' && potential_number <= '9') {
            digits[digits_count] = (i32) potential_number - (i32) '0';
            digits_count++;

            if (digits_count > SESTRING_MAX_NUM_OF_DIGITS) {
                digits_count = SESTRING_MAX_NUM_OF_DIGITS;
                break;
            }
        }

        if (potential_number == '.' && decimal_place_set == false) {
            decimal_place_set = true;
            decimal_place = semath_power(10, digits_count);
        }

        if (potential_number == '-' && i == 0) {
            sign = -1;
        }
    }

    for (u32 i = 0; i < digits_count; ++i) {
        i32 power = (i32)semath_power(10, i);
        result += digits[i] * power;
    }

    result /= (f32)decimal_place;
    result *= sign;
    return result;
}

b8 sestring_compare(const SE_String *string1, const SE_String *string2) {
    if (string1->size == 0 && string2->size == 0) return true;
    if (string1->size == 0 || string2->size == 0) return false;
    if (string1->size != string2->size) return false;

    for (u32 i = 0; i < string1->size; ++i) {
        if (string1->buffer[i] != string2->buffer[i]) {
            return false;
        }
    }

    return true;
}

b8 sestring_replace_space_with_underscore(SE_String *string_buffer) {
    se_assert(string_buffer->buffer);
    b8 found = false;
    for (u32 i = 0; i < string_buffer->size; ++i) {
        if (string_buffer->buffer[i] == ' ') {
            found = true;
            string_buffer->buffer[i] = '_';
        }
    }
    return found;
}