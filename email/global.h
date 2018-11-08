/* GLOBAL.H - RSAREF types and constants
 */

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

#ifndef ERROR_ERRORS_ENCOUNTERED
#define ERROR_ERRORS_ENCOUNTERED 0xC0000001 
#endif //ERROR_ERRORS_ENCOUNTERED

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

// typedefs
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif

char *  __cdecl strcpy(char *, const char *);
size_t  __cdecl strlen(const char *);

void *  __cdecl memset(void *, int, size_t); 
int     __cdecl memcmp(const void *, const void *, size_t); 
void *  __cdecl memcpy(void *, const void *, size_t); 

#ifdef _DEBUG
#include <assert.h>
#define ASSERT( x ) assert( x )
#else
#define ASSERT( x ) 
#endif //_DEBUG

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
