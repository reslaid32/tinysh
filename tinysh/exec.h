#ifndef _EXEC_H
#define _EXEC_H
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "ret.h"
#include "command.h"
#include "handlers.h"
ret_t _handle_command(char* token)
{
    char* args[256];
    int argc = 0;
    char* arg = strtok(NULL, " ");
    while (arg != NULL && argc < 256)
    {
        args[argc++] = arg;
        arg = strtok(NULL, " ");
    }
    for (size_t i = 0; i < num_commands; ++i)
    {
        if (strcmp(token, commands[i].name) == 0)
        {
            if (argc < commands[i].min_args || (commands[i].max_args != -1 && argc > commands[i].max_args))
            {
                if (commands[i].arg_annotations != NULL && strlen(commands[i].arg_annotations) > 0) {
                    printf("Usage: %s %s\n", commands[i].name, commands[i].arg_annotations);
                } else {
                    printf("This command does not exist or is not written completely.\n");
                }
                return EXIT_FAILURE;
            }
            return commands[i].handler(argc, args);
        }
    }

    handle_unknown(argc, args, token);
    return EXIT_FAILURE;
}
void _exec(const char* command) {
    char* _cmd_cp = strdup(command);
    if (_cmd_cp == NULL) {
        perror("Failed to allocate memory for command copy");
        return;
    }

    if (_cmd_cp[0] == '!') {
        system(_cmd_cp + 1);
        free(_cmd_cp);
        return;
    }

    char* token = strtok(_cmd_cp, " ");
    if (token == NULL) {
        free(_cmd_cp);
        return;
    }

    _set_last_return(_handle_command(token));

    free(_cmd_cp);
}
#endif // _EXEC_H