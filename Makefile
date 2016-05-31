prefix=/usr/local
sysconfdir=${prefix}/etc
localstatedir=${prefix}/var

CC=gcc

LDFLAGS= -lm -ldl -lc

CFLAGS= -Isdna/ -Isdna/sqlite-amalgamation-3100200 -D_GNU_SOURCE -O2 -fstack-protector --param=ssp-buffer-size=4 -Isdna/nacl/include
CFLAGS+=-DSYSCONFDIR="\"$(sysconfdir)\"" -DLOCALSTATEDIR="\"$(localstatedir)\""
CFLAGS+=-DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_DATETIME_FUNCS -DSQLITE_OMIT_COMPILEOPTION_DIAGS -DSQLITE_OMIT_DEPRECATED -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_VIRTUALTABLE -DSQLITE_OMIT_AUTHORIZATION
CFLAGS+=-fPIC
CFLAGS+=-Wall -Werror
# Solaris magic
CFLAGS+=-DSHA2_USE_INTTYPES_H -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -D__EXTENSIONS__=1
# OSX magic to compensate for the Solaris magic
CFLAGS+=-D_DARWIN_C_SOURCE
# More warnings, discover problems that only happen on some archs
CFLAGS+=-Wextra
# Security enhancements from Debian
CFLAGS+=-Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -ferror-limit=0

DEFS=	-DPACKAGE_NAME=\"servald\" -DPACKAGE_TARNAME=\"servald\" -DPACKAGE_VERSION=\"0.9\" -DPACKAGE_STRING=\"servald\ 0.9\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DHAVE_FUNC_ATTRIBUTE_ALIGNED=1 -DHAVE_FUNC_ATTRIBUTE_FORMAT=1 -DHAVE_FUNC_ATTRIBUTE_MALLOC=1 -DHAVE_FUNC_ATTRIBUTE_UNUSED=1 -DHAVE_FUNC_ATTRIBUTE_USED=1 -DHAVE_VAR_ATTRIBUTE_SECTION_SEG=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_MATH_H=1 -DHAVE_FLOAT_H=1 -DHAVE_LIBC=1 -DHAVE_GETPEEREID=1 -DHAVE_BCOPY=1 -DHAVE_BZERO=1 -DHAVE_BCMP=1 -DSIZEOF_OFF_T=8 -DHAVE_STDIO_H=1 -DHAVE_ERRNO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRINGS_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STRING_H=1 -DHAVE_ARPA_INET_H=1 -DHAVE_SYS_SOCKET_H=1 -DHAVE_SYS_MMAN_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_SYS_UCRED_H=1 -DHAVE_SYS_STATVFS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_POLL_H=1 -DHAVE_NETDB_H=1 -DHAVE_NET_IF_H=1 -DHAVE_NETINET_IN_H=1 -DHAVE_IFADDRS_H=1 -DHAVE_NET_ROUTE_H=1 -DHAVE_SIGNAL_H=1 -DHAVE_SYS_FILIO_H=1 -DHAVE_SYS_SOCKIO_H=1 -DHAVE_SYS_SOCKET_H=1 -DHAVE_SINF=1 -DHAVE_COSF=1 -DHAVE_TANF=1 -DHAVE_ASINF=1 -DHAVE_ACOSF=1 -DHAVE_ATANF=1 -DHAVE_ATAN2F=1 -DHAVE_CEILF=1 -DHAVE_FLOORF=1 -DHAVE_POWF=1 -DHAVE_EXPF=1 -DHAVE_LOGF=1 -DHAVE_LOG10F=1 -DHAVE_STRLCPY=1

RPC_SRC=$(wildcard *.c)
include obj_files

all: servalrpc

servalrpc: $(RPC_SRC)
	@$(CC) -o $@ $^ $(CFLAGS) $(DEFS) $(OBJS) $(LDFLAGS)

clean:
	@rm servalrpc
