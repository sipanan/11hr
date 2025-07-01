#ifndef _STDLIB_H_
#define _STDLIB_H_

/* Standard utility functions */

#ifndef _STDDEF_H_
#include <stddef.h>
#endif /* _STDDEF_H_ */

#define	RAND_MAX	32767		/* max value returned by rand() */
#define	EXIT_SUCCESS	0
#define	EXIT_FAILURE	1

#define	MB_CUR_MAX	1		/* locale currently not supported */

typedef struct {
	int	quot;	/* quotient */
	int	rem;	/* remainder */
} div_t;				/* div() return type */
typedef struct {
	unsigned	quot;	/* quotient */
	unsigned	rem;	/* remainder */
} udiv_t;				/* div() return type */
typedef struct {
	long 	quot;	/* quotient */
	long 	rem;	/* remainder */
} ldiv_t;				/* ldiv() return type */
typedef struct {
	unsigned long 	quot;	/* quotient */
	unsigned long 	rem;	/* remainder */
} uldiv_t;				/* ldiv() return type */


#if __DBL_DIG__ == __LDBL_DIG__
#define atof _datof
#define strtod _dstrtod
#endif
extern double		atof(const char *);
extern double		strtod(const char *, const char **);
extern int			atoi(const char *);
extern unsigned		xtoi(const char *);
extern long			atol(const char *);
extern long			strtol(const char *, char **, int);
#define	strtoul(a,b,c)	((unsigned long)strtol((a),(b),(c)))
extern int			rand(void);
extern void			srand(unsigned int);
extern void *		calloc(size_t, size_t);
extern div_t		div(int numer, int denom);
extern udiv_t		udiv(unsigned numer, unsigned denom);
extern ldiv_t		ldiv(long numer, long denom);
extern uldiv_t		uldiv(unsigned long numer,unsigned  long denom);

#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#define	min(a,b)	(((a) < (b)) ? (a) : (b))

extern void *		malloc(size_t);
extern void		free(void *);
extern void *		realloc(void *, size_t);

extern void	abort(void);
extern void	exit(int);
extern int	atexit(void (*)(void));
extern char *	getenv(const char *);
extern char **	environ;
extern int	system(char *);
extern void	qsort(void *, size_t, size_t, int (*)(const void *, const void *));
extern void *	bsearch(const void *, void *, size_t, size_t, int(*)(const void *, const void *));
extern int	abs(int);
extern long	labs(long);

//extern char *itoa(int val, char *buf, int base);
extern char *	utoa(char * buf, unsigned val, int base);
extern char *	ltoa(char * buf, long val, int base);
extern char *	ultoa(char * buf, unsigned long val, int base);

#endif /* _STDLIB_H_ */
