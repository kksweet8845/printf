#include "printf.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>



static const uint8_t jump_table[] = {
    /* ' ' */ 3,    /* '!' */ 0,    /* '"' */   0,    /* (hash) */ 5,
    /* '$' */ 0,    /* '%' */ 16,    /* '&' */   0,    /* '\''    */ 6,
    /* '(' */ 0,    /* ')' */ 0,    /* '*' */   7,    /* '+'    */ 2,
    /* ',' */ 0,    /* '-' */ 1,    /* '.' */   9,    /* '/'    */ 0,
    /* '0' */ 4,    /* '1' */ 8,    /* '2' */   8,    /* '3'    */ 8,
    /* '4' */ 8,    /* '5' */ 8,    /* '6' */   8,    /* '7'    */ 8,
    /* '8' */ 8,    /* '9' */ 8,    /* ':' */   0,    /* ';'    */ 0,
    /* '<' */ 0,    /* '=' */ 0,    /* '>' */   0,    /* '?'    */ 0,
    /* '@' */ 0,    /* 'A' */ 29,    /* 'B' */   0,    /* 'C'    */ 27,
    /* 'D' */ 0,    /* 'E' */ 20,    /* 'F' */  19,    /* 'G'    */ 21,
    /* 'H' */ 0,    /* 'I' */ 0,    /* 'J' */  0,    /* 'K'    */ 0,
    /* 'L' */ 12,   /* 'M' */ 0,    /* 'N' */ 0,    /* 'O'    */ 0,
    /* 'P' */ 0,    /* 'Q' */ 0,    /* 'R' */ 0,    /* 'S'    */ 25,
    /* 'T' */ 0,    /* 'U' */ 0,    /* 'V' */ 0,    /* 'W'    */ 0,
    /* 'X' */ 22,    /* 'Y' */ 0,    /* 'Z' */ 0,    /* '['    */ 0,
    /* '\' */  0,    /* ']' */  0,    /* '^' */  0,    /* '_'    */  0,
    /* '`' */  0,    /* 'a' */ 29,    /* 'b' */  0,    /* 'c'    */ 26,
    /* 'd' */  17,    /* 'e' */  20,    /* 'f' */  19,  /* 'g'    */  21,
    /* 'h' */ 10,    /* 'i' */ 17,    /* 'j' */ 14,    /* 'k'    */ 0,
    /* 'l' */  11,    /* 'm' */ 31,    /* 'n' */ 30,    /* 'o'    */ 23,
    /* 'p' */  28,    /* 'q' */ 0,    /* 'r' */ 0,    /* 's'    */  24,
    /* 't' */  15,    /* 'u' */  18,    /* 'v' */ 0,    /* 'w'    */ 0,
    /* 'x' */  22,    /* 'y' */ 0,    /* 'z' */  13
};



#define LABEL(name) do_##name
#define REF(label) &&do_##label
#define JUMP_TABLE_TYPE const void*
#define L_(ch) L##ch

#define NOT_IN_JUMP_TABLE(ch) ((int)(ch) < L_(' ') || (int)(ch) > L_('z') )
#define CHAR_CLASS(ch) (jump_table[(int)(ch) - L_(' ')])
#define JUMP(expr, table)     \
    do \
    { \
        void* ptr; \
        wchar_t spec = (expr); \
        ptr = NOT_IN_JUMP_TABLE(spec) ? REF(form_unknown) : table[CHAR_CLASS(spec)]; \
        goto *ptr; \
    }    \
    while(0)



#define STEP0_3_TABLE \
    /* In the beginning after read the prefix '%'  */\
    static JUMP_TABLE_TYPE step0[] = { \
        REF(form_unknown), /* unknown character */  \
        REF(flag_minus),  /* '-' */                 \
        REF(flag_plus),  /* '+' */                  \
        REF(flag_space), /* ' ' */                  \
        REF(flag_zero),  /* 0  */     \
        REF(flag_hash), /* '#' */ \
        REF(flag_quote), /* '\'' */ \
        REF(asterisk), /* '*' */ \
        REF(width), /* '[1-9]'*/\
        REF(precision),  /* '.' */ \
        REF(mod_half), /* 'hh' from char */ \
        REF(mod_long), /* 'h' from short*/ \
        REF(mod_longlong), /* 'L', long double sized*/ \
        REF(mod_z), /* sized_t sized */ \
        REF(mod_j), /* intmax_t sized */ \
        REF(mod_t), /* ptrdiff_t-sized */ \
        REF(percent), /* percent */\
        REF(type_int), /* d, i */\
        REF(type_unsigned), /* u */\
        REF(type_float), /* %fF */ \
        REF(type_floatExpo), /* %eE*/ \
        REF(type_floatAlter), /* %gG */ \
        REF(type_hex), /* %x %X*/\
        REF(type_octal), /* %o */\
        REF(type_string), /* string */\
        REF(type_wString), /* alias for %ls */ \
        REF(type_char), /* %c */\
        REF(type_wchar), /* %C */\
        REF(type_pointer), /* void* */\
        REF(type_floatHex), /* %aA */\
        REF(type_n), /* %n */\
        REF(type_strerrno), /* %m */ \
    };  \
    /* after processing width */ \
    static JUMP_TABLE_TYPE step1[] = { \
        REF(form_unknown), /* unknown character */  \
        REF(form_unknown),  /* '-' */                 \
        REF(form_unknown),  /* '+' */                  \
        REF(form_unknown), /* ' ' */                  \
        REF(form_unknown),  /* '0' */     \
        REF(form_unknown), /* '#' */ \
        REF(form_unknown), /* '\'' */ \
        REF(form_unknown), /* '*' */ \
        REF(form_unknown), /* '[1-9]'*/\
        REF(precision),  /* '.' */ \
        REF(mod_half), /* 'hh' from char */ \
        REF(mod_long), /* 'h' from short*/ \
        REF(mod_longlong), /* 'L', long double sized*/ \
        REF(mod_z), /* sized_t sized */ \
        REF(mod_j), /* intmax_t sized */ \
        REF(mod_t), /* ptrdiff_t-sized */ \
        REF(percent), /* percent */\
        REF(type_int), /* d, i */\
        REF(type_unsigned), /* u */\
        REF(type_float), /* %fF */ \
        REF(type_floatExpo), /* %eE*/ \
        REF(type_floatAlter), /* %gG */ \
        REF(type_hex), /* %x %X*/\
        REF(type_octal), /* %o */\
        REF(type_string), /* string */\
        REF(type_wString), /* alias for %ls */ \
        REF(type_char), /* %c */\
        REF(type_wchar), /* %C */\
        REF(type_pointer), /* void* */\
        REF(type_floatHex), /* %aA */\
        REF(type_n), /* %n */\
        REF(type_strerrno), /* %m */ \
    };  \
    /* after processing precision */ \
    static JUMP_TABLE_TYPE step2[] = { \
        REF(form_unknown), /* unknown character */  \
        REF(form_unknown),  /* '-' */                 \
        REF(form_unknown),  /* '+' */                  \
        REF(form_unknown), /* ' ' */                  \
        REF(form_unknown),  /* 0  */     \
        REF(form_unknown), /* '#' */ \
        REF(form_unknown), /* '\'' */ \
        REF(form_unknown), /* '*' */ \
        REF(form_unknown), /* '[1-9]'*/\
        REF(form_unknown),  /* '.' */ \
        REF(mod_half), /* 'hh' from char */ \
        REF(mod_long), /* 'l' from short*/ \
        REF(mod_longlong), /* 'L', long double sized*/ \
        REF(mod_z), /* sized_t sized */ \
        REF(mod_j), /* intmax_t sized */ \
        REF(mod_t), /* ptrdiff_t-sized */ \
        REF(percent), /* percent */\
        REF(type_int), /* d, i */\
        REF(type_unsigned), /* u */\
        REF(type_float), /* %fF */ \
        REF(type_floatExpo), /* %eE*/ \
        REF(type_floatAlter), /* %gG */ \
        REF(type_hex), /* %x %X*/\
        REF(type_octal), /* %o */\
        REF(type_string), /* string */\
        REF(type_wString), /* alias for %ls */ \
        REF(type_char), /* %c */\
        REF(type_wchar), /* %C */\
        REF(type_pointer), /* void* */\
        REF(type_floatHex), /* %aA */\
        REF(type_n), /* %n */\
        REF(type_strerrno), /* %m */ \
    };  \
    /* after processing first l*/ \
    static JUMP_TABLE_TYPE step3a[] = { \
        REF(form_unknown), /* unknown character */  \
        REF(form_unknown),  /* '-' */                 \
        REF(form_unknown),  /* '+' */                  \
        REF(form_unknown), /* ' ' */                  \
        REF(form_unknown),  /* 0  */     \
        REF(form_unknown), /* '#' */ \
        REF(form_unknown), /* '\'' */ \
        REF(form_unknown), /* '*' */ \
        REF(form_unknown), /* '[1-9]'*/\
        REF(form_unknown),  /* '.' */ \
        REF(form_unknown), /* 'hh' from char */ \
        REF(mod_long), /* 'l' from short*/ \
        REF(form_unknown), /* 'L', long double sized*/ \
        REF(form_unknown), /* sized_t sized */ \
        REF(form_unknown), /* intmax_t sized */ \
        REF(form_unknown), /* ptrdiff_t-sized */ \
        REF(form_unknown), /* percent */\
        REF(type_int), /* d, i */\
        REF(type_unsigned), /* u */\
        REF(type_float), /* %fF */ \
        REF(type_floatExpo), /* %eE*/ \
        REF(type_floatAlter), /* %gG */ \
        REF(type_hex), /* %x %X*/\
        REF(type_octal), /* %o */\
        REF(type_string), /* string */\
        REF(type_wString), /* alias for %ls */ \
        REF(type_char), /* %c */\
        REF(type_wchar), /* %C */\
        REF(type_pointer), /* void* */\
        REF(type_floatHex), /* %aA */\
        REF(type_n), /* %n */\
        REF(type_strerrno), /* %m */ \
    };  \
    /* After processing first h*/ \
    static JUMP_TABLE_TYPE step3b[] = { \
        REF(form_unknown), /* unknown character */  \
        REF(form_unknown),  /* '-' */                 \
        REF(form_unknown),  /* '+' */                  \
        REF(form_unknown), /* ' ' */                  \
        REF(form_unknown),  /* 0  */     \
        REF(form_unknown), /* '#' */ \
        REF(form_unknown), /* '\'' */ \
        REF(form_unknown), /* '*' */ \
        REF(form_unknown), /* '[1-9]'*/\
        REF(form_unknown),  /* '.' */ \
        REF(mod_half), /* 'hh' from char */ \
        REF(form_unknown), /* 'h' from short*/ \
        REF(form_unknown), /* 'L', long double sized*/ \
        REF(form_unknown), /* sized_t sized */ \
        REF(form_unknown), /* intmax_t sized */ \
        REF(form_unknown), /* ptrdiff_t-sized */ \
        REF(form_unknown), /* percent */\
        REF(type_int), /* d, i */\
        REF(type_unsigned), /* u */\
        REF(form_unknown), /* %fF */ \
        REF(form_unknown), /* %eE*/ \
        REF(form_unknown), /* %gG */ \
        REF(type_hex), /* %x %X*/\
        REF(type_octal), /* %o */\
        REF(form_unknown), /* string */\
        REF(form_unknown), /* alias for %ls */ \
        REF(type_char), /* %c */\
        REF(form_unknown), /* %C */\
        REF(form_unknown), /* void* */\
        REF(form_unknown), /* %aA */\
        REF(type_n), /* %n */\
        REF(form_unknown), /* %m */ \
    };  \


#define STEP4_TABLE \
    static JUMP_TABLE_TYPE step4[] = { \
        REF(form_unknown), /* unknown character */  \
        REF(form_unknown),  /* '-' */                 \
        REF(form_unknown),  /* '+' */                  \
        REF(form_unknown), /* ' ' */                  \
        REF(form_unknown),  /* 0  */     \
        REF(form_unknown), /* '#' */ \
        REF(form_unknown), /* '\'' */ \
        REF(form_unknown), /* '*' */ \
        REF(form_unknown), /* '[1-9]'*/\
        REF(form_unknown),  /* '.' */ \
        REF(form_unknown), /* 'hh' from char */ \
        REF(form_unknown), /* 'h' from short*/ \
        REF(form_unknown), /* 'L', long double sized*/ \
        REF(form_unknown), /* sized_t sized */ \
        REF(form_unknown), /* intmax_t sized */ \
        REF(form_unknown), /* ptrdiff_t-sized */ \
        REF(percent), /* percent */\
        REF(type_int), /* d, i */\
        REF(type_unsigned), /* u */\
        REF(type_float), /* %fF */ \
        REF(type_floatExpo), /* %eE*/ \
        REF(type_floatAlter), /* %gG */ \
        REF(type_hex), /* %x %X*/\
        REF(type_octal), /* %o */\
        REF(type_string), /* string */\
        REF(type_wString), /* alias for %ls */ \
        REF(type_char), /* %c */\
        REF(type_wchar), /* %C */\
        REF(type_pointer), /* void* */\
        REF(type_floatHex), /* %aA */\
        REF(type_n), /* %n */\
        REF(type_strerrno), /* %m */ \
    };



#define LEFT 0
#define RIGHT 1
#define PADDING 2
#define SPACE 3

#define INT 0
#define UINT 1
#define FLOAT 2
#define STRING 3
#define WSTRING 4
#define CHAR 5
#define WCHAR 6
#define POINTER 7

#define BASE_10 0
#define BASE_8 1
#define BASE_16 2

#define NORMAL 0
#define SCIENCE 1
#define EITHER 2


union Container {
    unsigned long long int longlong;
    unsigned long int word;
} number;

static char buffer[100];


#define MSB(num) (((int)num & 0x80000000) >> (sizeof(num)<<2))
#define Complement(num) ((~num) + 1)
#define DOUBLE_EXP_PART(d) ((d & 0x7ff0000000000000) >> 52) - 1023

void print_buffer() {

    char* tmp = buffer;
    fputc(" ", stdout);
    for(int i=0;i<100;i++, tmp++){
        if(*tmp == '\0') fputc('@', stdout);
        else {
            fputc(*tmp, stdout);
        }
    }
    fputc('\n', stdout);
}

#ifdef DEBUG
#define outString(str, width, pos, padding) \
    do { \
        print_buffer(); \
        int len = strlen(str); \
        int diff = width - len; \
        char* _ = str; \
        if(pos == RIGHT) { \
            while(diff-- > 0) { fputc(((padding) ? '0' : ' '), stdout); } \
            while(*_ != '\0') { fputc(*(_++), stdout); }\
        } else {\
            while(*_ != '\0' ) { fputc(*(_++), stdout); } \
            while(diff-- > 0) { fputc(((padding) ? '0' : ' '), stdout); } \
        }\
    }while(0)

#else
#define outString(str, width, pos, padding) \
    do { \
        int len = strlen(str); \
        int diff = width - len; \
        char* cur = str; \
        if(pos == RIGHT) { \
            while(diff-- > 0) { fputc(((padding) ? '0' : ' '), stdout); } \
            while(*cur != '\0') { fputc(*cur++, stdout); }\
        } else {\
            while(*cur != '\0' ) { fputc(*cur++, stdout); } \
            while(diff-- > 0) { fputc(((padding) ? '0' : ' '), stdout); } \
        }\
    }while(0)
#endif




void itowa( unsigned int num, unsigned int base, int uppercase, char** addr){

    char* cur = *addr;

    do {
        unsigned int r = num % base;
        num /= base;

        if(uppercase)
            *--cur = r >= 10 ? r - 10 + 'A' : r + '0';
        else
            *--cur = r >= 10 ? r - 10 + 'a' : r + '0';
    }while(num > 0);

    *addr = cur;
}

void itoa( unsigned long long int num, unsigned int base, int uppercase, char** addr){

    char* cur = *addr;

    do {
        unsigned int r = num % base;
        num /= base;

        if(uppercase)
            *--cur = r >= 10 ? r - 10 + 'A' : r + '0';
        else
            *--cur = r >= 10 ? r - 10 + 'a' : r + '0';
    }while(num > 0);

    *addr = cur;
}

void ftoa( double num, unsigned int base, int uppercase, char** addr ) {

    int intpart = (int)num;
    double floatpart = num - (int) num;

    char* cur = *addr;
    do {
        int tmp = floatpart * base;
        floatpart = tmp - (int) tmp;
        if(uppercase){
            *--cur = tmp >= 10 ? tmp-10 + 'A' : tmp + '0';
        } else {
            *--cur = tmp >= 10 ? tmp-10 + 'a' : tmp + '0';
        }
    } while(floatpart != 0);


    *--cur = '.';

    int is_negative = MSB(intpart);
    intpart = is_negative ? Complement(intpart) : intpart;
    itowa(intpart, base, uppercase, &cur);

    if(is_negative){
        *--cur = '-';
    }

    *addr = cur;

}

int frexp10(int intpart, double floatpart) {
    int int_ = 0, float_ = 0;
    while(intpart > 10){
        int_++;
        intpart /= 10;
    }
    while(floatpart < 1){
        float_++;
        floatpart *= 10;
    }
    return intpart - floatpart;
}


void ftoe( double num, unsigned int base, int uppercase, char** addr) {

    int intpart = (int) num;
    double floatpart = num - (int) num;

    char* cur = *addr;

    int exp = frexp10(intpart, floatpart);
    int is_negative = MSB(exp);
    exp = is_negative ? Complement(exp) : exp;

    itowa( exp, base, uppercase, &cur);

    if(is_negative)
        *--cur = '-';

    *--cur = uppercase ? 'E' : 'e';

    do {
        int tmp = floatpart * base;
        floatpart = tmp - (int) tmp;
        if(uppercase) {
            *--cur = tmp >= 10 ? tmp - 10 + 'A' : tmp + '0';
        } else {
            *--cur = tmp >= 10 ? tmp - 10 + 'a' : tmp + '0';
        }
    }while(floatpart != 0);

    is_negative = MSB(intpart);
    intpart = is_negative ? Complement(intpart) : intpart;

    while(intpart > 10) {
        int tmp = intpart % base;
        intpart /= base;

        if(uppercase) {
            *--cur = tmp >= 10 ? tmp - 10 + 'A' : tmp + '0';
        } else {
            *--cur = tmp >= 10 ? tmp - 10 + 'a' : tmp + '0';
        }
    }

    *--cur = '.';
    *--cur = intpart + '0';

    if(is_negative){
        *--cur = '-';
    }

    *addr = cur;
}


void ftog(double num, unsigned int base, int uppercase, char** addr) {

    int intpart = (int) num;
    double floatpart = num - (int) num;
    int exp = frexp10(intpart, floatpart);

    if(exp >= 5) {
        ftoe(num, base, uppercase, addr);
    } else if(exp <= -5) {
        ftoe(num, base, uppercase, addr);
    } else {
        ftoa(num, base, uppercase, addr);
    }

}




static int
read_int (const unsigned char* *pstr)
{
  int retval = **pstr - L_('0');

  while (isdigit (*++(*pstr)))
    if (retval >= 0)
      {
	if (INT_MAX / 10 < retval)
	  retval = -1;
	else
	  {
	    int digit = **pstr - L_('0');

	    retval *= 10;
	    if (INT_MAX - digit < retval)
	      retval = -1;
	    else
	      retval += digit;
	  }
      }

  return retval;
}


void _printf(const uint8_t* format, ...) {

    va_list args;

    va_start(args, format);


    uint8_t *cur = format;

    STEP0_3_TABLE
    STEP4_TABLE

    if(*cur == '\0') {
        goto all_done;
    }

    int** ncharptr_ptr = NULL;
    int written_char_num = 0;
    union
      {
	unsigned long long int longlong;
	unsigned long int word;
      } number;


    do {

        int length = 0;
        int width = 0;
        int precision = 0;
        int uppercase = 0;
        int pos = RIGHT;
        int type = 0;
        int notation = NORMAL;
        int base = BASE_10;
        int write_char = 0;
        int is_space = 0;
        unsigned char padding = 0;
        unsigned char alter = 0;
        unsigned char *tmp;

        while(*(cur) != '%' && *(cur) != '\0') { fputc(*(cur++), stdout); written_char_num+= 1; }

        if(*cur == '\0') {
            goto all_done;
        }

        JUMP((*++cur), step0);


        LABEL(form_unknown):
            JUMP(*++cur, step0);

        LABEL(flag_minus):
            pos = LEFT;
            // TODO

            goto do_width;

        LABEL(flag_plus):
            pos = RIGHT;
            // TODO

            goto do_width;

        LABEL(flag_space):
            pos = RIGHT;
            is_space = 1;

            goto do_width;

        LABEL(flag_zero):
            pos = RIGHT;
            padding = 1;

            goto do_width;

        LABEL(flag_hash):

            pos = RIGHT;
            alter = 1;

            goto do_width;

        LABEL(flag_quote):
            pos = RIGHT;
            goto do_width;

        LABEL(asterisk):

            goto do_width;
        LABEL(width):

            tmp = ++cur;
            if(isdigit(*tmp)) {
                width = read_int(&cur);
                if(width == -1){
                    // TODO Not a valid length
                }
            }

            JUMP((*cur), step1);

        LABEL(precision):

            tmp = ++cur;

            if( isdigit(*tmp) ) {
                precision = read_int(&cur);
                if(precision == -1){
                    // TODO Not a valid precision
                }
            }

            JUMP((*cur), step2);

        LABEL(mod_half):

            length = (length == 0) ? 2 : (length >> 1);

            JUMP((*++cur), step3b);

        LABEL(mod_long):
            length = (length == 0) ? 4 : (length << 1);

            JUMP((*++cur), step3a);

        LABEL(mod_longlong):
            length = 8;

            JUMP((*++cur), step4);

        LABEL(mod_z):
            length = sizeof(size_t);

            JUMP((*++cur), step4);

        LABEL(mod_j):
            length = sizeof(intmax_t);

            JUMP((*++cur), step4);

        LABEL(mod_t):
            length = sizeof(__darwin_ptrdiff_t);

            JUMP((*++cur), step4);

        LABEL(percent):
            goto finish_parsing;

        LABEL(type_int):

            type = INT;
            goto finish_parsing;

        LABEL(type_unsigned):
            type = UINT;
            goto finish_parsing;

        LABEL(type_float):

            type = FLOAT;
            notation = NORMAL;
            base = BASE_10;

            goto finish_parsing;

        LABEL(type_floatExpo):

            type = FLOAT;
            notation = SCIENCE;
            base = BASE_10;

            goto finish_parsing;


        LABEL(type_floatAlter):

            type = FLOAT;
            notation = EITHER;
            base = BASE_10;

            goto finish_parsing;

        LABEL(type_hex):

            type = UINT;
            notation = NORMAL;
            if(*cur == 'x') uppercase = 0;
            else if(*cur == 'X') uppercase = 1;

            base = BASE_16;

            goto finish_parsing;

        LABEL(type_octal):

            type = UINT;
            notation = NORMAL;
            base = BASE_8;

            goto finish_parsing;


        LABEL(type_string):

            type = STRING;

            goto finish_parsing;


        LABEL(type_wString):
            goto finish_parsing;

        LABEL(type_char):

            goto finish_parsing;

        LABEL(type_wchar):
            goto finish_parsing;

        LABEL(type_pointer):

            notation = BASE_16;
            alter = 1;
            goto finish_parsing;

        LABEL(type_floatHex):

            notation = BASE_16;

            goto finish_parsing;

        LABEL(type_n):
            write_char = 1;
            goto finish_parsing;

        LABEL(type_strerrno):

            // TODO write errno
            goto finish_parsing;


    finish_parsing:
            do {
                memset(buffer, '\0', sizeof(buffer));
                char* string = buffer + sizeof(buffer) - 1;
                unsigned char is_longlong = 0;
                unsigned char is_negative = 0;
                int diff;
                switch(*(cur)){
                    case '%': /* Print literal % */
                        fputc('%', stdout);
                        break;
                    case 'd':
                    case 'i':
                        switch(length) {
                            case 0:
                                number.word = va_arg(args, int);
                                break;
                            case 1:
                                number.word = va_arg(args, int8_t);
                                break;
                            case 2:
                                number.word = va_arg(args, short);
                                break;
                            case 4:
                                number.word = va_arg(args, int);
                                break;
                            case 8:
                                number.longlong = va_arg(args, int64_t);
                                is_longlong = 1;
                                break;
                        }
                        if(is_longlong) {
                            is_negative = MSB(number.longlong);
                            number.longlong = is_negative ? Complement(number.longlong) : number.longlong;
                            itoa(number.longlong, 10, uppercase, &string);
                        } else {
                            is_negative = MSB(number.word);
                            number.longlong = is_negative ? Complement(number.longlong) : number.longlong;
                            itowa(number.word, 10, uppercase, &string);
                        }
                        if(is_space || is_negative)
                            --width;
                        if(is_negative){
                            fputc('-', stdout);
                        } else if( is_space ) {
                            fputc(' ', stdout);
                        }
                        outString(string, width, pos, padding);
                        break;
                    case 'u':
                        switch(length) {
                            case 0:
                                number.word = va_arg(args, uint32_t);
                                break;
                            case 1:
                                number.word = va_arg(args, uint8_t);
                                break;
                            case 2:
                                number.word = va_arg(args, uint16_t);
                                break;
                            case 4:
                                number.word = va_arg(args, uint32_t);
                                break;
                            case 8:
                                number.longlong = va_arg(args, uint64_t);
                                is_longlong = 1;
                                break;
                        }
                        if(is_longlong) {
                            itoa(number.longlong, 10, uppercase, &string);
                        } else {
                            itowa(number.word, 10, uppercase, &string) ;
                        }
                        if(is_space)
                            --width;
                        if(is_space)
                            fputc(' ', stdout);
                        outString(string, width, pos, padding);
                        break;
                    case 'f':
                    case 'F':
                        number.longlong = va_arg(args, double);
                        ftoa(number.longlong, 10, uppercase, &string);
                        if(is_space || is_negative)
                            --width;
                        outString(string, width, pos, padding);
                        break;
                    case 'e':
                    case 'E':
                        number.longlong = va_arg(args, double);
                        ftoe(number.longlong, 10, uppercase, &string);
                        if(is_space || is_negative)
                            --width;
                        outString(string, width, pos, padding);
                        break;
                    case 'g': case 'G':
                        number.longlong = va_arg(args, double);
                        ftog(number.longlong, 10, uppercase, &string);
                        if(is_space || is_negative)
                            --width;
                        outString(string, width, pos, padding);
                        break;
                    case 'x':
                    case 'X':
                        switch(length) {
                            case 0:
                                number.word = va_arg(args, int32_t);
                                break;
                            case 1:
                                number.word = va_arg(args, int8_t);
                                break;
                            case 2:
                                number.word = va_arg(args, int16_t);
                                break;
                            case 4:
                                number.word = va_arg(args, int32_t);
                                break;
                            case 8:
                                number.longlong = va_arg(args, int64_t);
                                is_longlong = 1;
                                break;
                        }
                        if(is_longlong) {
                            itoa(number.longlong, 16, uppercase, &string);
                        } else {
                            itowa(number.word, 16, uppercase, &string);
                        }
                        if(alter){
                            *--string = (uppercase) ? 'X' : 'x';
                            *--string = '0';
                        }
                        if(is_space || is_negative)
                            --width;
                        outString(string, width, pos, padding);
                        break;
                    case 'o':
                        switch(length) {
                            case 0:
                                number.word = va_arg(args, int32_t);
                                break;
                            case 1:
                                number.word = va_arg(args, int8_t);
                                break;
                            case 2:
                                number.word = va_arg(args, int16_t);
                                break;
                            case 4:
                                number.word = va_arg(args, int32_t);
                                break;
                            case 8:
                                number.longlong = va_arg(args, int64_t);
                                is_longlong = 1;
                                break;
                        }
                        if(is_longlong) {
                            itoa(number.longlong, 8, uppercase, &string);
                        } else {
                            itowa(number.word, 8, uppercase, &string);
                        }
                        if(alter){
                            *--string = (uppercase) ? 'X' : 'x';
                            *--string = '0';
                        }
                        if(is_space || is_negative)
                            --width; \
                        outString(string, width, pos, padding);
                        break;
                    case 's':
                    case 'S':
                        number.longlong = va_arg(args, void*);
                        string = number.longlong;
                        outString(string, width, pos, 0);
                        break;
                    case 'c':
                    case 'C':
                        number.word = va_arg(args, char);
                        fputc((char) number.word, stdout);
                        break;
                    case 'p' :
                        number.longlong = va_arg(args, void*);
                        itoa(number.longlong, 16, uppercase, &string);
                        printf("%s\n", string);
                        *--string = (uppercase) ? 'X' : 'x';
                        *--string = '0';
                        outString(string, width, pos, padding);
                        break;
                    case 'a':
                    case 'A':
                        /* TODO */
                        break;
                    case 'n' :
                        /* TODO */
                        break;
                    case 'm' :
                        /* TODO */
                        break;
                }
            } while(0);

    } while(*++cur != '\0');

    all_done:
    va_end(args);
    return;

}
