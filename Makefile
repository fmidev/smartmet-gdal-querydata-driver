SUBNAME = macgyver
LIB = smartmet-$(SUBNAME)
SPEC = smartmet-library-$(SUBNAME)
INCDIR = smartmet/$(SUBNAME)

# Installation directories

processor := $(shell uname -p)

# Build configuration parameters

# NOTE: USE_OS_TZDB is determined by the presence of the USE_OS_TZDB macro in date_time/Base.h
#       and its value

# [USE_OS_TZDB==0 only]: If TZDB_REMOTE_API!=0 is defined, the tz.cpp file will try to download latest
# time zone database version from IANA's ftp server. If TZDB_REMOTE_API==0, the tz.cpp file will assume
# that the time zone database (source files) is already available
TZDB_REMOTE_API := 1

# [USE_OS_TZDB==1 and TBZD_REMOTE_API=0 only]: The folder where to downoad the time zone database files
# Files are downloaded from IANA's server and extracted to the subdirectory "tznames" in the specified
# directory. Value $(HOME)/Download is used when not specified (empty) or when TZBD_REMOTE_API==0
TZDB_FOLDER := $(datadir)/smartmet/timezones

EXTRA_TZDB_LIBS :=
ifeq ($(USE_OS_TZDB),0)
ifeq ($(TZDB_REMOTE_API),1)
	EXTRA_TZDB_LIBS -lcurl
endif
endif
export EXTRA_TZDB_LIBS

# Compiler options

DEFINES = -DUNIX -D_REENTRANT -DPQXX_HIDE_EXP_OPTIONAL

REQUIRES := libpqxx icu-i18n fmt ctpp2 filesystem

include $(shell smartbuildcfg --prefix)/share/smartmet/devel/makefile.inc

LIBS += \
	$(PREFIX_LDFLAGS) \
	-lboost_regex \
	-lboost_serialization \
	-lboost_chrono \
	-lboost_iostreams \
	-lboost_thread \
	-ldouble-conversion \
	$(REQUIRED_LIBS) \
	$(EXTRA_TZDB_LIBS) \
	-lpthread -lrt

RPMBUILD_OPT ?=

# Configuration

# Detects used impementation of calendar classes implementation and
# its parameters from macgyver/date_time/Base.h.
TZ_CFLAGS :=
EXTRA_SRC_DIRS :=
CALENDAR_USES_STD_CHRONO := $(shell echo -e '\043include "date_time/Base.h"' | $(CXX) -x c++ -E -dD -Imacgyver - | awk '/^\043define FMI_CALENDAR_USES_STD_CHRONO/ {print $$3}')
ifeq ($(CALENDAR_USES_STD_CHRONO),1)
	# std::chrono::implemantation is sufficient and is beinf used
	$(info CALENDAR_USES_STD_CHRONO: $(CALENDAR_USES_STD_CHRONO))
else
	# std::chrono::implemantation is too old -> use Date libraray
	EXTRA_SRC_DIRS := $(SUBNAME)/date_time/date
	USE_OS_TZDB := $(shell echo -e '\043include "date_time/Base.h"' | $(CXX) -x c++ -E -dD -Imacgyver - | awk '/^\043define USE_OS_TZDB/ {print $$3}')
    $(info USE_OS_TZDB: $(USE_OS_TZDB))
    $(info EXTRA_SRC_DIRS: $(EXTRA_SRC_DIRS))

	ifeq ($(USE_OS_TZDB),)
		$(error USE_OS_TZDB not defined in $(SUBNAME)/date_time/Base.h)
	endif

	ifeq ($(USE_OS_TZDB),0)
		# USE_OS_TZDB is defined and its value is 0
		TZ_CFLAGS := -DUSE_OS_TZDB=0
		ifeq ($(TZDB_REMOTE_API),0)
			TZ_CFLAGS += -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0
			ifneq ($(TZDB_FOLDER),)
				TZ_CFLAGS += -DTZ_SOURCE_FOLDER="\"$(TZDB_FOLDER)\""
			endif
		else
			LIBS += -lcurl
		endif
	else # USE_OS_TZDB is defined and its value is not 0
		TZ_CFLAGS := -DUSE_OS_TZDB=1
	endif
endif

# What to install

LIBFILE = libsmartmet-$(SUBNAME).so

# Compilation directories

SRC_DIRS = $(SUBNAME) $(SUBNAME)/date_time $(EXTRA_SRC_DIRS)

vpath %.cpp $(SRC_DIRS)
vpath %.h $(SRC_DIRS)

# The files to be compiled

# Headers that are only needed when building the library. These are not installed
INTERNAL_HDRS = \
	date_time/Internal.h \
	date_time/ParserDefinitions.h \
	date_time/date/tz_private.h \
	PostgreSQLConnectionImpl.h \
	TimeParserDefinitions.h

SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
HDRS = $(filter-out $(INTERNAL_HEADERS), $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h)))
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

# All object files are put into a subdirectory obj. As result the source files
# can not have the same name in different subdirectories. Detect it early to avoid
# confusing error messages later
ifneq ($(words $(sort $(OBJS))),$(words $(OBJS)))
$(info $(shell for f in $(OBJS); do echo $$f; done | sort | uniq -c | grep -v '^ *1 ' | sed -e 's/^ *//' -e 's/ /: /'))
$(error Source file name conflict detected)
endif

INCLUDES := -Iinclude $(INCLUDES)


ifeq ($(USE_OS_TZDB),0)
TZ_CFLAGS = -DUSE_OS_TZDB=0
ifeq ($(TZDB_REMOTE_API),0)
TZ_CFLAGS += -DHAS_REMOTE_API=1 -DAUTO_DOWNLOAD=0
else
LIBS += -lcurl
endif
ifneq ($(TZDB_FOLDER),)
TZ_CFLAGS += -DTZ_SOURCE_FOLDER="\"$(datadir)/smartmet/timezones\""
endif
else
TZ_CFLAGS = -DUSE_OS_TZDB=1
endif

.PHONY: test rpm

# The rules

all: objdir $(LIBFILE)
debug: all
release: all
profile: all

$(LIBFILE): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -rdynamic -o $(LIBFILE) $(OBJS) $(LIBS)
	@echo Checking $(LIBFILE) for unresolved references
	@if ldd -r $(LIBFILE) 2>&1 | c++filt | grep ^undefined\ symbol |\
			grep -Pv ':\ __(?:(?:a|t|ub)san_|sanitizer_)'; \
		then rm -v $(LIBFILE); \
		exit 1; \
	fi

clean:  $(CLEAN_TARGETS)
	rm -f $(LIBFILE) *~ $(SUBNAME)/*~
	rm -rf $(objdir)
	$(MAKE) -C test clean
	$(MAKE) -C examples clean

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp test/*.cpp

examples:
	$(MAKE) -C examples

install:
	@mkdir -p $(includedir)/$(INCDIR)
	@list='$(HDRS)'; set -x; \
	for hdr in $$list; do \
	  HDR=$$(echo $$hdr | sed -e 's:^$(SUBNAME)/::'); \
	  subdir=$$(dirname $$HDR); \
	  echo $(INSTALL_DATA) -D $$hdr $(includedir)/$(INCDIR)/$(subdir)/$$HDR; \
	  $(INSTALL_DATA) -D $$hdr $(includedir)/$(INCDIR)/$(subdir)/$$HDR; \
	set +x; done
	@mkdir -p $(libdir)
	$(INSTALL_PROG) $(LIBFILE) $(libdir)/$(LIBFILE)

test test-installed:
	$(MAKE) -C test $@

rpm: clean $(SPEC).spec
	rm -f $(SPEC).tar.gz # Clean a possible leftover from previous attempt
	tar -czvf $(SPEC).tar.gz --exclude-vcs --transform "s,^,$(SPEC)/," *
	rpmbuild -tb $(SPEC).tar.gz $(RPMBUILD_OPT)
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp

objdir:
	mkdir -p $(objdir)

obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d, $@) -MT $@ -o $@ $<

obj/tz.o: CFLAGS += $(TZ_CFLAGS)

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile-abicheck.inc
