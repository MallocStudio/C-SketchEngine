#include "sestring.h"
#include "memory.h"
void sestring_init(SE_String *string, const char *buffer) {
    if (buffer != NULL) {
        string->size = SDL_strlen(buffer);
        string->capacity = string->size + 1; // +1 to accomidate '\0'

        string->buffer = malloc(sizeof(char) * string->capacity);

        for (u32 i = 0; i < string->size+1; ++i) {
            string->buffer[i] = buffer[i];
        }
    } else {
        string->size = 0;
        string->capacity = 0;
        string->buffer = NULL;
    }
}

void sestring_deinit(SE_String *string) {
    if (string->buffer != NULL) {
        free(string->buffer);
        string->buffer = NULL;
        string->size = 0;
        string->capacity = 0;
    }
}

void sestring_duplicate(SE_String *string1, SE_String *string2) {
    sestring_deinit(string2);
    if (string1->buffer != NULL) {
        sestring_init(string2, string1->buffer);
    }
}

void sestring_append(SE_String *string, const char *buffer) {
    sestring_append_length(string, buffer, SDL_strlen(buffer));
}

void sestring_append_length(SE_String *string, const char *src, u32 length) {
    if (string->buffer == NULL) {
        // if this string is empty, initialise it with the given src
        sestring_init(string, src);
    } else {
        u32 start_index = string->size;

        string->size += length;

        if (string->size > string->capacity) {
            string->capacity = string->size+1; // +1 to accomidate '\0'
            string->buffer = realloc(string->buffer, sizeof(char) * string->capacity);
        }

        for (u32 i = start_index; i < string->size+1; ++i) { // +1 to inlcude '\0'
            string->buffer[i] = src[i - start_index];
        }
    }
}

u32 sestring_lastof(SE_String *string, const char letter) {
    if (string->buffer == NULL || string->size == 0) {
        return SESTRING_MAX_SIZE;
    }

    u32 result = SESTRING_MAX_SIZE;
    for (u32 i = string->size; i > 0; --i) {
        if (string->buffer[i] == letter) {
            result = i;
            break;
        }
    }
    return result;
}

void sestring_clear(SE_String *string) {
    if (string->buffer == NULL || string->size == 0) {
        return;
    }
    string->size = 0;
}

void sestring_delete_from_end(SE_String *string, u32 amount) {
    if (string->size >= amount) {
        string->size -= amount;
        string->buffer[string->size] = '\0';
    }
}