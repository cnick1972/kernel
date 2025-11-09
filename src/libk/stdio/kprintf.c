/**
 * @file libk/stdio/kprintf.c
 * @brief Kernel printf implementation.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

/**
 * @brief Bounded string length helper.
 *
 * @param s     Input string.
 * @param count Maximum characters to inspect.
 * @return Number of characters encountered before null or limit.
 */
size_t strnlen(const char * s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/**
 * @brief Test whether a character is a decimal digit.
 *
 * @param ch Character to test.
 * @return Non-zero if digit, otherwise zero.
 */
static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

/**
 * @brief Parse decimal digits from a string.
 *
 * @param s Pointer to string pointer; advanced past parsed digits.
 * @return Parsed integer value.
 */
static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */

#define __do_div(n, base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

/**
 * @brief Append a character to a string buffer with bounds checking.
 *
 * @param str       Pointer to current write cursor.
 * @param remaining Pointer to remaining capacity counter.
 * @param total     Pointer to total characters that would be written.
 * @param ch        Character to append.
 */
static inline void append_char(char **str, size_t *remaining, int *total, char ch)
{
	if (*remaining > 1) {
		**str = ch;
		(*str)++;
		(*remaining)--;
	}
	(*total)++;
}

/**
 * @brief Format an integer into a string buffer.
 *
 * @param str       Pointer to current write cursor.
 * @param remaining Pointer to remaining capacity counter.
 * @param total     Pointer to total characters that would be written.
 * @param num       Number to format.
 * @param base      Numeric base (8, 10, 16).
 * @param size      Field width.
 * @param precision Precision specifier.
 * @param type      Flag mask controlling formatting behaviour.
 * @return Updated write cursor.
 */
static char *number(char *str, size_t *remaining, int *total, long num, int base,
        int size, int precision, int type)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

	char tmp[66];
	char c, sign, locase;
	int i;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 16)
		return str;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = (digits[__do_div(num, base)] | locase);
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			append_char(&str, remaining, total, ' ');
	if (sign)
		append_char(&str, remaining, total, sign);
	if (type & SPECIAL) {
		if (base == 8)
			append_char(&str, remaining, total, '0');
		else if (base == 16) {
			append_char(&str, remaining, total, '0');
			append_char(&str, remaining, total, ('X' | locase));
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			append_char(&str, remaining, total, c);
	while (i < precision--)
		append_char(&str, remaining, total, '0');
	while (i-- > 0)
		append_char(&str, remaining, total, tmp[i]);
	while (size-- > 0)
		append_char(&str, remaining, total, ' ');
	return str;
}

/**
 * @brief Format a string into a buffer with explicit bounds.
 *
 * @param buf   Destination buffer.
 * @param size  Size of destination buffer in bytes.
 * @param fmt   Format string.
 * @param args  Variadic argument list.
 * @return Number of characters that would have been written (excluding null terminator).
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char *str = buf;
	const char *s;

	int flags;          /* flags to number() */

	int field_width;    /* width of output field */
	int precision;      /* min. # of digits for integers; max
			   number of chars for from string */
	int qualifier;      /* 'h', 'l', or 'L' for integer fields */

	size_t remaining = size;
	int total = 0;

	for (; *fmt; ++fmt) {
		if (*fmt != '%') {
			append_char(&str, &remaining, &total, *fmt);
			continue;
		}

		/* process flags */
		flags = 0;
	      repeat:
		++fmt;          /* this also skips first '%' */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					append_char(&str, &remaining, &total, ' ');
			append_char(&str, &remaining, &total, (unsigned char)va_arg(args, int));
			while (--field_width > 0)
				append_char(&str, &remaining, &total, ' ');
			continue;

		case 's':
			s = va_arg(args, char *);
			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--)
					append_char(&str, &remaining, &total, ' ');
			for (i = 0; i < len; ++i)
				append_char(&str, &remaining, &total, *s++);
			while (len < field_width--)
				append_char(&str, &remaining, &total, ' ');
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, &remaining, &total,
			    (unsigned long)va_arg(args, void *), 16,
			    field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = total;
			} else {
				int *ip = va_arg(args, int *);
				*ip = total;
			}
			continue;

		case '%':
			append_char(&str, &remaining, &total, '%');
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'x':
			flags |= SMALL;
			fallthrough;
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
			break;

		case 'u':
			break;

		default:
			append_char(&str, &remaining, &total, '%');
			if (*fmt)
				append_char(&str, &remaining, &total, *fmt);
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN)
				num = (short)num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
		str = number(str, &remaining, &total, num, base, field_width, precision, flags);
	}

	if (size > 0) {
		*str = '\0';
	}
	return total;
}

/**
 * @brief Format a string into a buffer without bounds checking.
 *
 * @param buf Destination buffer.
 * @param fmt Format string.
 * @param args Variadic argument list.
 * @return Number of characters written (excluding null terminator).
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
	return vsnprintf(buf, SIZE_MAX, fmt, args);
}

/**
 * @brief Format a string into a buffer with explicit bounds (variadic wrapper).
 *
 * @param buf  Destination buffer.
 * @param size Size of destination buffer in bytes.
 * @param fmt  Format string.
 * @param ...  Additional variadic arguments.
 * @return Number of characters that would have been written (excluding null terminator).
 */
int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return i;
}

/**
 * @brief Format a string into a buffer without bounds checking (variadic wrapper).
 *
 * @param buf Destination buffer.
 * @param fmt Format string.
 * @param ... Additional variadic arguments.
 * @return Number of characters written (excluding null terminator).
 */
int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, SIZE_MAX, fmt, args);
	va_end(args);
	return i;
}

/**
 * @brief Printf-style formatted output to the active console.
 *
 * @param fmt Format string.
 * @param ... Additional variadic arguments.
 * @return Number of characters written (excluding null terminator).
 */
int kprintf(const char* fmt, ...)
{
	char printf_buf[1024];
	va_list args;
	int printed;

	va_start(args, fmt);
	printed = vsnprintf(printf_buf, sizeof(printf_buf), fmt, args);
	va_end(args);

	kputs(printf_buf);

	return printed;	
}
