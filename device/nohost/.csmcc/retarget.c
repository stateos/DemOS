#include <stdio.h>

#if !defined(USE_SEMIHOST) && !defined(USE_NOHOST)

/* -------------------------------------------------------------------------- */

__weak
int getchar( void )
{
	return (int) 0;
}

/* -------------------------------------------------------------------------- */

__weak
int putchar( char c )
{
	return (int) c;
}

/* -------------------------------------------------------------------------- */

#endif // !USE_SEMIHOST && !USE_NOHOST
