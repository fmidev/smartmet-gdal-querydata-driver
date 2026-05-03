REQUIRES = fmt libpqxx

include $(shell smartbuildcfg --prefix)/share/smartmet/devel/makefile.inc

PROG = $(patsubst %.cpp,%,$(wildcard *Test.cpp))

COMPILE_TESTS=$(wildcard *Compile.cpp)

CFLAGS = -DUNIX -D_REENTRANT -O0 -g $(FLAGS)

ifeq ($(USE_CLANG),yes)
CharsetToolsTest: CFLAGS += -Wno-invalid-source-encoding
endif

# No need to relink if only libsmartmet-macgyver.so is changed
# Tracking include changes should be sufficient
MACGYVER_TARGET :=

ifneq (,$(findstring test-installed,$(MAKECMDGOALS)))

MACGYVER_INCLUDES := -I$(includedir)/smartmet/macgyver
MACGYVER_LDFLAGS := -L $(libdir) -lsmartmet-macgyver

else

MACGYVER_INCLUDES := -I../macgyver
MACGYVER_LDFLAGS := ../libsmartmet-macgyver.so

endif

INCLUDES += $(MACGYVER_INCLUDES) -I /usr/include/smartmet

EXTRA_LIBS :=

LIBS += $(PREFIX_LDFLAGS) \
	$(MACGYVER_LDFLAGS) \
	$(REQUIRED_LIBS) \
	$(EXTRA_LIBS) \
	-lboost_locale \
	-lboost_serialization \
	-lboost_regex \
	-lboost_iostreams \
	-lboost_thread \
	-lboost_chrono \
	-lpthread

DateTest LocalDateTimeTest TimeDurationTest: EXTRA_LIBS += -lboost_date_time
CacheTest: EXTRA_LIBS += -lboost_filesystem

#######  Test database for PostgrSQLConnection tests ######
TEST_DB_DIR := $(shell pwd)/tmp-geonames-db

ifdef CI
DATABASE_HOST = "$(TEST_DB_DIR)"
DATABASE_PORT = 5444
else
ifdef LOCAL_ONLY
DATABASE_HOST = 127.0.0.1
DATABASE_PORT = 5444
else
DATABASE_HOST = "smartmet-test"
DATABASE_PORT = 5444
endif
endif


ifdef CI
TEST_PREPARE_TARGETS := start-geonames-db
TEST_FINISH_TARGETS :=  stop-geonames-db
else
TEST_PREPARE_TARGETS := dummy
TEST_FINISH_TARGETS := dummy
endif

##########################################################

all: $(PROG)
clean:
	rm -f $(PROG) *~
	rm -f PostgreSQLConnectionTest.conf
	rm -rf tmp-geonames-db
	rm -f tmp-geonames-db.log
	rm -rf tmp obj

# FIXME: clean if switching between test and test-installed or use different directories
test test-installed: $(PROG) $(patsubst %.cpp,%.CompileOnly,$(COMPILE_TESTS)) $(TEST_PREPARE_TARGETS)
	@echo Running tests:
	@rm -f *.err
	@for prog in $(PROG); do \
	  ( ./$$prog || touch $$prog.err ) ; \
	done;
	$(MAKE) $(TEST_FINISH_TARGETS);
	@test `find . -name \*.err | wc -l` = "0" || ( echo ; echo "The following tests have errors:" ; \
		for i in *.err ; do echo `basename $$i .err`; done ; rm -f *.err ; false )


$(PROG) : % : obj/%.o $(MACGYVER_TARGET)
	$(CXX) $(CFLAGS) $< $(LIBS) -o $@

obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d, $@) -MT $@ -o $@ $<

obj/CharsetToolTest.o: FLAGS += -Wno-invalid-source-encoding
obj/StringConversionTest.o: FLAGS += -Wno-enum-constexpr-conversion

PostgreSQLConnectionTest: PostgreSQLConnectionTest.conf

%.CompileOnly : %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o /dev/null $<


geonames-database:
	@-$(MAKE) stop-geonames-db
	rm -rf tmp-geonames-db
	if ! /usr/share/smartmet/test/db/create-local-db.sh $(TEST_DB_DIR) >tmp-geonames-db.log 2>&1 ; then \
	    cat tmp-geonames-db.log; \
	    false; \
	fi

start-geonames-db: geonames-database
	/usr/share/smartmet/test/db/test-db-ctl.sh $(TEST_DB_DIR) start -w

stop-geonames-db:
	-/usr/share/smartmet/test/db/test-db-ctl.sh $(TEST_DB_DIR) stop -w

PostgreSQLConnectionTest.conf:
	@echo $(DATABASE_HOST) $(DATABASE_PORT) fminames fminames_user fminames_pw >$@

dummy:
	true

.PHONY: PostgreSQLConnectionTest.conf

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
