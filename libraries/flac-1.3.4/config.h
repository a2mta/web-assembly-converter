/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Target processor is big endian. */
#define CPU_IS_BIG_ENDIAN 0

/* Target processor is little endian. */
#define CPU_IS_LITTLE_ENDIAN 1

/* Set FLAC__BYTES_PER_WORD to 8 (4 is the default) */
#define ENABLE_64_BIT_WORDS 0

/* libtool defines DLL_EXPORT for windows dll builds,
   but flac code relies on FLAC_API_EXPORTS instead. */
#ifdef DLL_EXPORT
#ifdef __cplusplus
# define FLACPP_API_EXPORTS
#else
# define FLAC_API_EXPORTS
#endif
#endif

/* define to align allocated memory on 32-byte boundaries */
/* #undef FLAC__ALIGN_MALLOC_DATA */

/* define if building for ia32/i386 */
/* #undef FLAC__CPU_IA32 */

/* define if building for PowerPC */
/* #undef FLAC__CPU_PPC */

/* define if building for PowerPC64 */
/* #undef FLAC__CPU_PPC64 */

/* define if building for SPARC */
/* #undef FLAC__CPU_SPARC */

/* define if building for x86_64 */
/* #undef FLAC__CPU_X86_64 */

/* define if you have docbook-to-man or docbook2man */
/* #undef FLAC__HAS_DOCBOOK_TO_MAN */

/* define if you are compiling for x86 and have the NASM assembler */
/* #undef FLAC__HAS_NASM */

/* define if you have the ogg library */
#define FLAC__HAS_OGG 0

/* define if compiler has __attribute__((target("cpu=power8"))) support */
/* #undef FLAC__HAS_TARGET_POWER8 */

/* define if compiler has __attribute__((target("cpu=power9"))) support */
/* #undef FLAC__HAS_TARGET_POWER9 */

/* Set to 1 if <x86intrin.h> is available. */
#define FLAC__HAS_X86INTRIN 0

/* define to disable use of assembly code */
/* #undef FLAC__NO_ASM */

/* define if building for Darwin / MacOS X */
/* #undef FLAC__SYS_DARWIN */

/* define if building for Linux */
#define FLAC__SYS_LINUX 1

/* define to enable use of Altivec instructions */
#define FLAC__USE_ALTIVEC 1

/* define to enable use of AVX instructions */
#define FLAC__USE_AVX 1

/* define to enable use of VSX instructions */
#define FLAC__USE_VSX 1

/* Compiler has the __builtin_bswap16 intrinsic */
#define HAVE_BSWAP16 1

/* Compiler has the __builtin_bswap32 intrinsic */
#define HAVE_BSWAP32 1

/* Define to 1 if you have the <byteswap.h> header file. */
#define HAVE_BYTESWAP_H 1

/* define if you have clock_gettime */
#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the <cpuid.h> header file. */
/* #undef HAVE_CPUID_H */

/* Define to 1 if C++ supports variable-length arrays. */
#define HAVE_CXX_VARARRAYS 1

/* Define to 1 if C supports variable-length arrays. */
#define HAVE_C_VARARRAYS 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#define HAVE_LANGINFO_CODESET 1

/* lround support */
#define HAVE_LROUND 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if the system has the type `socklen_t'. */
/* #undef HAVE_SOCKLEN_T */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if typeof works with your compiler. */
#define HAVE_TYPEOF 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <x86intrin.h> header file. */
/* #undef HAVE_X86INTRIN_H */

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST 

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define if debugging is disabled */
#define NDEBUG /**/

/* Name of package */
#define PACKAGE "flac"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "flac-dev@xiph.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "flac"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "flac 1.3.4"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "flac"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://www.xiph.org/flac/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.3.4"

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 8

/* The size of `void*', as computed by sizeof. */
#define SIZEOF_VOIDP 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "1.3.4"

/* Target processor is big endian. */
#define WORDS_BIGENDIAN 0

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to __typeof__ if your compiler spells it that way. */
/* #undef typeof */
