#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char reader_error_msg[256];

const char* get_reader_error(void) {
    return reader_error_msg;
}

char* read_file(const char* filename) {
    errno = 0;
    FILE* file = fopen(filename, "r");
    if (!file) {
        if (errno == ENOENT) {
            snprintf(reader_error_msg, sizeof(reader_error_msg),
                     "File '%s' not found.", filename);
        } else if (errno == EACCES) {
            snprintf(reader_error_msg, sizeof(reader_error_msg),
                     "Cannot read '%s' — permission denied.", filename);
        } else {
            snprintf(reader_error_msg, sizeof(reader_error_msg),
                     "Cannot open file '%s'.", filename);
        }
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size == 0) {
        fclose(file);
        snprintf(reader_error_msg, sizeof(reader_error_msg),
                 "File '%s' is empty.", filename);
        return NULL;
    }

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        snprintf(reader_error_msg, sizeof(reader_error_msg),
                 "Out of memory reading '%s'.", filename);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}