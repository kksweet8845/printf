/**
 * Author: Nober
 * Desc  : Implement printf fn
 */




#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>


#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */


#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */



void itowa(unsigned int, unsigned int, int, char**);
void itoa(unsigned long long int, unsigned int, int, char**);
void ftoa(double, unsigned int, int, char**);
void ftoe(double, unsigned int, int, char**);
void ftog(double, unsigned int, int, char**);
int frexp10(int, double);


void _printf(const uint8_t* format, ...);

// static int read_int(const unsigned char* *pstr);



#endif