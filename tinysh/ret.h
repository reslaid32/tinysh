#ifndef _RET_H
#define _RET_H
#include <stdint.h>
typedef int32_t ret_t;
static ret_t _last_return = 0;
void _set_last_return(ret_t ret)
{
    _last_return = ret;
}
ret_t _get_last_return()
{
    return _last_return;
}
int _cmp_last_return(ret_t _oth_ret)
{
    return _get_last_return() == _oth_ret;
}
#endif // _RET_H