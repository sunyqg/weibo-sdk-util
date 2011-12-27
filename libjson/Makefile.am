# Set the automake options.while execute the automake command, use this option.
AUTOMAKE_OPTIONS=foreign

lib_LIBRARIES = libjson.a

SOURCEDIR=$(top_srcdir)/src
INCLUDEDIR=$(top_srcdir)/include/json

h_sources = \
	$(SOURCEDIR)/json_batchallocator.h

cpp_sources = \
	$(SOURCEDIR)/json_reader.cpp \
	$(SOURCEDIR)/json_writer.cpp \
	$(SOURCEDIR)/json_value.cpp

INCLUDES = -I$(top_srcdir)/include

libjson_a_SOURCES = $(h_sources) $(cpp_sources)

# Include directory of install
libjsonincludedir=$(prefix)/include/json

# Library directory of install
libdir=$(prefix)/lib/Unix

# Install header file
libjsoninclude_HEADERS = \
        $(INCLUDEDIR)/autolink.h\
        $(INCLUDEDIR)/forwards.h\
        $(INCLUDEDIR)/reader.h\
        $(INCLUDEDIR)/writer.h\
        $(INCLUDEDIR)/config.h\
        $(INCLUDEDIR)/json.h\
        $(INCLUDEDIR)/value.h

AM_CXXFLAGS=-D_LINUX -DONLY_EPOLL -D_SERVER