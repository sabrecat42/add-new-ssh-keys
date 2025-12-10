#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SSH_KEY_BUFF_SIZE 256
#define DEFAULT_KEY_LIST_SIZE 2

size_t str_len(char* str) {
    size_t len = 0;
    while (*(str + len) != '\0') {
        len++;
    }
    return len;
}

bool str_eq(char* str1, char* str2) {
    if (str_len(str1) != str_len(str2)) return false;
    int len = str_len(str1);
    for (size_t i = 0; i < len; i++) {
        if (*(str1 + i) != *(str2 + i)) return false;
    }
    return true;
}

// will modify old_key_count as side effect
// keep track of old_keys size via old_key_count
char* read_keys_from_file(FILE* f, int* key_count) {
    // allocate enough space to store 10 keys. As/if needed realloc more when reading keys
    char* keys = malloc(SSH_KEY_BUFF_SIZE * DEFAULT_KEY_LIST_SIZE);
    char buffer[SSH_KEY_BUFF_SIZE];
    // initialize to 0 just in case. 
    *key_count = 0;

    // char* test_str = malloc(3);
    // stpcpy(test_str, "123");
    // char char_str[] = "123";
    // // =3, only counts chars (excl. '\0')
    // printf("size of malloc_str: %zu\n", str_len(test_str));
    // // this = 4, bc '\0' at the end is part of the data size in memory
    // printf("size of str_arr: %zu\n", sizeof(char_str));
    
    while (fgets(buffer, SSH_KEY_BUFF_SIZE, f)) {
        // add new line to end of each shh key string
        if (*(buffer + str_len(buffer)-1) != '\n') {
            *(buffer + str_len(buffer)) = '\n';
            *(buffer + str_len(buffer) + 1) = '\0';
            printf("added newline char to ssh key: %s", buffer);
        }

        strcpy(keys + ((*key_count) * SSH_KEY_BUFF_SIZE), buffer);
        (*key_count)++;
        // realloc memory for 10 more keys when you reach previous allocated memory limit
        if (((*key_count) % DEFAULT_KEY_LIST_SIZE) == 0) {
            char* tmp = realloc(keys, (SSH_KEY_BUFF_SIZE * (*key_count + DEFAULT_KEY_LIST_SIZE)));
            if (!tmp) {
                perror("failed realloc extra memory for old ssh keys array");
                return NULL;
            }
            keys = tmp;
        }
    }
    return keys;
}

char* read_keys_from_location(const char* filename, int* old_key_count) {
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

int main(void) {
    int old_key_count = 0;
    int new_key_count = 0;
    const char* filename = "/Users/adrian/.ssh/authorized_keys";

    char* old_keys = read_keys_from_location(filename, &old_key_count);
    if (!old_keys) return 1;
    char* new_keys = read_keys_from_file(stdin, &new_key_count);
    if (!new_keys) return 1;

    printf("\n");
    printf("you already have %i keys:\n", old_key_count);
    for (int i = 0; i < old_key_count; i++) {
        printf("key #%i: %s", i, old_keys + (SSH_KEY_BUFF_SIZE * i));
    }
    printf("\n");
    printf("you have provided %i keys:\n", new_key_count);
    for (int i = 0; i < new_key_count; i++) {
        printf("key #%i: %s", i, new_keys + (SSH_KEY_BUFF_SIZE * i));
    }

    // for (size_t i = 0; i < old_key_count; i++) {
        
    // }   
}