/*****************************************************************************
 *
 * Implementation of the formatting part of sprintf
 *
 ****************************************************************************/

/* Set the max number of digits and prefix characters */
#define digits_max (64 + 8)

#include "sformat.h"
// #include "../intsizes/intsizes.h"

#define FMT_FLAGS_ZERO 1
#define FMT_FLAGS_SPACE 2
#define FMT_FLAGS_MINUS 4
#define FMT_FLAGS_PLUS 8
#define FMT_FLAGS_HASH 16

#define FMT_LQUAL_h 1
#define FMT_LQUAL_hh 2
#define FMT_LQUAL_l 4
#define FMT_LQUAL_ll 8
#define FMT_LQUAL_L 16
#define FMT_LQUAL_z 32
#define FMT_LQUAL_j 64
#define FMT_LQUAL_t 128

/*****************************************************************************
 *
 * A structure to represent the formatting options
 *
 ****************************************************************************/

struct fmt_t
{
  int flags;
  int width;
  int precision;
  int lqual;
} fmt;

int stringlen(char *);
int digits_divide_gen(char *, unsigned long, unsigned, char);
int digits_shift_gen(char *, unsigned long, unsigned, char);
int prepend(char *, struct fmt_t *, int);
char *pad_align(char *, struct fmt_t *, int);
int sint_format(char *, struct fmt_t *, long, unsigned int);
int uint_format(char *, struct fmt_t *, unsigned long, unsigned int);
int power_of_2_format(char *, struct fmt_t *, unsigned long, char);
int text_format(char *, struct fmt_t *, char *);
int hex_format(char *, struct fmt_t *, unsigned int);
int octal_format(char *, struct fmt_t *, unsigned int);
// int float_format(char *, struct fmt_t *, double);
// int double_format(char *, struct fmt_t *, double);
// int exponential_format(char *, struct fmt_t *, double);
int string_format(char *, struct fmt_t *, char *);
int pointer_format(char *, struct fmt_t *, void *);


/*****************************************************************************
 *
 * String length utility
 *
 ****************************************************************************/

int stringlen(char *s) {
  unsigned int i = 0;

  for (i = 0; s[i] != 0; i++);

  return i;
}

/*****************************************************************************
 *
 * Digit generation
 *
 ****************************************************************************/

/* Write the digits of an unsigned integer right justified and return the
 * number of digits produced.
 *
 * Two functions are provided to produce digits by
 *    division - slow but general
 *    shifts - fast but limited to powers of two
 *
 * These functions are unusual in that they expect to be passed a
 * pointer to just beyond where the number is to be written. The digits
 * of the number will be written right-to-left starting from the
 * location immediately before that point and working towards lower
 * addresses as far as required. On completion the pointer will be
 * pointing at the leftmost digit of the number.
 *
 * The parameter char_ten is applicable only for
 * number bases greater than 10 and specifies how the
 * number ten is to be represented. It is usually
 * specified as 'a' or 'A' depending on whether lower or
 * upper case letters are needed.
 *
 */

/**********
 *
 * Generate digits by division - slow but general
 *
 *********/

int digits_divide_gen(char *past, unsigned long value,
   unsigned int base, char char_ten)
{
  unsigned long val = value;
  unsigned long quotient;
  unsigned long remainder;
  char *p = past;

  if (base < 2 || base > 32) return -1;
  do
  {
    quotient = val / base;
    remainder = val - quotient * base;

    if (remainder < 10)
    {
      *--p = remainder + '0';
    }
    else
    {
      *--p = remainder - 10 + char_ten;
    }
    val = quotient;

// consio_char_write('{');
// consio_char_write(*p);
// consio_char_write('}');

  } while (val);

// consio_char_write(10);

  return past - p;
}

/**********
 *
 * Generate digits by shifting - fast but limited to bases 2, 4, 8,
 * 16 and 32 (by using shift amounts 1 to 5)
 *
 *********/

int digits_shift_gen(char *past, unsigned long value,
   unsigned int shift, char char_ten)
{
  unsigned long val = value;
  unsigned long mask;
  unsigned int digit;
  char *p = past;

  if (shift < 1 || shift > 5) return -1;

  mask = (1 << shift) - 1;
  do
  {
    digit = val & mask;




    if (digit < 10)
    {
      *--p = digit + '0';
    }
    else
    {
      *--p = digit - 10 + char_ten;
    }


// consio_char_write('{');
// consio_char_write(*p);
// consio_char_write('}');


    val >>= shift;
  } while (val);

// consio_char_write(10);

  return past - p;
}

/*****************************************************************************
 *
 * Leading sign or place for a sign on numbers
 *
 ****************************************************************************/

int prepend(char *ptr, struct fmt_t *f, int sign_char)
{
  char *p = ptr;

  if (f->flags & FMT_FLAGS_SPACE) *--p = ' ';
  if (f->flags & FMT_FLAGS_PLUS) *--p = sign_char;
  return ptr - p;
}


/*****************************************************************************
 *
 * Number alignment and padding
 *
 ****************************************************************************/

char *pad_align(char *p, struct fmt_t *f, int padding_reqd)
{
  char pad_char = ' '; /* Default to a space */

  if (f->flags & FMT_FLAGS_MINUS)
  {
    /* Told to left justify so no padding needed */
  }
  else
  {
    if (f->flags & FMT_FLAGS_ZERO) pad_char = '0';
    for (; padding_reqd-- > 0;) *p++ = pad_char;
  }
  return p;
}


/*****************************************************************************
 *
 * Format an unsigned integer
 *
 ****************************************************************************/

int uint_format(char *destination, struct fmt_t *f,
   unsigned long value, unsigned int base)
{
  int i;
  int qdigits;
  int qzeroes;
  int qspaces;
  int qsigns;
  char sign;
  char *dest = destination;
  char *p; /* Pointer into buffer */
  char buffer[digits_max];

  /* The order of printing will be as follows
   *   spaces - zero or more
   *   a sign - zero or one
   *   zeroes - zero or more
   *   the digits of the number
   * The only required field is the digits
   */

  /* Generate the digits and find the quantity of them */
  qdigits = digits_divide_gen(buffer + digits_max, value, base, 'a');
  if (qdigits == -1) return -1;
  p = buffer + digits_max - qdigits;

  /* If flags includes a space or a plus then reserve a sign position */
  qsigns = 0;
  if (f->flags & FMT_FLAGS_SPACE) {
    qsigns = 1;
    sign = ' ';
  }
  if (f->flags & FMT_FLAGS_PLUS) { /* Plus takes precedence over space */
    qsigns = 1;
    sign = '+';
  }

  /* Reserve space for leading zeroes */
  qzeroes = 0;
  if (f->flags & FMT_FLAGS_ZERO) {
    qzeroes = f->width - qdigits - qsigns;
    if (qzeroes < 0) {
      qzeroes = 0;
    }
  }

  /* Reserve space for leading spaces */
  qspaces = 0;
  qspaces = f->width - qdigits - qsigns - qzeroes;
  if (qspaces < 0) {
    qspaces = 0;
  }

  /* Write the fully formatted number */
  for (i = 0; i < qspaces; i++) *dest++ = ' ';
  if (qsigns != 0) *dest++ = sign;
  for (i = 0; i < qzeroes; i++) *dest++ = '0';
  for (i = 0; i < qdigits; i++) *dest++ = *p++;

  /* Return the number of characters written */
  return dest - destination;
}

/*****************************************************************************
 *
 * Format a signed integer
 *
 ****************************************************************************/

int sint_format(char *destination, struct fmt_t *f,
   long value, unsigned int base) {
  int i;
  unsigned long uvalue;
  int sbase = base;
  int qdigits;
  int qzeroes;
  int qspaces;
  int qsigns;
  char sign;
  char digit;
  char *dest = destination;
  char *p; /* Pointer into buffer */
  char buffer[digits_max];

  /* Generate the digits and find the quantity of them */
  if (value < 0 && value == - value) {
    /* Special case: number cannot be negated */
    digit = value % base;
    uvalue = - (value / sbase);
//printf("Val=%i,", value);
//printf("Dig=%i,", digit);
//printf("Uval=%i\n", uvalue);
    buffer[digits_max - 1] = digit + '0';
    qdigits = digits_divide_gen(buffer + digits_max - 1, uvalue, base,
       'a') + 1;
  }
  else {
    uvalue = value;
    if (value < 0) uvalue = - value;
    qdigits = digits_divide_gen(buffer + digits_max, uvalue, base, 'a');
  }
  p = buffer + digits_max - qdigits;

  /* If we need a sign postion then reserve one */
  qsigns = 0;
  if (f->flags & FMT_FLAGS_SPACE) {
    qsigns = 1;
    sign = ' ';
  }
  if (f->flags & FMT_FLAGS_PLUS) { /* Plus takes precedence over space */
    qsigns = 1;
    sign = '+';
  }
  if (value < 0) {
    qsigns = 1;
    sign = '-';
  }

  /* Reserve space for leading zeroes */
  qzeroes = 0;
  if (f->flags & FMT_FLAGS_ZERO) {
    qzeroes = f->width - qdigits - qsigns;
    if (qzeroes < 0) {
      qzeroes = 0;
    }
  }

  /* Reserve space for leading spaces */
  qspaces = 0;
  qspaces = f->width - qdigits - qsigns - qzeroes;
  if (qspaces < 0) {
    qspaces = 0;
  }

  /* Write the fully formatted number */
  for (i = 0; i < qspaces; i++) *dest++ = ' ';
  if (qsigns != 0) *dest++ = sign;
  for (i = 0; i < qzeroes; i++) *dest++ = '0';
  for (i = 0; i < qdigits; i++) *dest++ = *p++;

  /* Return the number of characters written */
  return dest - destination;

}



/*
 *
 * Placeholders
 *
 */

// int float_format() { return -1; }
// int exponential_format() { return -1; }
// int double_format() { return -1; }
// int octal_format(char *d, struct fmt_t *f, unsigned int val) { return -1; }
// int pointer_format(char *d, struct fmt_t *f, void *ptr) { return -1; }

/*****************************************************************************
 *
 * Format a power of two integer
 *
 ****************************************************************************/

int power_of_2_format(char *destination, struct fmt_t *f,
   unsigned long value, char format_char)
{
  int i;
  int qdigits;
  int qzeroes;
  int qspaces;
  int qsigns;
  unsigned int shift;
  char char_ten; /* The character to represent the number ten */
  char sign;
  char *dest = destination;
  char *p; /* Pointer into buffer */
  char buffer[digits_max];


  switch (format_char)
  {
    case 'x': shift = 4; char_ten = 'a'; break;
    case 'X': shift = 4; char_ten = 'A'; break;
    case 'p': shift = 4; char_ten = 'a'; break;
    case 'o': shift = 3; char_ten = ' '; break;
    case 'b': shift = 1; char_ten = ' '; break; /* Non-standard code */
    default: return -1;
  }

  /* Generate the digits into the temporary buffer */       
  qdigits = digits_shift_gen(buffer + digits_max, value, shift, char_ten);
  if (qdigits == -1) return -1;
  p = buffer + digits_max - qdigits;

  /* If flags includes a space or a plus then reserve a sign position */
  qsigns = 0;
  if (f->flags & FMT_FLAGS_SPACE) {
    qsigns = 1;
    sign = ' ';
  }
  if (f->flags & FMT_FLAGS_PLUS) { /* Plus takes precedence over space */
    qsigns = 1;
    sign = '+';
  }

  /* Reserve space for leading zeroes */
  qzeroes = 0;
  if (f->flags & FMT_FLAGS_ZERO) {
    qzeroes = f->width - qdigits - qsigns;
    if (qzeroes < 0) {
      qzeroes = 0;
    }
  }

  /* Reserve space for leading spaces */
  qspaces = 0;
  qspaces = f->width - qdigits - qsigns - qzeroes;
  if (qspaces < 0) {
    qspaces = 0;
  }

  /* Write the fully formatted number */
  for (i = 0; i < qspaces; i++) *dest++ = ' ';
  if (qsigns != 0) *dest++ = sign;
  for (i = 0; i < qzeroes; i++) *dest++ = '0';
  for (i = 0; i < qdigits; i++) *dest++ = *p++;

  /* Return the number of characters written */
  return dest - destination;
}


/*****************************************************************************
 *
 * Format text
 *
 ****************************************************************************/

int text_format(char *dest, struct fmt_t *f, char *s)
{
  int i;
  char *d = dest;
  int slen = stringlen(s);

//#include <stdio.h>
//printf("slen=%i ", slen);
//printf("width=%i ", f->width);
//printf("flags=%x ", f->flags);
//printf("FMT_FLAGS_MINUS=%x ", FMT_FLAGS_MINUS);
//printf("\n");

  /* Prepend spaces if needed */
  if ((f->flags & FMT_FLAGS_MINUS) == 0) { /* If to right justify */
//printf("(a)\n");
    for (i = 0; i < f->width - slen; i++) {
      *d++ = ' ';
    }
  }

  while (*s != 0) {
    *d++ = *s++;
  }

  /* Postfix spaces if needed */
  if ((f->flags & FMT_FLAGS_MINUS) != 0) { /* If to left justify */
//printf("(z)\n");
    for (i = 0; i < f->width - slen; i++) {
      *d++ = ' ';
    }
  }

  return d - dest;
}


/*****************************************************************************
 *
 * The overall formatter
 *
 ****************************************************************************/

int sformat(char *dest, char *fstring, ...)
{
//  double dval;
  unsigned long uval;
  unsigned long ival;
  int len;
  int more;
  void *pval;
  unsigned int ch;
  char *d = dest;
  char *f = fstring;
  va_list argp;

  *dest = '\0';
  va_start(argp, fstring);
  for (; (ch = *f++);)
  {
    if (ch != '%') /* Not a percent */
    {
      *d++ = ch; /* Copy the non-percent character */
    }
    else if (*f == '\0') /* Percent at end of string */
    {
      *d++ = ch; /* Copy the lone percent character */
    }
    else if (*f == '%') /* Double percent */
    {
      *d++ = '%'; /* Write a percent character */
      ++f; /* Skip the second input percent character */
    }
    else /* A percent sign with a non-percent non-null char after it */
    {
      fmt.flags = 0;
      fmt.width = 0;
      fmt.precision = 0;
      fmt.lqual = 0;
      ch = *f++;

      /* Deal with any flags characters */
      for (more = 1; more;)
      {
        switch (ch)
        {
          case '0': fmt.flags |= FMT_FLAGS_ZERO;  ch = *f++; break;
          case ' ': fmt.flags |= FMT_FLAGS_SPACE; ch = *f++; break;
          case '-': fmt.flags |= FMT_FLAGS_MINUS; ch = *f++; break;
          case '+': fmt.flags |= FMT_FLAGS_PLUS;  ch = *f++; break;
          case '#': fmt.flags |= FMT_FLAGS_HASH;  ch = *f++; break;
          default: more = 0;
        }
      }

      /* Deal with any width specification */
      for (;;)
      {
        if (ch < '0' || ch > '9') break;
        fmt.width *= 10;
        fmt.width += (ch - '0');
        ch = *f++;
      }

      /* Deal with any precision specification */
      if (ch == '.')
      {
        for (;;)
        {
          if (ch < '0' || ch > '9') break;
          fmt.precision *= 10;
          fmt.precision += (ch - '0');
          ch = *f++;
        }
      }

      /* Deal with any length qualifier */
      for (more = 1; more;)
      {
        switch (ch)
        {
          case 'h':
            ch = *f++;
            if (ch == 'h') /* Double h */
            {
              fmt.lqual |= FMT_LQUAL_hh;
              ch = *f++;
            }
            else
            {
              fmt.lqual |= FMT_LQUAL_h;
            }
            break;
          case 'l':
            ch = *f++;
            if (ch == 'l') /* Double l */
            {
              fmt.lqual |= FMT_LQUAL_ll;
              ch = *f++;
            }
            else
            {
              fmt.lqual |= FMT_LQUAL_l;
            }
            break;
          case 'L':
            ch = *f++;
            fmt.lqual |= FMT_LQUAL_L;
            break;
          case 'z':
            ch = *f++;
            fmt.lqual |= FMT_LQUAL_z;
            break;
          case 'j':
            ch = *f++;
            fmt.lqual |= FMT_LQUAL_j;
            break;
          case 't':
            ch = *f++;
            fmt.lqual |= FMT_LQUAL_t;
            break;
          default:
            more = 0;
        }
      }

      /* Dispatch based on the type of the field */
      switch (ch)
      {
        case 'u':
          if (fmt.lqual & FMT_LQUAL_l) {
            uval = va_arg(argp, unsigned long);
          }
          else {
            uval = va_arg(argp, unsigned int);
          }
          len = uint_format(d, &fmt, uval, 10);
          break;
        case 'd':
        case 'i':
          if (fmt.lqual & FMT_LQUAL_l) {
            ival = va_arg(argp, long);
          }
          else {
            ival = va_arg(argp, int);
          }
          len = sint_format(d, &fmt, ival, 10);
          break;
//        case 'f':
//        case 'F':
//          dval = va_arg(argp, double);
//          len = float_format(d, &fmt, dval);
//          break;
//        case 'e':
//        case 'E':
//          dval = va_arg(argp, double);
//          len = exponential_format(d, &fmt, dval);
//          break;
//        case 'g':
//        case 'G':
//          dval = va_arg(argp, double);
//          len = double_format(d, &fmt, dval);
//          break;
        case 'x':
        case 'X':
        case 'o':
        case 'p':
        case 'b': /* Print binary. Non-standard extension */
          if (0) {}
          else if (fmt.lqual & FMT_LQUAL_l)
          {
            uval = va_arg(argp, unsigned long);
            len = power_of_2_format(d, &fmt, uval, ch);
          }
          else
          {
            uval = va_arg(argp, unsigned);
            len = power_of_2_format(d, &fmt, uval, ch);
          }
          break;
        case 's':
          pval = va_arg(argp, char *);
          len = text_format(d, &fmt, pval);
          break;
        case 'c':
          ch = va_arg(argp, unsigned int);
          d[0] = ch;
          len = 1;
          break;
//        case 'n':
//          va_arg(argp, int) = d - dest;
//          len = 0;
//          break;
        default:
          return -1;
      }
    d = d + len;
    }
  }
  va_end(argp);
  *d++ = 0;
  return d - dest;
}


