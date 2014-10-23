#ifndef _BG_SSCANF_C
#define _BG_SSCANF_C
/*
Source:
http://www.koders.com/c/fid1D51E1E6F4F922088DF508FFEEEEC702AFED6333.aspx?s=__v_scanf
*/

//#include <stdarg.h>
//#include <limits.h>
//#include <string.h>

#define GBLONG_MAX 2147483647L
#define GBLONG_MIN (-GBLONG_MAX-1)
#define GBULONG_MAX (GBLONG_MAX * 2UL + 1)


extern long int strtol ( const char * str, char ** endptr, int base );

#define WANT_FLOATING_POINT_IN_SCANF 1
#define WANT_CHARACTER_CLASSES_IN_SCANF 1

#define	EOF	(-1)

struct arg_scanf {
  void *data;
  int (*getch)(void*);
  int (*putch)(int,void*);
};

struct str_data {
  unsigned char* str;
};

static int sgetc(struct str_data* sd) {
  register unsigned int ret = *(sd->str++);
  return (ret)?(int)ret:-1;
}

static int sputc(int c, struct str_data* sd) {
  return (*(--sd->str)==c)?c:-1;
}


#define A_GETC(fn)	(++consumed,(fn)->getch((fn)->data))
#define A_PUTC(c,fn)	(--consumed,(fn)->putch((c),(fn)->data))


static int isspace(int c)
{
	return ((c==(int)' ')) ? 1 : 0;
}

static int isdigit(int c)
{
	return ((c>=(int)'0') && (c<=(int)'9')) ? 1 : 0;
}

int __v_scanf(struct arg_scanf* fn, const unsigned char *format, va_list arg_ptr)
{
  unsigned int ch;	/* format act. char */
  int n=0;

  /* arg_ptr tmps */
#ifdef WANT_FLOATING_POINT_IN_SCANF
  double *pd;
  float  *pf;
#endif
  long   *pl;
  short  *ph;
  int    *pi;
  char    *s;

  unsigned int consumed=0;

  /* get one char */
  int tpch= A_GETC(fn);

  while (*format)
  {
    ch=*format++;
    switch (ch) {
    /* end of format string ?!? */
    case 0: return 0;

    /* skip spaces ... */
    case ' ':
    case '\f':
    case '\t':
    case '\v':
    case '\n':
    case '\r':
      while((*format)&&(isspace(*format))) ++format;
      while(isspace(tpch)) tpch=A_GETC(fn);
      break;

    /* format string ... */
    case '%':
      {
	unsigned int _div=0;
	int width=-1;
	char flag_width=0;
	char flag_discard=0;
	char flag_half=0;
	char flag_long=0;
	char flag_longlong=0;

in_scan:
	ch=*format++;
	if(ch!='n' && tpch==-1) goto err_out;
	switch (ch) {
	/* end of format string ?!? */
	case 0: return 0;

	/* check for % */
	case '%':
	  if ((unsigned char)tpch != ch) goto err_out;
	  tpch=A_GETC(fn);
	  break;

	/* FLAGS */
	case '*':
	  flag_discard=1;
	  goto in_scan;
	case 'h':
	  flag_half=1;
	  goto in_scan;
	case 'l':
	  if (flag_long) flag_longlong=1;
	  flag_long=1;
	  goto in_scan;
	case 'q':
	case 'L':
	  flag_longlong=1;
	  goto in_scan;

	/* WIDTH */
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  width=strtol((const char*)(format-1),&s,10);
	  format=(unsigned char*)s;
	  flag_width=1;
	  goto in_scan;

	/* scan for integer / strtol reimplementation ... */
	case 'p':
	case 'X':
	case 'x':
	  _div+=6;
	case 'd':
	  _div+=2;
	case 'o':
	  _div+=8;
	case 'u':
	case 'i':
	  {
	    unsigned long v=0;
	    unsigned int consumedsofar;
	    int neg=0;
	    while(isspace(tpch)) tpch=A_GETC(fn);
	    if (tpch=='-') {
	      tpch=A_GETC(fn);
	      neg=1;
	    }

	    if (tpch=='+') tpch=A_GETC(fn);

	    if (tpch==-1) return n;
	    consumedsofar=consumed;

	    if (!flag_width) {
	      if ((_div==16) && (tpch=='0')) goto scan_hex;
	      if (!_div) {
		_div=10;
		if (tpch=='0') {
		  _div=8;
scan_hex:
		  tpch=A_GETC(fn);
		  if ((tpch|32)=='x') {
		    tpch=A_GETC(fn);
		    _div=16;
		  }
		}
	      }
	    }
	    while ((width)&&(tpch!=-1)) {
	      register unsigned long c=tpch&0xff;
	      register unsigned long d=c|0x20;
	      c=(d>='a'?d-'a'+10:c<='9'?c-'0':0xff);
	      if (c>=_div) break;
	      d=v*_div;
	      v=(d<v)?GBULONG_MAX:d+c;
	      --width;
	      tpch=A_GETC(fn);
	    }

	    if (consumedsofar==consumed) return n;

	    if ((ch|0x20)<'p') {
	      register long l=v;
	      if (v>=-((unsigned long)GBLONG_MIN)) {
		l=(neg)?GBLONG_MIN:GBLONG_MAX;
	      }
	      else {
		if (neg) v*=-1;
	      }
	    }
	    if (!flag_discard) {
	      if (flag_long) {
		pl=(long *)va_arg(arg_ptr,long*);
		*pl=v;
	      } else if (flag_half) {
		ph=(short*)va_arg(arg_ptr,short*);
		*ph=v;
	      } else {
		pi=(int *)va_arg(arg_ptr,int*);
		*pi=v;
	      }
	      if(consumedsofar<consumed) ++n;
	    }
	  }
	  break;

	/* FIXME: return value of *scanf with ONE float maybe -1 instead of 0 */
#ifdef WANT_FLOATING_POINT_IN_SCANF
	/* floating point numbers */
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	  {
	    double d=0.0;
	    int neg=0;
	    unsigned int consumedsofar;

	    while(isspace(tpch)) tpch=A_GETC(fn);

	    if (tpch=='-') {
	      tpch=A_GETC(fn);
	      neg=1;
	    }
	    if (tpch=='+') tpch=A_GETC(fn);

	    consumedsofar=consumed;

	    while (isdigit(tpch)) {
	      d=d*10+(tpch-'0');
	      tpch=A_GETC(fn);
	    }
	    if (tpch=='.') {
	      double factor=.1;
	      consumedsofar++;
	      tpch=A_GETC(fn);
	      while (isdigit(tpch)) {
		d=d+(factor*(tpch-'0'));
		factor/=10;
		tpch=A_GETC(fn);
	      }
	    }
	    if (consumedsofar==consumed) return n;	/* error */
	    if ((tpch|0x20)=='e') {
	      int exp=0, prec=tpch;
	      double factor=10;
	      tpch=A_GETC(fn);
	      if (tpch=='-') {
		factor=0.1;
		tpch=A_GETC(fn);
	      } else if (tpch=='+') {
		tpch=A_GETC(fn);
	      } else {
		d=0;
		if (tpch!=-1) A_PUTC(tpch,fn);
		tpch=prec;
		goto exp_out;
	      }
	      consumedsofar=consumed;
	      while (isdigit(tpch)) {
		exp=exp*10+(tpch-'0');
		tpch=A_GETC(fn);
	      }
	      if (consumedsofar==consumed) return n;	/* error */
	      while (exp) {	/* as in strtod: XXX: this introduces rounding errors */
		d*=factor; --exp;
	      }
	    }
exp_out:
	    if (!flag_discard) {
	      if (flag_long) {
		pd=(double *)va_arg(arg_ptr,double*);
		*pd=d;
	      } else {
		pf=(float *)va_arg(arg_ptr,float*);
		*pf=d;
	      }
	      ++n;
	    }
	  }
	  break;
#endif

	/* char-sequences */
	case 'c':
	  if (!flag_discard) {
	    s=(char *)va_arg(arg_ptr,char*);
	    ++n;
	  }
	  if (!flag_width) width=1;
	  while (width && (tpch!=-1)) {
	    if (!flag_discard) *(s++)=tpch;
	    --width;
	    tpch=A_GETC(fn);
	  }
	  break;

	/* string */
	case 's':
	  if (!flag_discard) s=(char *)va_arg(arg_ptr,char*);
	  while(isspace(tpch)) tpch=A_GETC(fn);
	  if (tpch==-1) break;		/* end of scan -> error */
	  while (width && (tpch!=-1) && (!isspace(tpch))) {
	    if (!flag_discard) *s=tpch;
	    if (tpch) ++s; else break;
	    --width;
	    tpch=A_GETC(fn);
	  }
	  if (!flag_discard) { *s=0; ++n; }
	  break;

	/* consumed-count */
	case 'n':
	  if (!flag_discard) {
	    pi=(int *)va_arg(arg_ptr,int *);
            *pi=consumed-1;
	  }
	  break;

#ifdef WANT_CHARACTER_CLASSES_IN_SCANF
	case '[':
	  {
	    char cset[256];
	    int flag_not=0;
	    int flag_dash=0;
	    memset(cset,0,sizeof(cset));
	    ch=*format++;
	    /* first char specials */
	    if (ch=='^') {
	      flag_not=1;
	      ch=*format++;
	    }
	    if ((ch=='-')||(ch==']')) {
	      cset[ch]=1;
	      ch=*format++;
	    }
	    /* almost all non special chars */
	    for (;(*format) && (*format!=']');++format) {
	      if (flag_dash) {
		register unsigned char tmp=*format;
		for (;ch<=tmp;++ch) cset[ch]=1;
		flag_dash=0;
		ch=*format;
	      }
	      else if (*format=='-') flag_dash=1;
	      else {
		cset[ch]=1;
		ch=*format;
	      }
	    }
	    /* last char specials */
	    if (flag_dash) cset['-']=1;
	    else cset[ch]=1;

	    /* like %c or %s */
	    if (!flag_discard) {
	      s=(char *)va_arg(arg_ptr,char*);
	      ++n;
	    }
	    while (width && (tpch>=0) && (cset[tpch]^flag_not)) {
	      if (!flag_discard) *s=tpch;
	      if (tpch) ++s; else break;
	      --width;
	      tpch=A_GETC(fn);
	    }
	    if (!flag_discard) *s=0;
	    ++format;
	  }
	  break;
#endif
	default:
	  goto err_out;
	}
      }
      break;

    /* check if equal format string... */
    default:
      if ((unsigned char)tpch != ch) goto err_out;
      tpch=A_GETC(fn);
      break;
    }
  }

  /* maybe a "%n" follows */
  if(*format) {
    while(isspace(*format)) format++;
    if(format[0] == '%' && format[1] == 'n') {
      pi = (int *) va_arg(arg_ptr, int *);
      *pi = consumed - 1;
    }
  }

err_out:
  if (tpch<0 && n==0) return EOF;
  A_PUTC(tpch,fn);
  return n;
}


int bgvsscanf(const char* str, const char* format, va_list arg_ptr)
{
  struct str_data  fdat = { (unsigned char*)str };
  struct arg_scanf farg = { (void*)&fdat, (int(*)(void*))sgetc, (int(*)(int,void*))sputc };
  return __v_scanf(&farg,format,arg_ptr);
}

int bgsscanf(const char *str, const char *format, ...)
{
  int n;
  va_list arg_ptr;
  va_start(arg_ptr, format);
  n=bgvsscanf(str,format,arg_ptr);
  va_end (arg_ptr);
  return n;
}

#endif
