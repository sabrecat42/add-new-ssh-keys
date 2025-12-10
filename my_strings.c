#include <stddef.h>
#include <stdlib.h>

// string is defined as a contiguous sequence of chars in memory. The end of the char string is determined by the special value '\0'.
size_t my_strlen(char *str) 
{
    size_t len = 0;
    while (*(str + len) != '\0') {
        len++;
    }
    return len;
}

char *concat_strings(char *str_a, char *str_b) {
    size_t len_a = my_strlen(str_a);
    size_t len_b = my_strlen(str_b);

    char *str_final = malloc(len_a + len_b + 1);
    if (!str_final) return NULL;

    for (size_t i = 0; i < len_a; i++) {
        *(str_final + i) = *(str_a + i);
    }
    for (size_t j = 0; j < len_b; j++) {
        *(str_final + len_a + j) = *(str_b + j);
    }
    *(str_final + len_a + len_b) = '\0';

    return str_final;
}