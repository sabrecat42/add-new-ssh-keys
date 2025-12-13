#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SSH_KEY_BUFF_SIZE 256
#define DEFAULT_KEY_LIST_SIZE 2

bool using_def_keys_location = false;

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
    size_t len1 = str_len(str1);
    size_t len2 = str_len(str1);

    char* cat_str = malloc(len1 + len2 + 1);
    if (!cat_str) {
        fprintf(stderr, "cannot malloc str_cat");
        return NULL;
    }

    copystr(cat_str, str1);
    copystr(cat_str + len1, str2);
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

int main(int argc, char* argv[]) {
    char* filename = NULL;
    switch (argc)
    {
    case 1: { // read from stdin; write to default location
        using_def_keys_location = true;
        char* home_dir = getenv("HOME");
        if (!home_dir) {
            fprintf(stderr, "failed to get user home dir");
            return 1;
        }
        filename = str_cat(home_dir, "/.ssh/authorized_keys");
        break;
    }
    case 2: // read from stdin; write to user defined location
        filename = argv[1];
        break;
    // case 3:
    //     char* in_keys = argv[1];
    //     filename = argv[2];
    //     break;
    default:
        fprintf(stderr, "Usage:\ncurl \"https://...\" | amk-ssh\ncurl \"https://...\" | amk-ssh <authorized_keys-location>\n");
        // amk-ssh <public-key(s)> <authorized_keys-location>\n
        return 1;
    }
    if (!filename) {
        fprintf(stderr, "internal error: no filename was assigned");
        return 1;
    }

    int old_key_count = 0;
    int new_key_count = 0;
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
    bool* dupl_table = malloc(old_key_count * new_key_count * sizeof(bool));
    bool** dupl_table_idx = malloc(old_key_count * sizeof(bool*));
    for (size_t i = 0; i < old_key_count; i++) {
        dupl_table_idx[i] = dupl_table + i*new_key_count; 
    }
    

    int final_keys_count = old_key_count;
    for (size_t j = 0; j < new_key_count; j++) {
        char* new_key_j = (new_keys + (SSH_KEY_BUFF_SIZE * j));
        // printf("old key %zu: %s", i, old_key_i);
        bool is_duplicated = false;
        for (size_t i = 0; i < old_key_count; i++) {
            char* old_key_i = (old_keys + (SSH_KEY_BUFF_SIZE * i));
            // printf("new key %zu: %s", j, new_key_j);
            if (str_eq(old_key_i, new_key_j)) {
                // printf("\nold key %zu (%s) \n matches new key %zu (%s)\n", i, old_key_i, j, new_key_j);
                dupl_table_idx[i][j] = true;
                is_duplicated = true;
            } else {
                dupl_table_idx[i][j] = false;
            }
        }
        if (!is_duplicated) final_keys_count++;
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
            if (dupl_table_idx[j][i]) {
                matches_old_key  = true;
                break;
            }
        }
        if (!matches_old_key) {
            final_keys[old_key_count + unique_key_idx] = (new_keys + (SSH_KEY_BUFF_SIZE * i));
            unique_key_idx++;
        }
    }

    printf("\n\nFinal Keys:\n");
    for (size_t i = 0; i < final_keys_count; i++) {
        printf("Key %zu: %s", i, final_keys[i]);
    }

    FILE *fptr;

    // Open a file in writing mode
    fptr = fopen(filename, "w");
    if (using_def_keys_location) {
        free(filename);
    }
    // Write some text to the file
    for (size_t i=0; i<final_keys_count; i++) {
        fprintf(fptr, "%s", final_keys[i]);
    }
    // Close the file
    fclose(fptr);
    
    // printf("old key 4: %s", (old_keys + (SSH_KEY_BUFF_SIZE * 4)));
    // printf("old key 5: %s", (old_keys + (SSH_KEY_BUFF_SIZE * 5)));

    return 0;
}