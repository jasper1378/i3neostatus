# Prelude.
AC_PREREQ([2.72])
AC_INIT([i3neostatus], [0.0])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.17 foreign tar-ustar -Wall -Werror])
LT_PREREQ([2.5.0.14-9a4a-dirty])
LT_INIT([dlopen])
AC_LANG([C++])

# Checks for programs.
AC_PROG_AR
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_CXX([clang++])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_AR

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_REALLOC

# Misc.
AC_ARG_ENABLE([dyn-load-plugin-builtin], [AS_HELP_STRING([--disable-dyn-load-plugin-builtin], [do not dynamically load built-in plugins])], [enable_dyn_load_plugin_builtin=${enableval}], [enable_dyn_load_plugin_builtin=yes])
AS_IF([test "x${enable_dyn_load_plugin_builtin}" = xyes], \
      [AC_DEFINE([ENABLE_DYN_LOAD_PLUGIN_BUILTIN], 1, [Define to 1 if dyamically loading built-in plugins])])
AM_CONDITIONAL([ENABLE_DYN_LOAD_PLUGIN_BUILTIN], [test "x${enable_dyn_load_plugin_builtin}" = xyes])

# Output files.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile deps/Makefile src/Makefile include/i3neostatus/Makefile plugins/Makefile])
AC_CONFIG_SUBDIRS([deps/bits-and-bytes deps/libconfigfile])
AC_OUTPUT
