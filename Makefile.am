bin_PROGRAMS = memcached
pkginclude_HEADERS = protocol_binary.h
noinst_PROGRAMS = memcached-debug sizes testapp
if !BUILD_MINGW
noinst_PROGRAMS += timedrun
endif

BUILT_SOURCES=

testapp_SOURCES = testapp.c util.c util.h stats_prefix.c stats_prefix.h jenkins_hash.c murmur3_hash.c hash.c hash.h cache.c

if !BUILD_MINGW
timedrun_SOURCES = timedrun.c
endif

memcached_SOURCES = memcached.c memcached.h \
                    hash.c hash.h \
                    jenkins_hash.c jenkins_hash.h \
                    murmur3_hash.c murmur3_hash.h \
                    slabs.c slabs.h \
                    items.c items.h \
                    assoc.c assoc.h \
                    thread.c \
                    stats_prefix.c stats_prefix.h \
                    util.c util.h \
                    trace.h cache.c cache.h sasl_defs.h \
                    bipbuffer.c bipbuffer.h \
                    logger.c logger.h \
                    crawler.c crawler.h \
                    itoa_ljust.c itoa_ljust.h \
                    slab_automove.c slab_automove.h \
                    authfile.c authfile.h \
                    restart.c restart.h

if !BUILD_MINGW
memcached_SOURCES += daemon.c
endif

if BUILD_SOLARIS_PRIVS
memcached_SOURCES += solaris_priv.c
endif

if BUILD_LINUX_PRIVS
memcached_SOURCES += linux_priv.c
endif

if BUILD_OPENBSD_PRIVS
memcached_SOURCES += openbsd_priv.c
endif

if BUILD_FREEBSD_PRIVS
memcached_SOURCES += freebsd_priv.c
endif

if BUILD_MINGW
memcached_SOURCES += mingw/src/mingw_all.c

testapp_SOURCES += mingw/src/mingw_logger.c
testapp_SOURCES += mingw/src/mingw_signal.c
testapp_SOURCES += mingw/src/mingw_socket.c
testapp_SOURCES += mingw/src/mingw_unistd.c
endif

if ENABLE_SASL
memcached_SOURCES += sasl_defs.c
endif

if ENABLE_EXTSTORE
memcached_SOURCES += extstore.c extstore.h \
                     crc32c.c crc32c.h \
                     storage.c storage.h \
                     slab_automove_extstore.c slab_automove_extstore.h
endif

if ENABLE_TLS
memcached_SOURCES += tls.c tls.h
endif

memcached_debug_SOURCES = $(memcached_SOURCES)
memcached_CPPFLAGS = -DNDEBUG
memcached_debug_LDADD = @PROFILER_LDFLAGS@
memcached_debug_CFLAGS = @PROFILER_FLAGS@

memcached_LDADD =
memcached_DEPENDENCIES =
memcached_debug_DEPENDENCIES =
CLEANFILES=

if BUILD_LINUX_PRIVS
memcached_LDADD += -lseccomp
memcached_debug_LDADD += -lseccomp
endif

if BUILD_MINGW
MINGW_DEFS = -D_POSIX -D_WIN32_WINNT=0x0600 -DWIN32_LEAN_AND_MEAN
MINGW_DEFS += -DWINDOWS_ONLY_SIGNALS -DDISABLE_RUNAS_USER -DDISABLE_UNIX_SOCKET
MINGW_DEFS += -DDISABLE_RLIMIT_NOFILE -DDISABLE_MLOCKALL -DDISABLE_COREDUMP
MINGW_INCS = -I$(top_srcdir)/mingw/include -I$(top_srcdir)/mingw/src

memcached_CPPFLAGS += $(MINGW_DEFS)
memcached_CPPFLAGS += $(MINGW_INCS)
memcached_debug_CPPFLAGS = $(MINGW_DEFS)
memcached_debug_CPPFLAGS += $(MINGW_INCS)
testapp_CPPFLAGS = $(MINGW_DEFS)
testapp_CPPFLAGS += $(MINGW_INCS)
sizes_CPPFLAGS = $(MINGW_DEFS)
sizes_CPPFLAGS += $(MINGW_INCS)
endif

if BUILD_DTRACE
BUILT_SOURCES += memcached_dtrace.h
CLEANFILES += memcached_dtrace.h
endif

if DTRACE_INSTRUMENT_OBJ
memcached_LDADD += memcached_dtrace.o
memcached_DEPENDENCIES += memcached_dtrace.o
memcached_debug_LDADD += memcached_debug_dtrace.o
memcached_debug_DEPENDENCIES += memcached_debug_dtrace.o
CLEANFILES += memcached_dtrace.o memcached_debug_dtrace.o
endif

memcached_debug_CFLAGS += -DMEMCACHED_DEBUG

memcached_dtrace.h: memcached_dtrace.d
	${DTRACE} -h -s memcached_dtrace.d
	sed -e 's,void \*,const void \*,g' memcached_dtrace.h | \
            sed -e 's,char \*,const char \*,g' | tr '\t' ' ' > mmc_dtrace.tmp
	mv mmc_dtrace.tmp memcached_dtrace.h

memcached_dtrace.o: $(memcached_OBJECTS)
	$(DTRACE) $(DTRACEFLAGS) -G -o memcached_dtrace.o -s ${srcdir}/memcached_dtrace.d $(memcached_OBJECTS)

memcached_debug_dtrace.o: $(memcached_debug_OBJECTS)
	$(DTRACE) $(DTRACEFLAGS) -G -o memcached_debug_dtrace.o -s ${srcdir}/memcached_dtrace.d $(memcached_debug_OBJECTS)


SUBDIRS = doc
DIST_DIRS = scripts
EXTRA_DIST = doc scripts t memcached.spec memcached_dtrace.d version.m4 README.md LICENSE.bipbuffer

MOSTLYCLEANFILES = *.gcov *.gcno *.gcda *.tcov

if BUILD_MINGW
TESTDIR=$(srcdir)/mingw/t
endif

if ENABLE_TLS
test_tls:
	$(MAKE) SSL_TEST=1 test

if !BUILD_MINGW
test_basic_tls:
	@if test $(SSL_TEST)1 != 1; then \
	  echo "Running basic tests with TLS"; \
	  $(builddir)/testapp; \
	  prove $(srcdir)/t/binary.t $(srcdir)/t/getset.t $(srcdir)/t/ssl*; \
	  echo "Finished running basic TLS tests"; \
	else \
	  echo "Set SSL_TEST=1 to enable TLS tests"; \
	fi
else
test_basic_tls:
	echo "Running basic tests with TLS"
	mkdir -p tmp
	ln -sf $(TESTDIR)
	SSL_TEST=1 WINE_TEST=1 wine $(builddir)/testapp$(EXEEXT)
	SSL_TEST=1 WINE_TEST=1 prove $(TESTDIR)/binary.t $(TESTDIR)/getset.t $(TESTDIR)/ssl*
	echo "Finished running basic TLS tests"
endif

endif

if !BUILD_MINGW
test:	memcached-debug sizes testapp
	$(builddir)/sizes
	$(builddir)/testapp
if ENABLE_TLS
	@if test $(SSL_TEST)1 = 1; then \
          $(MAKE) SSL_TEST=1  test_basic_tls; \
	fi
endif
	@if test -n "${PARALLEL}"; then \
	  prove $(srcdir)/t -j ${PARALLEL}; \
	else \
	  prove $(srcdir)/t; \
	fi
	@if test `basename $(PROFILER)` = "gcov"; then \
	  for file in memcached_debug-*.gc??; do \
	    mv -f $$file `echo $$file | sed 's/memcached_debug-//'`; \
	  done && \
	  for file in *.gcda; do \
	    srcfile=`echo $$file | sed 's/.gcda/.c/'`; \
	    if test -n "`echo $(memcached_debug_SOURCES) | grep $$srcfile`"; then \
	      echo `$(PROFILER) $$srcfile` | sed 's/'$$srcfile':.*//'; \
	    fi \
	  done \
	elif test `basename $(PROFILER)` = "tcov"; then \
	  files=`grep SRCFILE memcached-debug.profile/tcovd | sed 's/SRCFILE://' | sort | uniq` && \
	  $(PROFILER) -x memcached-debug.profile $$files 2>&1; \
	  for file in *.tcov; do \
	    srcfile=`echo $$file | sed 's/.tcov//'`; \
	    if test -n "`echo $(memcached_debug_SOURCES) | grep $$srcfile`"; then \
	      echo $$srcfile : `grep 'Percent of the file executed' $$file`; \
	    fi \
	  done \
	else :; fi
else
test:	memcached-debug$(EXEEXT) sizes$(EXEEXT) testapp$(EXEEXT)
	wine $(builddir)/sizes$(EXEEXT)
	mkdir -p tmp
	ln -sf $(TESTDIR)
	WINE_TEST=1 wine $(builddir)/testapp$(EXEEXT)
	WINE_TEST=1 prove $(TESTDIR)
endif

docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`cat version.num`") | doxygen -

dist-hook:
	rm -f $(distdir)/*/*~ $(distdir)/t/lib/*~ $(distdir)/*~

maintainer-clean-local:
	-rm Makefile.in
	-rm aclocal.m4
	-rm config.guess
	-rm config.sub
	-rm depcomp
	-rm install-sh
	-rm ltmain.sh
	-rm missing
	-rm configure
	-rm config.log
	-rm config.status
