#ifndef _UTILS_H
#define _UTILS_H
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <lmcons.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#endif
static char _title[256] = "tinysh";
int _get_username(char* username, unsigned long size)
{
    int ret = 0;
    #ifdef _WIN32
    if (GetUserNameA(username, &size)) {
        ret = 1;
    }
    #else
    const char* env_user = getenv("USER") ? getenv("USER") : getenv("LOGNAME");
    if (env_user && strlen(env_user) < size) {
        strncpy(username, env_user, size);
        ret = 1;
    }
    #endif
    return ret;
}
int _get_hostname(char* hostname, unsigned long size)
{
    int ret = 0;
    #ifdef _WIN32
    if (GetComputerNameA(hostname, &size)) {
        ret = 1;
    }
    #else
    if (gethostname(hostname, size) == 0) {
        ret = 1;
    }
    #endif
    return ret;
}
int _get_root()
{
#ifdef _WIN32
    BOOL isAdmin = 0;
    PSID adminsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &adminsGroup)) {
        if (CheckTokenMembership(NULL, adminsGroup, &isAdmin)) {
            FreeSid(adminsGroup);
            return isAdmin ? 1 : 0;
        } else {
            FreeSid(adminsGroup);
            return 0;
        }
    } else {
        return 0;
    }
    #else
    return geteuid() == 0 ? 1 : 0;
    #endif
}
char _get_root_prompt(int root)
{
    return root ? '#' : '$';
}
void _set_title(const char* title)
{
    if (title != NULL)
    {
        strncpy(_title, title, sizeof(_title) - 1);
        _title[sizeof(_title) - 1] = '\0';
    }
}
char* _get_title()
{
    return _title;
}
char* _get_suffixed_title() {
    const char* title = _get_title();
    if (title == NULL) {
        return NULL;
    }

    if (_get_root()) {
        size_t new_title_len = strlen("Root: ") + strlen(title) + 1;

        char* new_title = (char*)malloc(new_title_len);
        if (new_title == NULL) {
            perror("Failed to allocate memory for suffixed title");
            return NULL;
        }

        strcpy(new_title, "Root: ");
        strcat(new_title, title);

        return new_title;
    } else {
        return strdup(title);
    }
}
void _update_title()
{
    #ifdef _WIN32
    SetConsoleTitleA(_get_suffixed_title());
    #else
    printf("\033]0;%s\007", _get_suffixed_title());
    #endif
}
int _get_curdir(char *cwd, unsigned long size)
{
    int ret;
    #ifdef _WIN32
    if (GetCurrentDirectoryA(size, cwd)) {
        ret = 1;
    } else {
        ret = 0;
    }
    #else
    if (getcwd(cwd, size) != NULL) {
        ret = 1;
    } else {
        perror("getcwd() error");
        ret = 0;
    }
    #endif
    return ret;
}
int _set_curdir(const char *path)
{
    int ret;
    #ifdef _WIN32
    if (SetCurrentDirectoryA(path)) {
        ret = 1;
    } else {
        ret = 0;
    }
    #else
    if (chdir(path) == 0) {
        ret = 1;
    } else {
        perror("chdir() error");
        ret = 0;
    }
    #endif
    return ret;
}
int _mkdir(const char* path) {
    int ret;
    #ifdef _WIN32
    if (CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        ret = 1;
    } else {
        ret = 0;
        fprintf(stderr, "Error creating directory: %ld\n", GetLastError());
    }
    #else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) {
        ret = 1;
    } else {
        ret = 0;
        perror("mkdir");
    }
    #endif
    return ret;
}
int _rmdir(const char* path) {
    int ret;
    #ifdef _WIN32
    if (RemoveDirectoryA(path)) {
        ret = 1;
    } else {
        ret = 0;
        fprintf(stderr, "Error removing directory: %ld\n", GetLastError());
    }
    #else
    if (rmdir(path) == 0) {
        ret = 1;
    } else {
        ret = 0;
        perror("rmdir");
    }
    #endif
    return ret;
}
int _rmfile(const char* path) {
    int ret;
    #ifdef _WIN32
    if (DeleteFileA(path)) {
        ret = 1;
    } else {
        ret = 0;
        fprintf(stderr, "Error deleting file: %ld\n", GetLastError());
    }
    #else
    if (remove(path) == 0) {
        ret = 1;
    } else {
        ret = 0;
        perror("remove");
    }
    #endif
    return ret;
}
int _touchfile(const char* path) {
    int ret;
    #ifdef _WIN32
    HANDLE hFile = CreateFileA(
        path,                   
        GENERIC_WRITE,          
        FILE_SHARE_WRITE,       
        NULL,                   
        OPEN_ALWAYS,            
        FILE_ATTRIBUTE_NORMAL,  
        NULL                    
    );
    
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        ret = 1;
    } else {
        ret = 0;
        fprintf(stderr, "Error creating or updating file: %ld\n", GetLastError());
    }
    #else
    int fd = open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        close(fd);
        ret = 1;
    } else {
        ret = 0;
        perror("open");
    }
    #endif
    return ret;
}
char** _list_curdir(size_t *num_files)
{
    #ifdef _WIN32
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA("*", &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("FindFirstFileA failed (%lu)\n", (unsigned long)GetLastError());
        return NULL;
    }

    size_t capacity = 10;
    size_t count = 0;
    char** files = (char**)malloc(capacity * sizeof(char*));

    if (files == NULL) {
        printf("Memory allocation failed\n");
        FindClose(hFind);
        return NULL;
    }

    do {
        if (count >= capacity) {
            capacity *= 2;
            files = (char**)realloc(files, capacity * sizeof(char*));
            if (files == NULL) {
                printf("Memory allocation failed\n");
                FindClose(hFind);
                return NULL;
            }
        }

        files[count] = strdup(findFileData.cFileName);
        if (files[count] == NULL) {
            printf("Memory allocation failed\n");
            FindClose(hFind);
            return NULL;
        }
        count++;
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);

    *num_files = count;
    return files;
    #else
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir() error");
        return NULL;
    }

    size_t capacity = 10;
    size_t count = 0;
    char** files = (char**)malloc(capacity * sizeof(char*));

    if (files == NULL) {
        perror("Memory allocation failed");
        closedir(dir);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (count >= capacity) {
            capacity *= 2;
            files = (char**)realloc(files, capacity * sizeof(char*));
            if (files == NULL) {
                perror("Memory allocation failed");
                closedir(dir);
                return NULL;
            }
        }

        files[count] = strdup(entry->d_name);
        if (files[count] == NULL) {
            perror("Memory allocation failed");
            closedir(dir);
            return NULL;
        }
        count++;
    }

    closedir(dir);

    *num_files = count;
    return files;
    #endif
}
void _show_prompt()
{
    char /*hostname[256], username[256],*/ cwd[260];
    if (!_get_curdir(cwd, sizeof(cwd)))
    {
        perror("Failed to get current directory!");
        printf("> ");
    }
    else
    {
        printf("[%s]:~%c ", cwd, _get_root_prompt(_get_root()));
    }
}
char* _concatenate_args(int argc, char* argv[]) {
    size_t total_size = 1;

    for (int i = 0; i < argc; ++i) {
        total_size += strlen(argv[i]) + 1;
    }

    char* result = (char*)malloc(total_size);
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    result[0] = '\0';

    for (int i = 0; i < argc; ++i) {
        if (i > 0) {
            strcat(result, " ");
        }
        strcat(result, argv[i]);
    }

    return result;
}
void _clear_console()
{
    #ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    COORD coord = {0, 0};
    DWORD dwCharsWritten;

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

        FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coord, &dwCharsWritten);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coord, &dwCharsWritten);

        SetConsoleCursorPosition(hConsole, coord);
    }
    #else
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    printf("\033[H\033[J");
    fflush(stdout);
    #endif
}
ret_t _execute_file(const char* file, int argc, char* argv[])
{
    #ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    CHAR cmd_line[1024];
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    snprintf(cmd_line, sizeof(cmd_line), "%s %s", file, _concatenate_args(argc, argv));
    
    if (CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
    #else
    pid_t pid = fork();
    
    if (pid == 0) {
        char* args[argc + 2];
        args[0] = (char*)file;
        for (int i = 1; i <= argc; ++i) {
            args[i] = argv[i - 1];
        }
        args[argc + 1] = NULL;
        
        execvp(file, args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);
        return EXIT_SUCCESS;
    } else {
        perror("fork failed");
        return EXIT_FAILURE;
    }
    #endif
}
#endif // _UTILS_H