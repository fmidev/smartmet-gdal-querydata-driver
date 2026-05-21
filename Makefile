# =============================================================================
# Build of the QUERYDATA GDAL driver.
#
# The driver depends on three smartmet libraries (newbase, macgyver, gis),
# which must be installed as static libraries plus headers from these RPM
# packages:
#
#   smartmet-library-newbase-static   smartmet-library-newbase-devel
#   smartmet-library-macgyver-static  smartmet-library-macgyver-devel
#   smartmet-library-gis-static       smartmet-library-gis-devel
#
# The static archives are linked into the plugin so the installed
# `gdal_querydata.so` carries no runtime dependency on the smartmet shared
# libraries — only on the stock geospatial stack (gdal, fmt, geos, proj, ...).
#
# Targets:
#   make              build gdal_querydata.so
#   make install      install plugin into $(GDAL_PLUGIN_DIR)
#   make rpm          build the binary RPM
#   make clean
# =============================================================================

SUBNAME := qd_driver
DRIVER  := querydata
PLUGIN  := gdal_$(DRIVER).so
SPEC    := smartmet-gdal-querydata-driver

# -- Compiler and flags ------------------------------------------------------
# Use pkg-config directly so the build works on any distro that has the stock
# packages, without needing the smartmet build environment (smartbuildcfg).
# For non-stock GDAL installs (e.g. /usr/gdal312/), let pkg-config find them
# via PKG_CONFIG_PATH but fall back to the FMI side-by-side layout.
PKG_CONFIG ?= pkg-config
# FMI uses side-by-side installs of gdal/geos/proj under /usr/<name><ver>/.
# These .pc files may live in either lib/pkgconfig or lib64/pkgconfig depending
# on the version. Add both for each known location. (No line continuations
# here — Make would insert literal spaces, which breaks pkg-config's parser.)
__SBS_PCDIRS := /usr/gdal312/lib/pkgconfig /usr/gdal312/lib64/pkgconfig /usr/gdal310/lib/pkgconfig /usr/gdal310/lib64/pkgconfig /usr/geos313/lib/pkgconfig /usr/geos313/lib64/pkgconfig /usr/geos312/lib/pkgconfig /usr/geos312/lib64/pkgconfig /usr/proj97/lib/pkgconfig /usr/proj97/lib64/pkgconfig /usr/proj95/lib/pkgconfig /usr/proj95/lib64/pkgconfig
empty :=
space := $(empty) $(empty)
PKG_CONFIG_PATH := $(PKG_CONFIG_PATH):$(subst $(space),:,$(__SBS_PCDIRS))
export PKG_CONFIG_PATH

PKG_MODULES := gdal fmt geos proj icu-i18n

CXX      ?= g++
CXX_STD  ?= c++17
OPTIMIZE ?= -O2

DEFINES  := -DUNIX -D_REENTRANT -DBOOST -DPQXX_HIDE_EXP_OPTIONAL \
            -DUSE_UNSTABLE_GEOS_CPP_API
WARNINGS := -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations
FLAGS    := -std=$(CXX_STD) -fPIC -fno-omit-frame-pointer -ggdb3 $(OPTIMIZE) \
            -DNDEBUG $(WARNINGS)

PKG_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(PKG_MODULES))
PKG_LIBS   := $(shell $(PKG_CONFIG) --libs   $(PKG_MODULES))

# -- Install paths -----------------------------------------------------------
# $(libdir) is also where we look up the installed smartmet static archives,
# so on distros that use /usr/lib instead of /usr/lib64 (e.g. ArchLinux,
# Debian) override with `make libdir=/usr/lib`.
PREFIX          ?= /usr
libdir          ?= $(PREFIX)/lib64
includedir      ?= $(PREFIX)/include
GDAL_PLUGIN_DIR ?= $(libdir)/gdalplugins

# Smartmet headers are all under one root: $(includedir)/smartmet/{newbase,
# macgyver,gis}/. The driver sources include them as <newbase/X.h> etc.
INCLUDES := -Iqd_driver -I$(includedir)/smartmet $(PKG_CFLAGS)

# Smartmet static archives. Order matters for ld: newbase pulls gis pulls
# macgyver (matches the NEEDED chain in the shared-library builds).
SMARTMET_STATIC_LIBS := \
    $(libdir)/libsmartmet-newbase.a  \
    $(libdir)/libsmartmet-gis.a      \
    $(libdir)/libsmartmet-macgyver.a

# Boost components needed by the union of the three libraries. boost_system
# is header-only since boost 1.69 (the stub library was dropped entirely in
# 1.91 on both Fedora and Arch), so it is not listed here.
BOOST_LIBS := -lboost_regex -lboost_serialization -lboost_chrono \
              -lboost_iostreams -lboost_thread

# sqlite3 and curl show up as NEEDED on the shared builds of gis/macgyver;
# include them defensively so any pulled-in static object that references
# them still resolves. --as-needed below drops them again if nothing did.
SYSTEM_LIBS := $(PKG_LIBS) $(BOOST_LIBS) -ldouble-conversion \
               -lsqlite3 -lcurl -lpthread -lrt

# -- Sources -----------------------------------------------------------------
DRIVER_SRCS := $(wildcard $(SUBNAME)/*.cpp)
DRIVER_OBJS := $(patsubst %.cpp,obj/%.o,$(notdir $(DRIVER_SRCS)))

# -- Top-level targets -------------------------------------------------------
.PHONY: all debug release clean format install test rpm

all: $(PLUGIN)

debug release: all

# -- Build rules -------------------------------------------------------------
$(PLUGIN): $(DRIVER_OBJS) $(SMARTMET_STATIC_LIBS)
	$(CXX) $(FLAGS) -shared -rdynamic -o $@ $(DRIVER_OBJS) \
	  $(SMARTMET_STATIC_LIBS) -Wl,--as-needed $(SYSTEM_LIBS) -Wl,--no-as-needed
	@echo "Checking $@ for unresolved references"
	@if ldd -r $@ 2>&1 | c++filt | grep ^undefined\ symbol | \
	   grep -Pv ':\ __(?:(?:a|t|ub)san_|sanitizer_)'; \
	then rm -v $@; exit 1; fi

obj/%.o: $(SUBNAME)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $(DEFINES) $(INCLUDES) -c -MD -MF $(@:.o=.d) -o $@ $<

# -- Auxiliary targets -------------------------------------------------------
clean:
	rm -f $(PLUGIN)
	rm -rf obj
	rm -rf test/output

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp

install: $(PLUGIN)
	mkdir -p $(DESTDIR)$(GDAL_PLUGIN_DIR)
	install -p -m 755 $(PLUGIN) $(DESTDIR)$(GDAL_PLUGIN_DIR)/$(PLUGIN)

test: $(PLUGIN)
	bash test/smoke.sh

# -- RPM target --------------------------------------------------------------
rpm: clean $(SPEC).spec
	rm -f $(SPEC).tar.gz
	tar -czf $(SPEC).tar.gz \
	    --transform "s,^,$(SPEC)/," \
	    --exclude-vcs \
	    --exclude="obj" --exclude="plugins" --exclude="*.so" \
	    *
	rpmbuild -tb $(SPEC).tar.gz $(RPMBUILD_OPT)
	rm -f $(SPEC).tar.gz

# -- Auto-generated dependency files -----------------------------------------
-include $(DRIVER_OBJS:.o=.d)
