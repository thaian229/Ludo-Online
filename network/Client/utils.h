#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdlib.h>

FILE *getFile(char *filePath);
FILE *createFile(char *filePath);

char **str_split(char *a_str, const char a_delim);

#endif
