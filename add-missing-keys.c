#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SSH_KEY_BUFF_SIZE 8192

int main(void) {
    char buffer[SSH_KEY_BUFF_SIZE];
    int old_key_count = 0;
    int new_key_count = 0;

    // allocate enough space to store 10 keys. As/if needed realloc more when reading keys
    char* old_keys = malloc(SSH_KEY_BUFF_SIZE * 10);
    
    // open authorized_keys
    const char* filename = ".ssh/authorized_keys";
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    // store old keys 
    // char* old_keys = malloc(SSH_KEY_BUFF_SIZE * old_key_count);

    while (fgets(buffer, SSH_KEY_BUFF_SIZE, fp)) {
        // strcpy(old_keys[(old_key_count * SSH_KEY_BUFF_SIZE)], buffer);
        strcpy(old_keys + (old_key_count * SSH_KEY_BUFF_SIZE), buffer);
        // *(old_keys + (old_key_count * SSH_KEY_BUFF_SIZE)) = buffer;

        old_key_count++;
        printf("%i: %s", old_key_count, buffer);
        // realloc memory for 10 more keys when you reach previous allocated memory limit
        if ((old_key_count % 10) == 0) {
            char* tmp = realloc(old_keys, (SSH_KEY_BUFF_SIZE * (old_key_count + 10)));
            if (!tmp) {
                perror("failed realloc extra memory for old ssh keys array");
                return 1;
            }
            old_keys = tmp;
        }
    }

    printf("\n");

    for (int i = 0; i < old_key_count; i++) {
        printf("you have %i keys", old_key_count);
        printf("key #%i: %s", i, old_keys + (SSH_KEY_BUFF_SIZE * i));
    }
    

    // while (fgets(buffer, SSH_KEY_BUFF_SIZE, stdin)) {
    //     key_count++;
    //     printf("%i: %s\n", key_count, buffer);
    // }
}