#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#define DEFAULT_STYLE 7
#define FUN_PRINTF_COLOR(colorName, style) \
int colorName(const char* stuff, ...) \
{ \
    /*if(hConsole == 0)*/ \
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
\
    SetConsoleTextAttribute(hConsole, style); \
\
    va_list args; \
    va_start(args, stuff); \
    int x = vprintf(stuff, args); \
    va_end(args); \
\
    SetConsoleTextAttribute(hConsole, 7); \
    return x;\
}

HANDLE hConsole;

FUN_PRINTF_COLOR(printf_red, 12)
FUN_PRINTF_COLOR(printf_magenta, 13)
FUN_PRINTF_COLOR(printf_yellow, 14)
FUN_PRINTF_COLOR(printf_green, 10)
FUN_PRINTF_COLOR(printf_cyan, 11)
FUN_PRINTF_COLOR(printf_blue, 9)
FUN_PRINTF_COLOR(printf_highlight, 249);


/**
 * printcf formatting codes
 *  &C - print cyan
 *  &R - print red
 *  &Y - print yellow
 *  &B - print blue
 *  &G - print green
 *  &N - print normal color
 *
 *  &i  - print int in current color
 *  &lld - print long long in current color
 */

 //FORMAT must not contain invlalid use of %
void printcf(char* format, ...)
{
    //format may be in read-only memory, so we copy it first.
    char* format2 = malloc(strlen(format) * sizeof(char));
    strcpy(format2, format);

    va_list args;
    va_start(args, format);

    int (*fprintf) (const char* format2, ...) = &printf;

    char* p1 = format2;
    char* p2 = format2;

    while(p2[0] != '\0')
    {
        if(p2[0] == '%')
        {
            if(p2[1] == '%')
            {
                p2++;
            }
            else if(p2[1] == 'i')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                (*fprintf)("%i", va_arg(args, int));
                p1 = p2+2;
                p2 = p2+2;
            }
            else if(p2[1] == 'l' && p2[2] == 'l' && p2[3] == 'd')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                (*fprintf)("%lld", va_arg(args, long long));
                p1 = p2+4;
                p2 = p2+4;
            }
            else if(p2[1] == 'N')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                p1 = p2+2;
                p2 = p2+2;

                fprintf = &printf;
            }
            else if(p2[1] == 'C')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                p1 = p2+2;
                p2 = p2+2;

                fprintf = &printf_cyan;
            }
            else if(p2[1] == 'R')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                p1 = p2+2;
                p2 = p2+2;

                fprintf = &printf_red;
            }
            else if(p2[1] == 'Y')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                p1 = p2+2;
                p2 = p2+2;

                fprintf = &printf_yellow;
            }
            else if(p2[1] == 'G')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                p1 = p2+2;
                p2 = p2+2;

                fprintf = &printf_green;
            }
            else if(p2[1] == 'M')
            {
                p2[0] = '\0';
                (*fprintf)(p1);
                p1 = p2+2;
                p2 = p2+2;

                fprintf = &printf_magenta;
            }
        }
        else
        {
            p2++;
        }
    }

    (*fprintf)(p1);

    va_end(args);

    free(format2);
}

void print_all_styles()
{
    if(hConsole == 0)
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    int k;
    for(k = 0; k < 256; k++)
    {
        SetConsoleTextAttribute(hConsole, k);
        printf("style %i",k);
    }

    SetConsoleTextAttribute(hConsole, DEFAULT_STYLE);
}
