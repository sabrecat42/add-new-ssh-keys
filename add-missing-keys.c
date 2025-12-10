#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SSH_KEY_BUFF_SIZE 8192
#define DEFAULT_KEY_LIST_SIZE 2

// will modify old_key_count as side effect
// keep track of old_keys size via old_key_count
char* read_keys_from_file(FILE* f, int* key_count) {
    // allocate enough space to store 10 keys. As/if needed realloc more when reading keys
    char* keys = malloc(SSH_KEY_BUFF_SIZE * DEFAULT_KEY_LIST_SIZE);
    char buffer[SSH_KEY_BUFF_SIZE];
    // initialize to 0 just in case. 
    *key_count = 0;
    while (fgets(buffer, SSH_KEY_BUFF_SIZE, f)) {
        // printf("fgets iteration\n");
        strcpy(keys + ((*key_count) * SSH_KEY_BUFF_SIZE), buffer);
        (*key_count)++;
        printf("%i: %s", *key_count, (keys + ((*key_count) * SSH_KEY_BUFF_SIZE)));
        // realloc memory for 10 more keys when you reach previous allocated memory limit
        if (((*key_count) % DEFAULT_KEY_LIST_SIZE) == 0) {
            char* tmp = realloc(keys, (SSH_KEY_BUFF_SIZE * (((*key_count) / DEFAULT_KEY_LIST_SIZE)+1)));
            if (!tmp) {
                perror("failed realloc extra memory for old ssh keys array");
                return NULL;
            }
            keys = tmp;
            printf("added 2 more via realloc; key_count=%i\n",*key_count);
            printf("current key_count = %i\n\n", (((*key_count) / DEFAULT_KEY_LIST_SIZE)+1));
        }
    }
    return keys;
}

char* read_old_keys(const char* filename, int* old_key_count) {
    // open authorized_keys
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return NULL;
    }
    char* result = read_keys_from_file(fp, old_key_count);
    fclose(fp);
    return result;
}

// char* read_new_keys(FILE* f, int* new_key_count) {
//     char buffer[SSH_KEY_BUFF_SIZE];
//     // allocate enough space to store 10 keys. As/if needed realloc more when reading keys
//     char* new_keys = malloc(SSH_KEY_BUFF_SIZE * DEFAULT_KEY_LIST_SIZE);
//     // open authorized_keys
//     while (fgets(buffer, SSH_KEY_BUFF_SIZE, f)) {
//         // printf("fgets iteration\n");
//         strcpy(new_keys + ((*new_key_count) * SSH_KEY_BUFF_SIZE), buffer);
//         (*new_key_count)++;
//         printf("%i: %s", *new_key_count, buffer);
//         // realloc memory for DEFAULT_KEY_LIST_SIZE more keys when you reach previous allocated memory limit (new_key_count % DEFAULT_KEY_LIST_SIZE) * 
//         if (((*new_key_count) % DEFAULT_KEY_LIST_SIZE) == 0) {
//             printf("added 2 more via realloc; key_count=%i\n",*new_key_count);
//             char* tmp = realloc(new_keys, (SSH_KEY_BUFF_SIZE * ((*new_key_count / DEFAULT_KEY_LIST_SIZE)+1));
//             if (!tmp) {
//                 perror("failed realloc extra memory for old ssh keys array");
//                 return NULL;
//             }
//             new_keys = tmp;
//         }
//     }
//     return new_keys;
// }

int main(void) {
    int old_key_count = 0;
    // int new_key_count = 0;
    const char* filename = "/Users/adrian/.ssh/authorized_keys";

    char* old_keys = read_old_keys(filename, &old_key_count);
    if (!old_keys) return 1;

    printf("\n");
    printf("you have %i keys\n", old_key_count);
    for (int i = 0; i < old_key_count; i++) {
        printf("key #%i: %s", i, old_keys + (SSH_KEY_BUFF_SIZE * i));
    }
    
    // while (fgets(buffer, SSH_KEY_BUFF_SIZE, stdin)) {
    //     key_count++;
    //     printf("%i: %s\n", key_count, buffer);
    // }
}