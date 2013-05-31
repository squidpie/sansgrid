#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sansgrid], [0.1], [])
AM_INIT_AUTOMAKE([-Wall silent-rules foreign])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB

case $(uname -m) in
	*arm*)		SANSGRIDROUTER_SYSTEM='sg_serial_rpi.$(OBJEXT)' ;;
	*)		SANSGRIDROUTER_SYSTEM='sg_serial_generic.$(OBJEXT)' ;;
esac

# Checks for libraries.
AC_CHECK_LIB([check], [srunner_create], [], AC_MSG_ERROR([check unit testing library is required]))
AC_CHECK_LIB([pthread], [pthread_create], [], AC_MSG_ERROR([posix thread library required]))
AC_CHECK_LIB([wiringPi], [wiringPiSPISetup], [], SANSGRIDROUTER_SYSTEM='sg_serial_generic.$(OBJEXT)')
# Have to link with -lrt for glibc versions before 2.17
AC_CHECK_LIB([rt], [clock_gettime], [], AC_MSG_WARN([librt required for glibc versions before 2.17]))

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h stdint.h stdlib.h string.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile \
		 sansgridrouter/Makefile])
                 #test/Makefile
                 #test/dispatch/Makefile
                 #test/routing/Makefile])
AC_SUBST([SANSGRIDROUTER_SYSTEM])
AC_OUTPUT
