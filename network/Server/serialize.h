#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__
#include "request.h"
#include <stdbool.h>

unsigned char *serialize_int(unsigned char *buffer, int value);
unsigned char *serialize_type(unsigned char *buffer, Type value);
unsigned char *serialize_string(unsigned char *buffer, char *value);
unsigned char *serialize_bool(unsigned char *buffer, bool value);

#endif
