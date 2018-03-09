AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-nistpqc.la
else
plugin_LTLIBRARIES = libstrongswan-nistpqc.la
endif

libstrongswan_nistpqc_la_SOURCES = \
	nistpqc_plugin.h nistpqc_plugin.c \
	nistpqc_ke.h nistpqc_ke.c

libstrongswan_nistpqc_la_LDFLAGS = -module -avoid-version
libstrongswan_nistpqc_la_LIBADD  = -lnistpqc
