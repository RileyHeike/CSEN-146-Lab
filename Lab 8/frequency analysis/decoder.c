#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALPHABET_SIZE 26
#define MAX_TEXT_LEN 10000

char ciphertext[MAX_TEXT_LEN + 1];
char mapping[ALPHABET_SIZE];  // cipher letter -> plain letter mapping
int text_len = 0;

// Load ciphertext from file into global ciphertext buffer
void load_ciphertext(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open ciphertext file");
        exit(1);
    }
    text_len = fread(ciphertext, 1, MAX_TEXT_LEN, f);
    ciphertext[text_len] = '\0';
    fclose(f);
}

// Load mapping from file; format: one line per mapping like 'P:E'
void load_mapping(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Mapping file not found, using identity mapping.\n");
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            mapping[i] = 'A' + i;
        }
        return;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) mapping[i] = 'A' + i;

    char line[100];
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) < 3) continue;
        if (line[1] != ':') continue;
        char c_from = toupper(line[0]);
        char c_to = toupper(line[2]);
        if (c_from >= 'A' && c_from <= 'Z' && c_to >= 'A' && c_to <= 'Z') {
            mapping[c_from - 'A'] = c_to;
        }
    }
    fclose(f);
}

// Save mapping to a file
void save_mapping(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to open mapping file for writing");
        return;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        fprintf(f, "%c:%c\n", 'A' + i, mapping[i]);
    }
    fclose(f);
    printf("Mapping saved to %s\n", filename);
}

// Apply mapping and print the decoded text
void print_decoded_text() {
    printf("\n--- Decoded Text Preview ---\n");
    for (int i = 0; i < text_len; i++) {
        char ch = ciphertext[i];
        if (isalpha(ch)) {
            int upper = isupper(ch);
            char base = upper ? 'A' : 'a';
            char mapped = mapping[toupper(ch) - 'A'];
            if (!upper) mapped = tolower(mapped);
            putchar(mapped);
        } else {
            putchar(ch);
        }
    }
    printf("\n--- End Preview ---\n");
}

// Swap two mappings by cipher letters input by user
void swap_mappings(char c1, char c2) {
    c1 = toupper(c1);
    c2 = toupper(c2);
    if (c1 < 'A' || c1 > 'Z' || c2 < 'A' || c2 > 'Z') {
        printf("Invalid input. Use letters A-Z.\n");
        return;
    }
    int idx1 = c1 - 'A';
    int idx2 = c2 - 'A';
    char temp = mapping[idx1];
    mapping[idx1] = mapping[idx2];
    mapping[idx2] = temp;
    printf("Swapped mapping: %c ↔ %c\n", c1, c2);
}

void print_mapping() {
    printf("\nCurrent substitution mapping:\n");
    printf("Cipher : Plain\n");
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        printf("   %c   :   %c\n", 'A' + i, mapping[i]);
    }
}

void usage() {
    printf("\nCommands:\n");
    printf("  p            : print decoded text preview\n");
    printf("  m            : print current mapping\n");
    printf("  s c1 c2      : swap mapping for cipher letters c1 and c2\n");
    printf("  w filename   : write/save current mapping to filename\n");
    printf("  q            : quit\n");
    printf("  h            : help\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s ciphertext_file [mapping_file]\n", argv[0]);
        return 1;
    }

    load_ciphertext(argv[1]);
    if (argc >= 3)
        load_mapping(argv[2]);
    else {
        // Identity mapping by default
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            mapping[i] = 'A' + i;
        }
    }

    printf("Loaded ciphertext (%d chars).\n", text_len);
    usage();

    char command[256];
    while (1) {
        printf("> ");
        if (!fgets(command, sizeof(command), stdin)) break;

        // Remove trailing newline
        size_t len = strlen(command);
        if (len > 0 && command[len-1] == '\n') command[len-1] = '\0';

        if (strlen(command) == 0) continue;

        if (command[0] == 'p') {
            print_decoded_text();
        } else if (command[0] == 'm') {
            print_mapping();
        } else if (command[0] == 's') {
            // swap command: format "s c1 c2"
            char c1 = 0, c2 = 0;
            if (sscanf(command + 1, " %c %c", &c1, &c2) == 2) {
                swap_mappings(c1, c2);
            } else {
                printf("Invalid swap command. Usage: s c1 c2\n");
            }
        } else if (command[0] == 'w') {
            char filename[256];
            if (sscanf(command + 1, " %255s", filename) == 1) {
                save_mapping(filename);
            } else {
                printf("Invalid write command. Usage: w filename\n");
            }
        } else if (command[0] == 'q') {
            printf("Quitting.\n");
            break;
        } else if (command[0] == 'h') {
            usage();
        } else {
            printf("Unknown command '%c'. Type h for help.\n", command[0]);
        }
    }

    return 0;
}

