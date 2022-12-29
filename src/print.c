#include <print.h>
#include <syscall.h>
#include <allocator.h>


GLOBAL struct allocator global_print_allocator;


void initialize_print_buffer(void *memory, usize size)
{
    initialize_memory_arena(&global_print_allocator, memory, size);
}


void print_flush(void)
{
    write(1, global_print_allocator.memory, global_print_allocator.used);
    global_print_allocator.used = 0;
}


void print_n(char const *string, usize size)
{
    if (global_print_allocator.used + size >= global_print_allocator.size)
    {
        print_flush();
    }

    void *buffer = ALLOCATE_BUFFER_(&global_print_allocator, size);
    memcpy(buffer, string, size);
}


void print_s(struct string string)
{
    print_n(string.data, string.size);
}


void print(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    usize start = 0;
    usize index = 0;
    while (fmt[index])
    {
        char c = fmt[index];
        if (c == '%')
        {
            char c1 = fmt[index + 1];
            if (c1 == '%')
            {
                // skip '%' and print only one '%'
                print_n(fmt + start, index - start + 1);

                start = index + 2;
                index = index + 2;
            }
            else if (c1 == 'd')
            {
                print_n(fmt + start, index - start);

                int number = va_arg(args, int);
                
                char buffer[16] = {0};
                int n = ARRAY_COUNT(buffer);
                int counter = 0;

                b32 is_negative = (number < 0);
                if (is_negative) number *= -1;
                
                if (number == 0)
                {
                    buffer[--n] = '0';
                    counter += 1;
                }
                else
                {
                    while (number)
                    {
                        buffer[--n] = '0' + number % 10;
                        number = number / 10;
                        counter += 1;
                    }
                }
                if (is_negative) {
                    buffer[--n] = '-';
                    counter += 1;
                }

                print_n(buffer + n, counter);
                start = index + 2;
                index = index + 2;
            }
            else
            {
                index += 1;
            }
        }
        else
        {
            index += 1;
        }
    }

    print_n(fmt + start, index - start + 1);

    va_end(args); 
}

