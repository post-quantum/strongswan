AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) \
	@COVERAGE_CFLAGS@

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ntru_prime.la
else
plugin_LTLIBRARIES = libstrongswan-ntru_prime.la
endif

libstrongswan_ntru_prime_la_SOURCES = \
	ntru_prime_plugin.h ntru_prime_plugin.c \
	ntru_prime_ke.h ntru_prime_ke.c \
	ntru_prime.h ntru_prime.c \
	math_utils.h math_utils.c \
	pq_random.h pq_random.c \
	z_polynomial.h z_polynomial.c \
	fisher_yates_shuffle.h fisher_yates_shuffle.c

libstrongswan_ntru_prime_la_LDFLAGS = -module -avoid-version
