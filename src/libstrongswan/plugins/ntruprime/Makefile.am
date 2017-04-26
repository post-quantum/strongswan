AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) \
	@COVERAGE_CFLAGS@

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ntruprime.la
else
plugin_LTLIBRARIES = libstrongswan-ntruprime.la
endif

libstrongswan_ntruprime_la_SOURCES = \
	ntruprime_plugin.h ntruprime_plugin.c \
	ntruprime_ke.h ntruprime_ke.c \
	ntruprime_qske.h ntruprime_qske.c \
	ntruprime.h ntruprime.c \
	ntruprime_math_utils.h ntruprime_math_utils.c \
	ntruprime_pq_random.h ntruprime_pq_random.c \
	ntruprime_z_polynomial.h ntruprime_z_polynomial.c \
	ntruprime_fisher_yates_shuffle.h ntruprime_fisher_yates_shuffle.c

libstrongswan_ntruprime_la_LDFLAGS = -module -avoid-version
