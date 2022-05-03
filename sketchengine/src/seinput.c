#include "seinput.h"

void seinput_text_input_consume(SE_Input *input, char *dest) {
    strcat(dest, input->text_input.buffer);
    sestring_clear(&input->text_input);
}