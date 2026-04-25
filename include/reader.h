#ifndef READER_H
#define READER_H

char* read_file(const char* filename);

/* Returns the last error message set by read_file().
   Valid only after read_file() has returned NULL. */
const char* get_reader_error(void);

#endif