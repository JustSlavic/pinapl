#include "parse.h"


uint32 parse_integer(char const *data, uint64 size, int64 *result)
{
    char c = data[0];
    if (c == '0')
    {
        /* Either octal or hex */
        if (data[1] == 'x')
        {
            if (size < 3)
            {
                /* '0x' is not acceptable string. */
                return 0;
            }
            /* Hex */
            int64 n = 0;
            int32 i = 2;
            for (; i < size; i++)
            {
                c = data[i];
                if (ascii_is_hex(c))
                {
                    n *= 16;
                    n += ascii_to_hex(c);
                }
                else
                {
                    break;
                }
            }
            *result = n;
            return i;
        }
        else if (ascii_is_oct(c))
        {
            int64 n = 0;
            int32 i = 1;
            for (; i < size; i++)
            {
                c = data[i];
                if (ascii_is_oct(c))
                {
                    n *= 8;
                    n += ascii_to_oct(c);
                }
                else
                {
                    break;
                }
            }
            *result = n;
            return i;
        }
        else
        {
            /* Zero followed by other character */
            *result = 0;
            return 1;
        }
    }
    else if (ascii_is_digit(c))
    {
        /* Decimal */
        int64 n = 0;
        int32 i = 0;
        for (; i < size; i++)
        {
            char c = data[i];
            if (ascii_is_dec(c))
            {
                n *= 10;
                n += ascii_to_dec(c);
            }
            else
            {
                break;
            }
        }
        *result = n;
        return i;
    }
    else
    {
        return 0;
    }
}
