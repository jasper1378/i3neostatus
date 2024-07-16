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
	module_api.cpp             \
	module_api.hpp             \
	module_base.cpp            \
	module_base.hpp            \
	module_dev.hpp             \
	module_error.cpp           \
	module_error.hpp           \
	module_handle.cpp          \
	module_handle.hpp          \
	module_id.cpp              \
	module_id.hpp              \
	theme.hpp                  \
	thread_comm.hpp
i3neostatus_CPPFLAGS = -I$(top_srcdir)/deps/bits-and-bytes/include -I$(top_srcdir)/deps/libconfigfile/include
i3neostatus_CPPFLAGS += -DAM_PKGLIBDIR='"$(pkglibdir)"' -DAM_PKGDATADIR='"$(pkgdatadir)"'
i3neostatus_LDFLAGS = -export-dynamic
i3neostatus_LDADD = $(top_builddir)/deps/libconfigfile/src/libconfigfile.la
