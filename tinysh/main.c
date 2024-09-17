#include "ret.h"
#include "command.h"
#include "handlers.h"
#include "exec.h"
#include "utils.h"
int main(int argc, char* argv[])
{
    _update_title();
    printf("\nWelcome to tinysh [https://github.com/reslaid/tinysh]\n");
    char cbuf[256];
    while (1)
    {
        printf("\n");
        _show_prompt();
        if (fgets(cbuf, sizeof(cbuf), stdin) != NULL)
        {
            size_t cbuf_len = strlen(cbuf);
            if (cbuf_len > 0 && cbuf[cbuf_len - 1] == '\n')\
            {
                cbuf[cbuf_len - 1] = '\0';
            }
            _exec(cbuf);
        }
    }
    return 0;
}
