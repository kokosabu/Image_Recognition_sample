if WITH_CUTTER
TESTS = run-test.sh
TESTS_ENVIRONMENT = NO_MAKE=yes CUTTER="$(CUTTER)"

noinst_LTLIBRARIES = test_decoder.la
endif

#INCLUDES = -I$(top_srcdir)/src
AM_CFLAGS = -I$(top_srcdir)/src $(CUTTER_CFLAGS)
LIBS = $(CUTTER_LIBS)\
	   $(top_builddir)/src/libdecoder.la

#AM_CFLAGS = $(CUTTER_CFLAGS)

LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined

test_decoder_la_SOURCES = test-decoder.c

echo-cutter:
	@echo $(CUTTER)
