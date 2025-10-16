#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>


unsigned int pack(unsigned char *buf, char *format, ...);
void unpack(unsigned char *buf, char *format, ...);
