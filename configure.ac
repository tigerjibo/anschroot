# Initialise autoconf
AC_INIT([anschroot], [1.0.0], [https://github.com/aaronmdjones/anschroot/issues], [anschroot], [https://github.com/aaronmdjones/anschroot])

# Initialise automake
AM_INIT_AUTOMAKE

# Test for required toolchain utilities or features
AC_PROG_CC_C99
PKG_PROG_PKG_CONFIG

# Test for libcap-ng
LIBCAPNG_ERRSTR="your libcap-ng is missing, broken or too old"
PKG_CHECK_MODULES([LIBCAPNG], [libcap-ng >= 0.7])
AC_CHECK_LIB([cap-ng], [capng_get_caps_process], AS_UNSET(PH), AC_MSG_ERROR($LIBCAPNG_ERRSTR))
AC_CHECK_LIB([cap-ng], [capng_clear], AS_UNSET(PH), AC_MSG_ERROR($LIBCAPNG_ERRSTR))
AC_CHECK_LIB([cap-ng], [capng_updatev], AS_UNSET(PH), AC_MSG_ERROR($LIBCAPNG_ERRSTR))
AC_CHECK_LIB([cap-ng], [capng_apply], AS_UNSET(PH), AC_MSG_ERROR($LIBCAPNG_ERRSTR))

# Generate the necessary files
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
