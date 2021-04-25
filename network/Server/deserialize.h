#ifndef __DESERIALIZE_H__
#define __DESERIALIZE_H__
#include "request.h"
#include <stdbool.h>

unsigned char *deserialize_int(unsigned char *buffer, int *value);
unsigned char *deserialize_type(unsigned char *buffer, Type *value);
unsigned char *deserialize_string(unsigned char *buffer, char *value);
unsigned char *deserialize_bool(unsigned char *buffer, bool *value);

#endif
