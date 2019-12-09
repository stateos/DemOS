#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

/* -------------------------------------------------------------------------- */

__attribute__((weak))
caddr_t _sbrk_r( struct _reent *reent, size_t size )
{
	extern char __heap_start[];
	extern char __heap_end[];
	static char * heap = __heap_start;
	       char * base;
	      (void)  reent;

	if (heap + size <= __heap_end)
	{
		base  = heap;
		heap += size;
	}
	else
	{
		errno = ENOMEM;
		base  = (caddr_t) -1;
	}

	return base;
}

/* -------------------------------------------------------------------------- */

#if !defined(USE_SEMIHOST) && !defined(USE_NOHOST)

/* -------------------------------------------------------------------------- */

static
int __enosys()
{
	errno = ENOSYS; // procedure not implemented

	return -1;
}

/* -------------------------------------------------------------------------- */

int   _open_r( struct _reent *reent, const char *path, int flags, int mode ) __attribute__((weak, alias("__enosys")));
int  _close_r( struct _reent *reent, int file )                              __attribute__((weak, alias("__enosys")));
int  _lseek_r( struct _reent *reent, int file, int pos, int whence )         __attribute__((weak, alias("__enosys")));
int   _read_r( struct _reent *reent, int file, char *buf, size_t size )      __attribute__((weak, alias("__enosys")));
int  _write_r( struct _reent *reent, int file, char *buf, size_t size )      __attribute__((weak, alias("__enosys")));
int _isatty_r( struct _reent *reent, int file )                              __attribute__((weak, alias("__enosys")));
int  _fstat_r( struct _reent *reent, int file, struct stat *st )             __attribute__((weak, alias("__enosys")));
int _getpid_r( struct _reent *reent )                                        __attribute__((weak, alias("__enosys")));
int   _kill_r( struct _reent *reent, int pid, int sig )                      __attribute__((weak, alias("__enosys")));

/* -------------------------------------------------------------------------- */

#endif // !USE_SEMIHOST && !USE_NOHOST
