#include "./include/sfwrite.h"

/*


macros from stdarg.h
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void va_start(va_list ap, argN);  invoked to initialize ap to the beginning of the list before any calls to va_arg().

void va_copy(va_list dest, va_list src); initializes dest as a copy of src, as if the va_start() macro had been applied to dest 
followed by the same sequence of uses of the va_arg() macro as had previously been used to reach the present state of src.

type va_arg(va_list ap, type);  shall return the next argument in the list pointed to by ap

void va_end(va_list ap); used to clean up, it invalidates ap for use (unless va_start() or va_copy() is invoked again)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

functions with variable arguments that dont use these macros become non-portable

uses a mutex to lock an output stream
use vfprintf to print string
stdout fd is 1

*/

void sfwrite(pthread_mutex_t *lock, FILE* stream, char *fmt, ...){
	

	va_list ap;
	
	va_start(ap, fmt);

	

	pthread_mutex_lock(lock);
	
	vfprintf(stream, fmt, ap);
	
	fflush(0);
	pthread_mutex_unlock(lock);

	va_end(ap);

}