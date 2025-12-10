#include <stdio.h>
#include <stdlib.h>
#include "my_strings.h"

int main() {
    char *s1 = "Hello ";
    char *s2 = "World";

    char *hw = concat_strings(s1, s2);

    printf("%s", hw);

    return 0;
}