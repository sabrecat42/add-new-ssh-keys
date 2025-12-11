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

// don't know how much mem is allocated for dst_str -> assume it's enough
void copystr(char* dst_str, char* src_str) {
    for (size_t i = 0; i < str_len(src_str); i++) {
        *(dst_str + i) = *(src_str + i);
    }
    *(dst_str + str_len(src_str)) = '\0';
}

bool str_eq(char* str1, char* str2) {
    if (str_len(str1) != str_len(str2)) return false;
    int len = str_len(str1);
    for (size_t i = 0; i < len; i++) {
        if (*(str1 + i) != *(str2 + i)) return false;
    }
    return true;
}

char* str_cat(char* str1, char* str2) {
    char* cat_str = malloc(str_len(str1) + str_len(str2) + 1);
    if (!cat_str) fprintf(stderr, "cannot malloc str_cat");
    for (size_t i = 0; i < str_len(str2); i++) {
        copystr(cat_str, str1);
        copystr(cat_str + str_len(str1), str2);
    }
    return cat_str;
}

// will modify old_key_count as side effect
// keep track of old_keys size via old_key_count
char* read_keys_from_file(FILE* f, int* key_count) {
    // allocate enough space to store 10 keys. As/if needed realloc more when reading keys
    char* keys = malloc(SSH_KEY_BUFF_SIZE * DEFAULT_KEY_LIST_SIZE);
    char* buffer = malloc(SSH_KEY_BUFF_SIZE + 1);
    // initialize to 0 just in case. 
    *key_count = 0;

    while (fgets(buffer, SSH_KEY_BUFF_SIZE + 1, f)) {
        // add new line to end of each shh key string
        size_t buff_str_len = str_len(buffer);
        if (buff_str_len == 0 || (buff_str_len == 1 && buffer[0] == '\n')) continue;
        if (*(buffer + buff_str_len - 1) != '\n') {
            *(buffer + buff_str_len) = '\n';
            *(buffer + buff_str_len + 1) = '\0';
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
    free(buffer);
    buffer = NULL;
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

// void*** malloc_2d_array(size_t row_size, size_t col_size, size_t data_size, size_t data_pointer_size) {
//     // each row is a pointer -> actual rows (arrays)
//     void*** matrix = malloc(row_size * data_pointer_size);
//     for (size_t i=0; i < row_size; i++) {
//         // allocate 
//         matrix[i] = malloc(col_size * data_pointer_size);
//         for (size_t j=0; j < col_size; j++) {
//             matrix[i][j] = malloc(data_size);
//         }
//     }
// }

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

    printf("\n\n");

    // could be optimized if exact count of unique keys are known
    // char* final_keys = malloc(SSH_KEY_BUFF_SIZE * (old_key_count + new_key_count));
    int final_keys_count = old_key_count + new_key_count;

    // int* dupl_table[][] = malloc(old_key_count * new_key_count);

    bool* dupl_table = malloc(old_key_count * new_key_count * sizeof(bool));
    bool** dupl_table_idx = malloc(old_key_count * sizeof(bool*));
    for (size_t i = 0; i < old_key_count; i++) {
        dupl_table_idx[i] = dupl_table + i*new_key_count; 
    }
    

    for (size_t i = 0; i < old_key_count; i++) {
        char* old_key_i = (old_keys + (SSH_KEY_BUFF_SIZE * i));
        // printf("old key %zu: %s", i, old_key_i);
        for (size_t j = 0; j < new_key_count; j++) {
            char* new_key_j = (new_keys + (SSH_KEY_BUFF_SIZE * j));
            // printf("new key %zu: %s", j, new_key_j);
            if (str_eq(old_key_i, new_key_j)) {
                // printf("\nold key %zu (%s) \n matches new key %zu (%s)\n", i, old_key_i, j, new_key_j);
                dupl_table_idx[i][j] = true;
                final_keys_count = final_keys_count - 1;
            } else {
                dupl_table_idx[i][j] = false;
                // copystr(final_keys+(final_keys_count * SSH_KEY_BUFF_SIZE), old_key_i);
                // final_keys_count++;
            }
        }
        // TODO: figure out how to write unique keys to final_keys
    }

    char** final_keys = malloc(final_keys_count * sizeof(char*));

    // first, write all old keys
    for (size_t i = 0; i < old_key_count; i++) {
        // copystr(final_keys[SSH_KEY_BUFF_SIZE*i], old_keys[SSH_KEY_BUFF_SIZE*i]);
        final_keys[i] = (old_keys + (SSH_KEY_BUFF_SIZE*i));
    }
    // then, write only unique new keys
    int unique_key_idx = 0;
    for (size_t i = 0; i < new_key_count; i++) {
        // final_keys[SSH_KEY_BUFF_SIZE*i] = old_keys[final_keys[SSH_KEY_BUFF_SIZE*i]]
        // skip adding old_key if it matches any of the new keys
        bool matches_old_key = false;
        for (size_t j = 0; j < old_key_count; j++) {
            if (dupl_table_idx[i][j]) {
                matches_old_key  = true;
                break;
            }
        }
        if (!matches_old_key) {
            final_keys[old_key_count + i] = (new_keys + (SSH_KEY_BUFF_SIZE * i));
        }
    }

    printf("\n\nFinal Keys:\n");
    for (size_t i = 0; i < final_keys_count; i++) {
        printf("Key %zu: %s", i, final_keys[i]);
    }
    

    // printf("old key 4: %s", (old_keys + (SSH_KEY_BUFF_SIZE * 4)));
    // printf("old key 5: %s", (old_keys + (SSH_KEY_BUFF_SIZE * 5)));

    return 0;
}