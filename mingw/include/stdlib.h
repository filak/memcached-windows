#ifndef STDLIB_H_INCLUDED
#define STDLIB_H_INCLUDED

#include_next <stdlib.h>

int getsubopt(char **optionp, char * const *tokens, char **valuep);

#endif // STDLIB_H_INCLUDED
