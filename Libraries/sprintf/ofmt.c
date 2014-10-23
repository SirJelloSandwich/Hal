/*
 *	__ofmt - in-memory text conversion and formatting
 *
 *	__ofmt is used by printf(), fprintf() and sprintf() to format
 *	a string.
 *
 *	A format argument is a string to be output with optional imbedded
 *	conversion specifications.  The conversion specifications start with
 *	a `%' and wnd with a conversion character
 */

#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
/*#include <introl.h>*/
#define FLOATS	/*!!!*/
#define LONGS	/*!!!*/

extern void __cvtflasc(double arg, int ndigits, int *decpt, int *sign, char eflag, int bufsize, char *buf);

/* to save space on smaller processors, we will undef
 * all ctype macros, this will cause their functions to be used.
 */
#if defined __CC01__ || defined __CC03__ || defined __CC11__ || defined __CC09__
#undef isalnum
#undef isalpha
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef __tolower
#undef __toupper
#endif /* use less memory */

/*	LONGINTS will be defined if longs and ints are the same size
 *	on the target.
 */
 
#if INT_MAX == LONG_MAX
#define LONGINTS 1
#undef LONGS
#else
#define LONGINTS 0
#endif

/*
 * a few typedefs and defines
 */
#ifdef UCHAR
#undef UCHAR
#endif
typedef unsigned char UCHAR;
#ifdef UINT
#undef UINT
#endif
typedef unsigned int UINT;
#ifdef ULONG
#undef ULONG
#endif
typedef unsigned long ULONG;

#define	INITPREC	6	/* number + (opt sign) */
#define MAXLEN 21

static void putcurrent(FILE *stream, char **line, /*int buf_size,*/ int putvalue, int *numberput);

/*
 * definitions for __ofmt flags.
 */
#define	INTPAD		0x01
#define	LEFTJUST	0x02	/* left justify output */
#define	ALTERNATE	0x04	/* use "#" alternate form */
#define	LONGFLAG	0x08	/* is this a long argument */
#define	SIGNFLAG	0x10	/* must we output a sign */
#define	LEADINGZERO	0x20	/* output a leasing zero */
#define	PRECISIONFLAG	0x40	/* user specified precision */

#define	FLOATFLAG	0x01	/* are we in a float format */
#define	DECIMALPOINT	0x02	/* should we output a decimalpoint */
#define	EXPSIGN		0x04	/* sign of exponent 0:+, 1:- */
#define	EXPONENT	0x08	/* exponent: yes or no */
#define	ETYPE		0x10	/* "e" format conversion in __cvtflasc */

int bf_sz;

int __ofmt(FILE *stream, char *line, int buf_size, const char *format, va_list ap)      // WBR: added buf_size
	{
	UCHAR	flags;		/* flag holder for output formatting */
	UCHAR	fflags;		/* flags for float formatting */
	char	work[MAXLEN];	/* Maximum character length or any float, int, etc. */
	char	*workptr;	/* temp pointer used to move through work[] */
	char	c;		/* current character in format string */
	char	base;		/* base for numerical conversions */
	char	sign;		/* (+,-, ,\0) used when SIGNFLAG is set */
	int	width;		/* %3: format width: default 0 */
	int	precision;	/* %.3: format precision: default -1 */
	int	numberput = 0;	/* number of characters output */
	char	pad;		/* character to pad fields with */
	int	totallength;	/* Number of spaces to pad determined in each case */
	int	numchars;	/* number of characters in 'number' */
#ifdef  LONGS
	long	ltemp;		/* long temporary */
	ULONG	conv_val;	/* variable used by 'convert:' which contains the number */
#else
	int	ltemp;		/* int temporary */
	UINT	conv_val;	/* variable used by 'convert:' which contains the number */
#endif
	int	decpt = 0;	/* double decimal point	(needed anyway) */
#ifdef  FLOATS
	int	exponent;	/* represents the exponent value */
	double	ftemp;		/* double temporary */
	int	exponent_begin;	/* beginning index into "work[]" for the exponent */
#endif

bf_sz=  buf_size;     // WBR: set buffer size for snprintf2

	while ((c = *format))	/* get a format character */
		{
		if (c != '%')	/* if no conversion specifier ... */
			{
			/* just send character */
			putcurrent(stream, &line, c, &numberput);
			++format;
			continue;	/* and try again */
			}

		workptr = work;		/* set up work buffer ptr */
		fflags = flags = 0;	/* initial all flag bits to 0 */
		precision = -1;
		width = 0;
		pad = ' ';		/* default padding caracter */

incloop:	++format;
loop:		switch (c = *format)
			{
			case ' ':
				flags |= SIGNFLAG;
				sign = ' ';
				goto incloop;
			case '#':
				flags |= ALTERNATE;
				goto incloop;
			case '-':
				flags |= LEFTJUST;
				goto incloop;
			case '+':
				flags |= SIGNFLAG;
				sign = '+';
				goto incloop;
			case '%':
				putcurrent(stream, &line, '%', &numberput);
				++format;
				break;
			case '0':
				/* set pad character to zero */
				pad = '0';
				goto incloop;
#if defined(LONGS) || defined(LONGINTS)
			case 'l':
				flags |= LONGFLAG;
				goto incloop;
#endif /* LONGFLAGS */
			case '*':
				/* get width */
				width = (va_arg(ap, int));
				if (*(++format) != '.')
					goto loop;
				++format;
				if (isdigit(*format))
					goto numprecision;
				if (*format != '*')
					goto loop;
argprecision:
				flags |= PRECISIONFLAG;
				precision = va_arg(ap, int);
				goto incloop;
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				while(isdigit(*format))
					width = 10 * width + (*(format)++ - '0');
				if (*format == '.')
					{
			case '.':
					/* get the maximum width allowed
					 */
					flags |= PRECISIONFLAG;
					precision = 0;
					++format;	/* skip the '.'	*/
		
					if (*format == '*')
						goto argprecision;
					else
numprecision:
						while (isdigit(*format))
							precision = 10 * precision +
							(*(format)++ - '0');
					}
				goto loop;
#ifdef  FLOATS
			{
			int	negative;

			case 'G':
			case 'g':
				ftemp = (va_arg(ap,double));
				__cvtflasc(ftemp, precision, &decpt, &negative,
					0, sizeof(work), workptr);
				if (decpt < -4 && decpt < precision)
					goto float_common;
				goto set_e_flags;
			case 'E':
			case 'e':
				/* set the E format flag and other float flags */
				ftemp = (va_arg(ap,double));
			set_e_flags:
				fflags |= EXPONENT | ETYPE;
				goto float_common;
			case 'F':
			case 'f':
				ftemp = (va_arg(ap,double));
			float_common:
				fflags |= FLOATFLAG;
				exponent = 0;
				workptr = work;

				if (precision == -1)
					precision = INITPREC;

				__cvtflasc(ftemp, precision, &decpt, &negative,
					fflags & ETYPE, sizeof(work), workptr);

				if (precision != 0)
					fflags |= DECIMALPOINT;

				if (negative)
					{
					flags |= SIGNFLAG;
					sign = '-';
					}

				if (decpt < 0)
					flags |= LEADINGZERO;
				else
					flags &= ~LEADINGZERO;

				if (fflags & EXPONENT)
					{
					if (decpt < 0)
						{
						fflags |= EXPSIGN;
						decpt = -decpt;
						exponent = decpt + 1;
						}
					else
						{
						exponent = decpt - 1;
						fflags &= ~EXPSIGN;
						}
					decpt = 1;
					workptr[MAXLEN-1] = '\0';
					if (exponent > -10 && exponent < 10)
						{
						workptr[MAXLEN-2] = exponent + '0';
						workptr[MAXLEN-3] = '0';
						exponent = 4;
						exponent_begin = MAXLEN - 3;
						}
					else if (exponent > -100 && exponent < 100)
						{
						workptr[MAXLEN-2] = exponent % 10 + '0';
						workptr[MAXLEN-3] = (exponent / 10) % 10 + '0';
						exponent = 4;
						exponent_begin = MAXLEN - 3;
						}
					else
						{
						char *s = workptr + MAXLEN - 1;
						int tmp;
						for (tmp = 0 ; tmp < 3 ; ++tmp)
							{
							*--s = exponent % 10 + '0';
							exponent /= 10;
							}
						exponent = 5;
						exponent_begin = MAXLEN - 4;
						}
					}

				if (decpt > 0)
					totallength = decpt;
				else
					totallength = 1;

				totallength += precision + totallength +
					((fflags & DECIMALPOINT) != 0) + ((flags & SIGNFLAG) != 0);

				goto output;	/* output */
			}
#endif
			case 'N':
			case 'n':
				*(va_arg(ap, int *)) = numberput;
				continue;
			case 'I':
			case 'i':
			case 'D':
			case 'd':
				/* convert signed `int' or `long' to decimal
				 */
#ifdef  LONGS
				if (flags & LONGFLAG)
					{
					ltemp = (va_arg(ap, long));
					if (ltemp < 0)
						{
						flags |= SIGNFLAG;
						ltemp = -ltemp;
						sign = '-';
						}
					}
				else
#endif
					{
					ltemp = (va_arg(ap, int));
					if (ltemp < 0)
						{
						flags |= SIGNFLAG;
						sign = '-';
						ltemp = -ltemp;
						}
					}

				conv_val = ltemp;
				base = 10;
				goto convert;

			case 'U':
			case 'u':
				/* convert unsigned instead of signed
				 */
#ifdef  LONGS
				if (flags & LONGFLAG)
					conv_val = (va_arg(ap,unsigned long));
				else
#endif
					conv_val = (va_arg(ap,unsigned));

				/* clear signflag
				 */
				flags &= ~SIGNFLAG;
				base = 10;
				goto convert;

			case 'P':
			case 'p':
			case 'X':
			case 'x':
				/* convert to hexadecimal instead of decimal
				 */
#ifdef  LONGS
				if (flags & LONGFLAG)
					conv_val = (va_arg(ap,unsigned long));
				else
#endif
					conv_val = (va_arg(ap,unsigned));
				base = 16;
				flags &= ~SIGNFLAG;
				goto convert;

			case 'O':
			case 'o':
				/* convert to octal instead of decimal
				 */
#ifdef  LONGS
				if (flags & LONGFLAG)
					conv_val = (va_arg(ap,unsigned long));
				else
#endif
					conv_val = (va_arg(ap,unsigned));

				base = 8;
				flags &= ~SIGNFLAG;
				goto convert;

				/* convert - convert the value to the given
				 * base and advance all the pointers
				 */
			convert:
				{
				int val;
				char *v, *x, y, letter;

				numchars = 1;
				letter = (c == 'X' ? 'A' : 'a');
				val = conv_val % base;
				*workptr++ = val < 10 ? val + '0' : val + letter - 10;

				/* convert the string backwards and then reverse it
				 */
				while ( ( conv_val = (conv_val / base) ) )/* changed parentheses order - 20100517 DFB */
														/* originally  'while ((conv_val = conv_val)  / (base))' */
					{
					val = conv_val % base;                 /* get lowest digit */
					*workptr++ = (val < 10 ? val + '0' : val + letter - 10);
					++numchars;
					}

				/* if alternate is set, use 'c' to determine which prefix
				 * to output
				 */
				if (flags & ALTERNATE)
					switch (c)
						{
						case 'x':
							*workptr++ = 'x';
							++numchars;
							goto addthe0;
						case 'X':
							*workptr++ = 'X';
							++numchars;
							goto addthe0;
						case 'o':
						case 'O':
addthe0:
							*workptr++ = '0';
							++numchars;
							break;
						}
				*workptr = '\0';	/* terminate work buffer */

				/* reverse the string
				 */
				for (v = work ,  x = v + numchars ; v < x ; )
					{
					y = *v;
					*v++ = *--x;
					*x = y;
					}

				flags &= ~LEADINGZERO;
				fflags &= ~(DECIMALPOINT | EXPONENT);

				if (precision && !width)
					width = precision;
				if (!(flags & PRECISIONFLAG))
					precision = numchars;
				else if (numchars <= precision && width <= precision)
					pad = '0';
				totallength = numchars + ((flags & SIGNFLAG) != 0);
				}
				goto output;
			case 'C':
			case 'c':
				/* send a character
				 */
				*workptr++ = (va_arg(ap, int));
				*workptr = '\0';	/* terminate work buffer */
				numchars = totallength = precision = 1;
				goto output;

			case 'S':
			case 's':
				/* send a string
				 */
				workptr = (va_arg(ap, char *));

				numchars=totallength=strlen(workptr);

				if ((flags & PRECISIONFLAG) && precision < totallength)
					totallength = precision;
				else
					precision = totallength;
					
				if (totallength > width)
					{
					numchars = totallength;
					width =  totallength;
					}
				
				/* fall into output, but don't overwrite "workptr" */
				goto output1;
			output:
				workptr = work;	/* point to work buffer	*/
			output1:

				if (!(flags & LEFTJUST))
					{
					int i;

					for (i = width - (totallength < precision ? precision : totallength) ; i > 0 ; --i)
						putcurrent(stream, &line, pad ,&numberput);
					for (i = precision - totallength; i > 0 ; i--)
						putcurrent(stream, &line, '0' ,&numberput);
					}

				/* send out leading padding character
				 */
				if (flags & SIGNFLAG)
					putcurrent(stream, &line, sign, &numberput);
		
				/* transfer the work buffer to the line
				 */
#ifdef FLOATS
				if (fflags & FLOATFLAG)
					{
					if (decpt > 0)
						do
							{
							putcurrent(stream, &line,*workptr++,&numberput);
							}
						while (--decpt > 0);
					else
						putcurrent(stream, &line,'0',&numberput);

					if (fflags & DECIMALPOINT)
						putcurrent(stream, &line,'.',&numberput);
					while(decpt < 0 && precision > 0)
						{
						putcurrent(stream, &line,'0',&numberput);
						++decpt;
						--precision;
						}

					while (precision > 0)
						{
						putcurrent(stream, &line,*workptr++,&numberput);
						--precision;
						}

					if (fflags & EXPONENT)
						{
						char *s;
						int i;

						putcurrent(stream, &line,'e',&numberput);
						if (fflags & EXPSIGN)
							putcurrent(stream, &line, '-', &numberput);
						else
							putcurrent(stream, &line, '+', &numberput);
						for ((s = &work[exponent_begin]); (i = *s) ; ++s)
							 putcurrent(stream, &line, i, &numberput);
						}
					}
				else
#endif
					{
					int i;
					if (!precision || numchars < precision)
						i = numchars;
					else
						i = precision;
					for ( ; i ; --i)
						putcurrent(stream, &line,*workptr++,&numberput);
					}
				/* if left justified, pad on the right
				 */
				if (flags & LEFTJUST)
					{
					int i;

					for (i = width - totallength; i > 0 ; i--)
						/* here always pad with spaces */
						putcurrent(stream, &line, ' ', &numberput);
					}
				++format;
				break;

			default:
				/* if the character after the `%' wasn't a
				 * null then add it to the print buffer and
				 * increment `format'
				 */
				if (*format)
					putcurrent(stream, &line, *format++, &numberput);
			}
		}
	/* terminate line */
	putcurrent(stream, &line, '\0', &numberput);
	return numberput - 1;
	}

static void putcurrent(FILE *stream, char **line, int putvalue, int *numberput)
	{
	(*numberput)++;
	if((stream == NULL) && ((*numberput < bf_sz) || (putvalue == '\0')))         // WBR: added size check for snprintf function
		*(*line)++ = putvalue;
	else
		/*putc(putvalue,stream)*/;
	}
