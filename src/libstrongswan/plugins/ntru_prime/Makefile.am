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
	ntru_prime_qske.h ntru_prime_qske.c \
	ntru_prime.h ntru_prime.c \
	ntru_prime_math_utils.h ntru_prime_math_utils.c \
	ntru_prime_pq_random.h ntru_prime_pq_random.c \
	ntru_prime_z_polynomial.h ntru_prime_z_polynomial.c \
	ntru_prime_fisher_yates_shuffle.h ntru_prime_fisher_yates_shuffle.c

libstrongswan_ntru_prime_la_LDFLAGS = -module -avoid-version
