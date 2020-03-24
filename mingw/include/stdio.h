#ifndef STDIO_H_INCLUDED
#define STDIO_H_INCLUDED

#include_next <stdio.h>

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#endif // STDIO_H_INCLUDED
