#include "printf.h"


int main(void) {


    int a = 18;

    _printf("%%d %d\n", a);
    _printf("%%x %04x\n", a);
    _printf("%%X %#X\n", a);

    char* str = "Happy Birthday";
    _printf("%s\n", str);

    // _printf("StartAddress  |heapSTRUCT_SIZE  |xBlockSize   |EndAddress\n\r");
    // _printf("%-14p %-17d %-13d %-10p\n", &a, 2000, 198810, &a);
    _printf("%p \n", 2000);


    return 0;
}