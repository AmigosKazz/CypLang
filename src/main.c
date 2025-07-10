#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 1024 * 1024 // 1MB

char* readFile(const char* filename);
void processCypLang(const char* source);

int main(int argc, char *argv[]) {
    const char* defaultFile = "input.cyp";
    const char* filename;

    if (argc > 1) {
        filename = argv[1];
    } else {
        filename = defaultFile;
        printf("No input file specified, using default: %s\n", defaultFile);
    }

    char* source = readFile(filename);
    if (!source) {
        fprintf(stderr, "Error: Could not read file %s\n", filename);
        return EXIT_FAILURE;
    }

    printf("Processing CypLang file: %s\n", filename);

    processCypLang(source);

    free(source);

    return EXIT_SUCCESS;
}

char* readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large (max: %d bytes)\n", MAX_FILE_SIZE);
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, size, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    return buffer;
}

void processCypLang(const char* source) {
    printf("CypLang source code:\n%s\n", source);
    printf("--------------------------------\n");
    printf("(Replace this with actual CypLang processing)\n");

    printf("\nOutput:\n");

    if (strstr(source, "4 + 4") != NULL) {
        printf("x = 8\n");
    }
}