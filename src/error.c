#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static char pending_hint[256] = "";

void set_error_hint(const char *hint) {
    snprintf(pending_hint, sizeof(pending_hint), "%s", hint);
}

void error_at(int line, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, " at line %d\n", line);
    if (pending_hint[0] != '\0') {
        fprintf(stderr, "  Hint: %s\n", pending_hint);
        pending_hint[0] = '\0';
    }
    exit(1);
}
