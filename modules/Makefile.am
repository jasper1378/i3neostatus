moduledir = $(pkglibdir)/modules
AM_CXXFLAGS = -std=c++20
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/deps/bits-and-bytes/include -I$(top_srcdir)/deps/libconfigfile/include
AM_LDFLAGS = -module -avoid-version
module_LTLIBRARIES = test_module.la

test_module_la_SOURCES = test_module.cpp
