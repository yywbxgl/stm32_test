#include "utils.h"
#include "logging.h"

void PrintHex(u8 *buf,u16 len)
{
    u16 i;
    LOGV("\r\n");
    for(i=0;i<len;i++)
    {
        LOGV("%02x ",buf[i]);
    }
    LOGV("\r\n\r\n");
}
