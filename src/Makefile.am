AM_CXXFLAGS = -std=c++20
bin_PROGRAMS = i3neostatus
i3neostatus_SOURCES =              \
	block_state.hpp            \
	click_event_listener.cpp   \
	click_event_listener.hpp   \
	config_file.cpp            \
	config_file.hpp            \
	dyn_load_lib.cpp           \
	dyn_load_lib.hpp           \
	generic_callback.hpp       \
	hide_block.cpp             \
	hide_block.hpp             \
	i3bar_data_conversions.cpp \
	i3bar_data_conversions.hpp \
	i3bar_data.hpp             \
	i3bar_protocol.cpp         \
	i3bar_protocol.hpp         \
	main.cpp                   \
	make_block.cpp             \
	make_block.hpp             \
	message_printing.cpp       \
	message_printing.hpp       \
	plugin_api.cpp             \
	plugin_api.hpp             \
	plugin_base.cpp            \
	plugin_base.hpp            \
	plugin_dev.hpp             \
	plugin_error.cpp           \
	plugin_error.hpp           \
	plugin_handle.cpp          \
	plugin_handle.hpp          \
	plugin_id.cpp              \
	plugin_id.hpp              \
	theme.hpp                  \
	thread_comm.hpp
i3neostatus_CPPFLAGS = -I$(top_srcdir)/deps/bits-and-bytes/include -I$(top_srcdir)/deps/libconfigfile/include
i3neostatus_CPPFLAGS += -DAM_PKGLIBDIR='"$(pkglibdir)"' -DAM_PKGDATADIR='"$(pkgdatadir)"'
i3neostatus_LDFLAGS = -export-dynamic
i3neostatus_LDADD = $(top_builddir)/deps/libconfigfile/src/libconfigfile.la
