/*
 *	cvtflasc - Convert floating-point to ASCII
 *
 *	arg		number to convert
 *	ndigits		number of digits to place in buffer
 *	decpt		place to return position of decimal point
 *	sign		place to return sign of number
 *	eflag		flag -- non-zero if `e' format should be used
 *	bufsize		size of `buf'
 *	buf		array in which to place result
 */

void __cvtflasc(double arg, int ndigits, int *decpt, int *sign, char eflag, int bufsize, char *buf)
	{
	int	exp10;		/* base 10 exponent of number		*/

	static const double roundvals[] =	/* values to add for rounding	*/
		{ 5.0e0, 5.0e-1, 5.0e-2, 5.0e-3, 5.0e-4, 5.0e-5,
		  5.0e-6, 5.0e-7, 5.0e-8, 5.0e-9, 5.0e-10, 5.0e-11,
		  5.0e-12, 5.0e-13, 5.0e-14, 5.0e-15, 5.0e-16, 5.0e-17,
		};
#define MAX (sizeof roundvals / sizeof *roundvals - 1)

	/* set sign
	 */
	if (arg < 0)
		{
		arg = -arg;
		*sign = 1;
		}
	else
		*sign = 0;

	/* Scale the number so it is in the range 1 to 10
	 */
	exp10 = 0;
	if (arg > 0)
		{
		while (arg >= 10)
			arg /= 10, ++exp10;
		while (arg < 1)
			arg *= 10, --exp10;
		}

	/* If `f' format, we want `ndigits' past the decimal point.
	 * If 'e' format we want ndigits + 1 digit before the decimal place.
	 */
	if (!eflag)
		ndigits += exp10 + 1;
	else
		++ndigits;

	/* Round the number to the specified number of digits
	 */
	arg += roundvals[ndigits > MAX ? MAX : ndigits];

	/* It might need more scaling after rounding
	 */
	if (arg >= 10)
		{
		arg = 1;
		++exp10;
		if (!eflag)
			++ndigits;
		}

	/* Set `decpt'
	 */
	*decpt = exp10 + 1;

	{
	register char *bufend = &buf[bufsize] - 1;
	register int c;
	register int i;

	for (i = 0 ; i < ndigits ; ++i)
		{
		if (buf >= bufend)
			break;
		if (i > 17)
			*buf++ = '0';
		else
			{
			c = (int)arg;
			*buf++ = '0' + c;
			arg = (arg - c) * 10;
			}
		}

	*buf = '\0';
	}

	}
