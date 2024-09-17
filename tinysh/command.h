#ifndef _COMMAND_H
#define _COMMAND_H
#include "ret.h"
typedef struct
{
    const char* name;
    const char* description;
    ret_t (*handler)(int argc, char* argv[]);
    int min_args;
    int max_args;
    const char* arg_annotations;
} command_t;
#endif // _COMMAND_H