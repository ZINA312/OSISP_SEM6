#include "reverse.h"

void reverse_buffer(unsigned char *buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return; 
    }
    for (size_t i = 0; i < size / 2; i++) {
        unsigned char temp = buffer[i];
        buffer[i] = buffer[size - 1 - i];
        buffer[size - 1 - i] = temp;
    }
}
