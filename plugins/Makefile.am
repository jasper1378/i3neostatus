plugindir = $(pkglibdir)/plugins
AM_CXXFLAGS = -std=c++20
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/deps/bits-and-bytes/include -I$(top_srcdir)/deps/libconfigfile/include
if ENABLE_DYN_LOAD_PLUGIN_BUILTIN
  AM_LDFLAGS = -module -avoid-version
  plugin_LTLIBRARIES = test_plugin.la
  test_plugin_la_SOURCES = test_plugin.cpp
else
  pkglib_LTLIBRARIES = libplugins_builtin.la
  libplugins_builtin_la_SOURCES = libplugins_builtin.hpp test_plugin.cpp
endif
