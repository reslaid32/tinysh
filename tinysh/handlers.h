#ifndef _HANDLERS_H
#define _HANDLERS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ret.h"
#include "command.h"
#include "utils.h"
ret_t handle_echo(int argc, char* argv[])
{
    printf("%s", _concatenate_args(argc, argv));
    printf("%c", '\n');
    return EXIT_SUCCESS;
}
ret_t handle_exit(int argc, char* argv[])
{
    printf("Bye!\n");
    exit(EXIT_SUCCESS);
}
ret_t handle_title(int argc, char* argv[])
{
    _set_title(_concatenate_args(argc, argv));
    _update_title();
    return EXIT_SUCCESS;
}
ret_t handle_clear(int argc, char* argv[])
{
    _clear_console();
    return EXIT_SUCCESS;
}
ret_t handle_cd(int argc, char* argv[])
{
    _set_curdir(_concatenate_args(argc, argv));
    return EXIT_SUCCESS;
}
ret_t handle_cwd(int argc, char* argv[])
{
    char cwd[260];
    if (!_get_curdir(cwd, sizeof(cwd)))
    {
        perror("Failed to get current directory");
        return EXIT_FAILURE;
    }
    printf("%s\n", cwd);
    return EXIT_SUCCESS;
}
ret_t handle_ls(int argc, char* argv[])
{
    size_t num_files, columns = 3;
    char** files = _list_curdir(&num_files);
    if (num_files == 0) return EXIT_FAILURE;

    size_t max_length = 0;

    for (size_t i = 0; i < num_files; ++i) {
        size_t len = strlen(files[i]);
        if (len > max_length) {
            max_length = len;
        }
    }

    size_t col_width = max_length + 2;
    size_t rows = (num_files + columns - 1) / columns;

    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < columns; ++c) {
            size_t index = r + c * rows;
            if (index < num_files) {
                printf("%-*s", (int)col_width, files[index]);
            }
        }
        printf("\n");
    }
    return EXIT_SUCCESS;
}
ret_t handle_mkdir(int argc, char* argv[])
{
    if (!_mkdir(_concatenate_args(argc, argv)))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
ret_t handle_rmdir(int argc, char* argv[])
{
    if (!_rmdir(_concatenate_args(argc, argv)))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
ret_t handle_touch(int argc, char* argv[])
{
    if (!_touchfile(_concatenate_args(argc, argv)))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
ret_t handle_rm(int argc, char* argv[])
{
    if (!_rmfile(_concatenate_args(argc, argv)))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
ret_t handle_lrc(int argc, char* argv[])
{
    printf("Last return code: [%d]\n", _get_last_return());
    return EXIT_SUCCESS;
}
ret_t handle_help(int argc, char* argv[]);
const command_t commands[] 
= {
    {"title", "Changes the command line title.", handle_title, 1, -1, "<title>"},
    {"ls", "List of all files and directories.", handle_ls, 0, 0, "<title>"},
    {"cd", "Change current directory.", handle_cd, 1, -1, "<path>"},
    {"cwd", "Get current directory.", handle_cwd, 0, 0, ""},
    {"mkdir", "Creates a new directory.", handle_mkdir, 1, -1, "<directory>"},
    {"rmdir", "Removes a directory.", handle_rmdir, 1, -1, "<directory>"},
    {"touch", "Creates a new file.", handle_touch, 1, -1, "<file>"},
    {"rm", "Removes a file.", handle_rm, 1, -1, "<file>"},
    {"echo", "Prints the message to the screen.", handle_echo, 1, -1, "<message>"},
    {"clear", "Clears the console.", handle_clear, 0, 0, ""},
    {"lrc", "Shows the last return code.", handle_lrc, 0, 0, ""},
    {"exit", "Exits the shell.", handle_exit, 0, 0, ""},
    {"help", "Displays help information.", handle_help, 0, 0, ""},
};
const size_t num_commands = sizeof(commands) / sizeof(commands[0]);
ret_t handle_help(int argc, char* argv[])
{
    printf("Available commands:\n");
    for (size_t i = 0; i < num_commands; ++i) {
        const command_t* cmd = &commands[i];
        printf("  %s: %s\n", cmd->name, cmd->description);
        if (cmd->arg_annotations != NULL && strlen(cmd->arg_annotations) > 0) {
            printf("    Usage: %s %s\n", cmd->name, cmd->arg_annotations);
        }
    }
    return EXIT_SUCCESS;
}
ret_t handle_unknown(int argc, char* argv[], char* token)
{
    if (_execute_file(token, argc, argv) == EXIT_FAILURE)
    {
        printf("Unknown command: %s\n", token);
    }
    return EXIT_FAILURE;
}
#endif // _HANDLERS_H