#include "sqlite32.h"

/************** Begin file global.c ******************************************/
/*
** 2008 June 13
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains definitions of global variables and contants.
*/

/* An array to map all upper-case characters into their corresponding
** lower-case character. 
**
** SQLite only considers US-ASCII (or EBCDIC) characters.  We do not
** handle case conversions for the UTF character set since the tables
** involved are nearly as big or bigger than SQLite itself.
*/
SQLITE_PRIVATE const unsigned char sqlite3UpperToLower[] = {
#ifdef SQLITE_ASCII
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
     18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
     36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
     54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99,100,101,102,103,
    104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,
    122, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
    126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
    162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,
    198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,
    234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
    252,253,254,255
#endif
#ifdef SQLITE_EBCDIC
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, /* 0x */
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, /* 1x */
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, /* 2x */
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, /* 3x */
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, /* 4x */
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, /* 5x */
     96, 97, 66, 67, 68, 69, 70, 71, 72, 73,106,107,108,109,110,111, /* 6x */
    112, 81, 82, 83, 84, 85, 86, 87, 88, 89,122,123,124,125,126,127, /* 7x */
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143, /* 8x */
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,156,159, /* 9x */
    160,161,162,163,164,165,166,167,168,169,170,171,140,141,142,175, /* Ax */
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191, /* Bx */
    192,129,130,131,132,133,134,135,136,137,202,203,204,205,206,207, /* Cx */
    208,145,146,147,148,149,150,151,152,153,218,219,220,221,222,223, /* Dx */
    224,225,162,163,164,165,166,167,168,169,232,203,204,205,206,207, /* Ex */
    239,240,241,242,243,244,245,246,247,248,249,219,220,221,222,255, /* Fx */
#endif
};

/*
** The following 256 byte lookup table is used to support SQLites built-in
** equivalents to the following standard library functions:
**
**   isspace()                        0x01
**   isalpha()                        0x02
**   isdigit()                        0x04
**   isalnum()                        0x06
**   isxdigit()                       0x08
**   toupper()                        0x20
**   SQLite identifier character      0x40
**
** Bit 0x20 is set if the mapped character requires translation to upper
** case. i.e. if the character is a lower-case ASCII character.
** If x is a lower-case ASCII character, then its upper-case equivalent
** is (x - 0x20). Therefore toupper() can be implemented as:
**
**   (x & ~(map[x]&0x20))
**
** Standard function tolower() is implemented using the sqlite3UpperToLower[]
** array. tolower() is used more often than toupper() by SQLite.
**
** Bit 0x40 is set if the character non-alphanumeric and can be used in an 
** SQLite identifier.  Identifiers are alphanumerics, "_", "$", and any
** non-ASCII UTF character. Hence the test for whether or not a character is
** part of an identifier is 0x46.
**
** SQLite's versions are identical to the standard versions assuming a
** locale of "C". They are implemented as macros in sqliteInt.h.
*/
#ifdef SQLITE_ASCII
SQLITE_PRIVATE const unsigned char sqlite3CtypeMap[256] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 00..07    ........ */
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,  /* 08..0f    ........ */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 10..17    ........ */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 18..1f    ........ */
  0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  /* 20..27     !"#$%&' */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 28..2f    ()*+,-./ */
  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  /* 30..37    01234567 */
  0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 38..3f    89:;<=>? */

  0x00, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x02,  /* 40..47    @ABCDEFG */
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,  /* 48..4f    HIJKLMNO */
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,  /* 50..57    PQRSTUVW */
  0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x40,  /* 58..5f    XYZ[\]^_ */
  0x00, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x22,  /* 60..67    `abcdefg */
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,  /* 68..6f    hijklmno */
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,  /* 70..77    pqrstuvw */
  0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 78..7f    xyz{|}~. */

  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* 80..87    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* 88..8f    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* 90..97    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* 98..9f    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* a0..a7    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* a8..af    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* b0..b7    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* b8..bf    ........ */

  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* c0..c7    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* c8..cf    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* d0..d7    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* d8..df    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* e0..e7    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* e8..ef    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  /* f0..f7    ........ */
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40   /* f8..ff    ........ */
};
#endif

#ifndef SQLITE_USE_URI
# define  SQLITE_USE_URI 0
#endif

/*
** The following singleton contains the global configuration for
** the SQLite library.
*/
SQLITE_PRIVATE SQLITE_WSD struct Sqlite3Config sqlite3Config = {
   SQLITE_DEFAULT_MEMSTATUS,  /* bMemstat */
   1,                         /* bCoreMutex */
   SQLITE_THREADSAFE==1,      /* bFullMutex */
   SQLITE_USE_URI,            /* bOpenUri */
   0x7ffffffe,                /* mxStrlen */
   100,                       /* szLookaside */
   500,                       /* nLookaside */
   {0,0,0,0,0,0,0,0},         /* m */
   {0,0,0,0,0,0,0,0,0},       /* mutex */
   {0,0,0,0,0,0,0,0,0,0,0},   /* pcache */
   (void*)0,                  /* pHeap */
   0,                         /* nHeap */
   0, 0,                      /* mnHeap, mxHeap */
   (void*)0,                  /* pScratch */
   0,                         /* szScratch */
   0,                         /* nScratch */
   (void*)0,                  /* pPage */
   0,                         /* szPage */
   0,                         /* nPage */
   0,                         /* mxParserStack */
   0,                         /* sharedCacheEnabled */
   /* All the rest should always be initialized to zero */
   0,                         /* isInit */
   0,                         /* inProgress */
   0,                         /* isMutexInit */
   0,                         /* isMallocInit */
   0,                         /* isPCacheInit */
   0,                         /* pInitMutex */
   0,                         /* nRefInitMutex */
   0,                         /* xLog */
   0,                         /* pLogArg */
   0,                         /* bLocaltimeFault */
};


/*
** Hash table for global functions - functions common to all
** database connections.  After initialization, this table is
** read-only.
*/
SQLITE_PRIVATE SQLITE_WSD FuncDefHash sqlite3GlobalFunctions;

/*
** Constant tokens for values 0 and 1.
*/
SQLITE_PRIVATE const Token sqlite3IntTokens[] = {
   { "0", 1 },
   { "1", 1 }
};


/*
** The value of the "pending" byte must be 0x40000000 (1 byte past the
** 1-gibabyte boundary) in a compatible database.  SQLite never uses
** the database page that contains the pending byte.  It never attempts
** to read or write that page.  The pending byte page is set assign
** for use by the VFS layers as space for managing file locks.
**
** During testing, it is often desirable to move the pending byte to
** a different position in the file.  This allows code that has to
** deal with the pending byte to run on files that are much smaller
** than 1 GiB.  The sqlite3_test_control() interface can be used to
** move the pending byte.
**
** IMPORTANT:  Changing the pending byte to any value other than
** 0x40000000 results in an incompatible database file format!
** Changing the pending byte during operating results in undefined
** and dileterious behavior.
*/
#ifndef SQLITE_OMIT_WSD
SQLITE_PRIVATE int sqlite3PendingByte = 0x40000000;
#endif

/*
** Properties of opcodes.  The OPFLG_INITIALIZER macro is
** created by mkopcodeh.awk during compilation.  Data is obtained
** from the comments following the "case OP_xxxx:" statements in
** the vdbe.c file.  
*/
SQLITE_PRIVATE const unsigned char sqlite3OpcodeProperty[] = OPFLG_INITIALIZER;

/************** End of global.c **********************************************/
/************** Begin file ctime.c *******************************************/
/*
** 2010 February 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements routines used to report what compile-time options
** SQLite was built with.
*/

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS


/*
** An array of names of all compile-time options.  This array should 
** be sorted A-Z.
**
** This array looks large, but in a typical installation actually uses
** only a handful of compile-time options, so most times this array is usually
** rather short and uses little memory space.
*/
static const char * const azCompileOpt[] = {

/* These macros are provided to "stringify" the value of the define
** for those options in which the value is meaningful. */
#define CTIMEOPT_VAL_(opt) #opt
#define CTIMEOPT_VAL(opt) CTIMEOPT_VAL_(opt)

#ifdef SQLITE_32BIT_ROWID
  "32BIT_ROWID",
#endif
#ifdef SQLITE_4_BYTE_ALIGNED_MALLOC
  "4_BYTE_ALIGNED_MALLOC",
#endif
#ifdef SQLITE_CASE_SENSITIVE_LIKE
  "CASE_SENSITIVE_LIKE",
#endif
#ifdef SQLITE_CHECK_PAGES
  "CHECK_PAGES",
#endif
#ifdef SQLITE_COVERAGE_TEST
  "COVERAGE_TEST",
#endif
#ifdef SQLITE_DEBUG
  "DEBUG",
#endif
#ifdef SQLITE_DEFAULT_LOCKING_MODE
  "DEFAULT_LOCKING_MODE=" CTIMEOPT_VAL(SQLITE_DEFAULT_LOCKING_MODE),
#endif
#ifdef SQLITE_DISABLE_DIRSYNC
  "DISABLE_DIRSYNC",
#endif
#ifdef SQLITE_DISABLE_LFS
  "DISABLE_LFS",
#endif
#ifdef SQLITE_ENABLE_ATOMIC_WRITE
  "ENABLE_ATOMIC_WRITE",
#endif
#ifdef SQLITE_ENABLE_CEROD
  "ENABLE_CEROD",
#endif
#ifdef SQLITE_ENABLE_COLUMN_METADATA
  "ENABLE_COLUMN_METADATA",
#endif
#ifdef SQLITE_ENABLE_EXPENSIVE_ASSERT
  "ENABLE_EXPENSIVE_ASSERT",
#endif
#ifdef SQLITE_ENABLE_FTS1
  "ENABLE_FTS1",
#endif
#ifdef SQLITE_ENABLE_FTS2
  "ENABLE_FTS2",
#endif
#ifdef SQLITE_ENABLE_FTS3
  "ENABLE_FTS3",
#endif
#ifdef SQLITE_ENABLE_FTS3_PARENTHESIS
  "ENABLE_FTS3_PARENTHESIS",
#endif
#ifdef SQLITE_ENABLE_FTS4
  "ENABLE_FTS4",
#endif
#ifdef SQLITE_ENABLE_ICU
  "ENABLE_ICU",
#endif
#ifdef SQLITE_ENABLE_IOTRACE
  "ENABLE_IOTRACE",
#endif
#ifdef SQLITE_ENABLE_LOAD_EXTENSION
  "ENABLE_LOAD_EXTENSION",
#endif
#ifdef SQLITE_ENABLE_LOCKING_STYLE
  "ENABLE_LOCKING_STYLE=" CTIMEOPT_VAL(SQLITE_ENABLE_LOCKING_STYLE),
#endif
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  "ENABLE_MEMORY_MANAGEMENT",
#endif
#ifdef SQLITE_ENABLE_MEMSYS3
  "ENABLE_MEMSYS3",
#endif
#ifdef SQLITE_ENABLE_MEMSYS5
  "ENABLE_MEMSYS5",
#endif
#ifdef SQLITE_ENABLE_OVERSIZE_CELL_CHECK
  "ENABLE_OVERSIZE_CELL_CHECK",
#endif
#ifdef SQLITE_ENABLE_RTREE
  "ENABLE_RTREE",
#endif
#ifdef SQLITE_ENABLE_STAT2
  "ENABLE_STAT2",
#endif
#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
  "ENABLE_UNLOCK_NOTIFY",
#endif
#ifdef SQLITE_ENABLE_UPDATE_DELETE_LIMIT
  "ENABLE_UPDATE_DELETE_LIMIT",
#endif
#ifdef SQLITE_HAS_CODEC
  "HAS_CODEC",
#endif
#ifdef SQLITE_HAVE_ISNAN
  "HAVE_ISNAN",
#endif
#ifdef SQLITE_HOMEGROWN_RECURSIVE_MUTEX
  "HOMEGROWN_RECURSIVE_MUTEX",
#endif
#ifdef SQLITE_IGNORE_AFP_LOCK_ERRORS
  "IGNORE_AFP_LOCK_ERRORS",
#endif
#ifdef SQLITE_IGNORE_FLOCK_LOCK_ERRORS
  "IGNORE_FLOCK_LOCK_ERRORS",
#endif
#ifdef SQLITE_INT64_TYPE
  "INT64_TYPE",
#endif
#ifdef SQLITE_LOCK_TRACE
  "LOCK_TRACE",
#endif
#ifdef SQLITE_MEMDEBUG
  "MEMDEBUG",
#endif
#ifdef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
  "MIXED_ENDIAN_64BIT_FLOAT",
#endif
#ifdef SQLITE_NO_SYNC
  "NO_SYNC",
#endif
#ifdef SQLITE_OMIT_ALTERTABLE
  "OMIT_ALTERTABLE",
#endif
#ifdef SQLITE_OMIT_ANALYZE
  "OMIT_ANALYZE",
#endif
#ifdef SQLITE_OMIT_ATTACH
  "OMIT_ATTACH",
#endif
#ifdef SQLITE_OMIT_AUTHORIZATION
  "OMIT_AUTHORIZATION",
#endif
#ifdef SQLITE_OMIT_AUTOINCREMENT
  "OMIT_AUTOINCREMENT",
#endif
#ifdef SQLITE_OMIT_AUTOINIT
  "OMIT_AUTOINIT",
#endif
#ifdef SQLITE_OMIT_AUTOMATIC_INDEX
  "OMIT_AUTOMATIC_INDEX",
#endif
#ifdef SQLITE_OMIT_AUTORESET
  "OMIT_AUTORESET",
#endif
#ifdef SQLITE_OMIT_AUTOVACUUM
  "OMIT_AUTOVACUUM",
#endif
#ifdef SQLITE_OMIT_BETWEEN_OPTIMIZATION
  "OMIT_BETWEEN_OPTIMIZATION",
#endif
#ifdef SQLITE_OMIT_BLOB_LITERAL
  "OMIT_BLOB_LITERAL",
#endif
#ifdef SQLITE_OMIT_BTREECOUNT
  "OMIT_BTREECOUNT",
#endif
#ifdef SQLITE_OMIT_BUILTIN_TEST
  "OMIT_BUILTIN_TEST",
#endif
#ifdef SQLITE_OMIT_CAST
  "OMIT_CAST",
#endif
#ifdef SQLITE_OMIT_CHECK
  "OMIT_CHECK",
#endif
/* // redundant
** #ifdef SQLITE_OMIT_COMPILEOPTION_DIAGS
**   "OMIT_COMPILEOPTION_DIAGS",
** #endif
*/
#ifdef SQLITE_OMIT_COMPLETE
  "OMIT_COMPLETE",
#endif
#ifdef SQLITE_OMIT_COMPOUND_SELECT
  "OMIT_COMPOUND_SELECT",
#endif
#ifdef SQLITE_OMIT_DATETIME_FUNCS
  "OMIT_DATETIME_FUNCS",
#endif
#ifdef SQLITE_OMIT_DECLTYPE
  "OMIT_DECLTYPE",
#endif
#ifdef SQLITE_OMIT_DEPRECATED
  "OMIT_DEPRECATED",
#endif
#ifdef SQLITE_OMIT_DISKIO
  "OMIT_DISKIO",
#endif
#ifdef SQLITE_OMIT_EXPLAIN
  "OMIT_EXPLAIN",
#endif
#ifdef SQLITE_OMIT_FLAG_PRAGMAS
  "OMIT_FLAG_PRAGMAS",
#endif
#ifdef SQLITE_OMIT_FLOATING_POINT
  "OMIT_FLOATING_POINT",
#endif
#ifdef SQLITE_OMIT_FOREIGN_KEY
  "OMIT_FOREIGN_KEY",
#endif
#ifdef SQLITE_OMIT_GET_TABLE
  "OMIT_GET_TABLE",
#endif
#ifdef SQLITE_OMIT_INCRBLOB
  "OMIT_INCRBLOB",
#endif
#ifdef SQLITE_OMIT_INTEGRITY_CHECK
  "OMIT_INTEGRITY_CHECK",
#endif
#ifdef SQLITE_OMIT_LIKE_OPTIMIZATION
  "OMIT_LIKE_OPTIMIZATION",
#endif
#ifdef SQLITE_OMIT_LOAD_EXTENSION
  "OMIT_LOAD_EXTENSION",
#endif
#ifdef SQLITE_OMIT_LOCALTIME
  "OMIT_LOCALTIME",
#endif
#ifdef SQLITE_OMIT_LOOKASIDE
  "OMIT_LOOKASIDE",
#endif
#ifdef SQLITE_OMIT_MEMORYDB
  "OMIT_MEMORYDB",
#endif
#ifdef SQLITE_OMIT_OR_OPTIMIZATION
  "OMIT_OR_OPTIMIZATION",
#endif
#ifdef SQLITE_OMIT_PAGER_PRAGMAS
  "OMIT_PAGER_PRAGMAS",
#endif
#ifdef SQLITE_OMIT_PRAGMA
  "OMIT_PRAGMA",
#endif
#ifdef SQLITE_OMIT_PROGRESS_CALLBACK
  "OMIT_PROGRESS_CALLBACK",
#endif
#ifdef SQLITE_OMIT_QUICKBALANCE
  "OMIT_QUICKBALANCE",
#endif
#ifdef SQLITE_OMIT_REINDEX
  "OMIT_REINDEX",
#endif
#ifdef SQLITE_OMIT_SCHEMA_PRAGMAS
  "OMIT_SCHEMA_PRAGMAS",
#endif
#ifdef SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS
  "OMIT_SCHEMA_VERSION_PRAGMAS",
#endif
#ifdef SQLITE_OMIT_SHARED_CACHE
  "OMIT_SHARED_CACHE",
#endif
#ifdef SQLITE_OMIT_SUBQUERY
  "OMIT_SUBQUERY",
#endif
#ifdef SQLITE_OMIT_TCL_VARIABLE
  "OMIT_TCL_VARIABLE",
#endif
#ifdef SQLITE_OMIT_TEMPDB
  "OMIT_TEMPDB",
#endif
#ifdef SQLITE_OMIT_TRACE
  "OMIT_TRACE",
#endif
#ifdef SQLITE_OMIT_TRIGGER
  "OMIT_TRIGGER",
#endif
#ifdef SQLITE_OMIT_TRUNCATE_OPTIMIZATION
  "OMIT_TRUNCATE_OPTIMIZATION",
#endif
#ifdef SQLITE_OMIT_UTF16
  "OMIT_UTF16",
#endif
#ifdef SQLITE_OMIT_VACUUM
  "OMIT_VACUUM",
#endif
#ifdef SQLITE_OMIT_VIEW
  "OMIT_VIEW",
#endif
#ifdef SQLITE_OMIT_VIRTUALTABLE
  "OMIT_VIRTUALTABLE",
#endif
#ifdef SQLITE_OMIT_WAL
  "OMIT_WAL",
#endif
#ifdef SQLITE_OMIT_WSD
  "OMIT_WSD",
#endif
#ifdef SQLITE_OMIT_XFER_OPT
  "OMIT_XFER_OPT",
#endif
#ifdef SQLITE_PERFORMANCE_TRACE
  "PERFORMANCE_TRACE",
#endif
#ifdef SQLITE_PROXY_DEBUG
  "PROXY_DEBUG",
#endif
#ifdef SQLITE_SECURE_DELETE
  "SECURE_DELETE",
#endif
#ifdef SQLITE_SMALL_STACK
  "SMALL_STACK",
#endif
#ifdef SQLITE_SOUNDEX
  "SOUNDEX",
#endif
#ifdef SQLITE_TCL
  "TCL",
#endif
#ifdef SQLITE_TEMP_STORE
  "TEMP_STORE=" CTIMEOPT_VAL(SQLITE_TEMP_STORE),
#endif
#ifdef SQLITE_TEST
  "TEST",
#endif
#ifdef SQLITE_THREADSAFE
  "THREADSAFE=" CTIMEOPT_VAL(SQLITE_THREADSAFE),
#endif
#ifdef SQLITE_USE_ALLOCA
  "USE_ALLOCA",
#endif
#ifdef SQLITE_ZERO_MALLOC
  "ZERO_MALLOC"
#endif
};

/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" but the "SQLITE_" prefix
** is not required for a match.
*/
SQLITE_API int sqlite3_compileoption_used(const char *zOptName){
  int i, n;
  if( sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  /* Since ArraySize(azCompileOpt) is normally in single digits, a
  ** linear search is adequate.  No need for a binary search. */
  for(i=0; i<ArraySize(azCompileOpt); i++){
    if(   (sqlite3StrNICmp(zOptName, azCompileOpt[i], n)==0)
       && ( (azCompileOpt[i][n]==0) || (azCompileOpt[i][n]=='=') ) ) return 1;
  }
  return 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
SQLITE_API const char *sqlite3_compileoption_get(int N){
  if( N>=0 && N<ArraySize(azCompileOpt) ){
    return azCompileOpt[N];
  }
  return 0;
}

#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/************** End of ctime.c ***********************************************/
