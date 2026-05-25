# =============================================================================
# Self-contained build of the QUERYDATA GDAL driver.
#
# The driver depends on three smartmet libraries (newbase, macgyver, gis).
# To keep desktop deployment simple — `git clone --recurse-submodules && make
# && sudo make install` on a stock Fedora — those three libraries are vendored
# in `vendor/` as git submodules, compiled into a single static archive, and
# statically linked into the driver. The resulting `gdal_querydata.so` has only
# Fedora-stock packages (gdal-libs, fmt, geos, proj, etc.) as runtime
# dependencies — no smartmet RPMs.
#
# Targets:
#   make              build gdal_querydata.so
#   make install      install plugin into $(GDAL_PLUGIN_DIR)
#   make rpm          build a self-contained source RPM (vendored sources go in)
#   make vendor-init  initialise the vendored submodules if not already present
#   make vendor-pull  advance vendored submodules to the configured upstream ref
#   make clean
# =============================================================================

SUBNAME := qd_driver
DRIVER  := querydata
PLUGIN  := gdal_$(DRIVER).so
SPEC    := smartmet-gdal-querydata-driver

# -- Vendored submodule configuration ----------------------------------------
# The exact commit each submodule is pinned to lives in the git index (see
# `git submodule status`); the tracking branch is in .gitmodules. To pin a
# different ref for a release build, check out that ref inside the submodule
# directory and commit the gitlink update in this repo.
#
# The .spec file from each submodule doubles as a sentinel (existence check)
# and as the upstream version source for vendor-check-version.
NEWBASE_SPEC  := vendor/newbase/smartmet-library-newbase.spec
MACGYVER_SPEC := vendor/macgyver/smartmet-library-macgyver.spec
GIS_SPEC      := vendor/gis/smartmet-library-gis.spec
VENDOR_SPECS  := $(NEWBASE_SPEC) $(MACGYVER_SPEC) $(GIS_SPEC)

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
# sqlite3 and libpqxx are deliberately omitted: the only vendored translation
# units that referenced them (EPSGInfo, PostgreSQLConnection*) are excluded
# from VENDOR_SRCS below. Adding them back here would be a sign that something
# unexpected is now being pulled in.

CXX      ?= g++
CXX_STD  ?= c++17
OPTIMIZE ?= -O2

DEFINES := -DUNIX -D_REENTRANT -DBOOST -DPQXX_HIDE_EXP_OPTIONAL \
           -DUSE_UNSTABLE_GEOS_CPP_API \
           -DUSE_OS_TZDB=1 -DAUTO_DOWNLOAD=0 -DHAS_REMOTE_API=0
# Without the tz-related defines: macgyver's vendored copy of Howard Hinnant's
# date/tz library tries to fetch the IANA timezone database from ftp.iana.org
# during static initialisation — which deadlocks dlopen() of the plugin in any
# environment that doesn't have outbound HTTP. USE_OS_TZDB=1 makes it use the
# system's /usr/share/zoneinfo instead (tzdata package on Fedora).
WARNINGS := -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations
FLAGS    := -std=$(CXX_STD) -fPIC -fno-omit-frame-pointer -ggdb3 $(OPTIMIZE) \
            -DNDEBUG $(WARNINGS)

PKG_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(PKG_MODULES))
PKG_LIBS   := $(shell $(PKG_CONFIG) --libs   $(PKG_MODULES))

# Vendored headers live in vendor/<name>/<name>/. Sources reference siblings
# via either bare "X.h" or <X.h>; both work as long as the source directory is
# on the include path.
INCLUDES := -Iqd_driver \
            -Ivendor/newbase  -Ivendor/newbase/newbase \
            -Ivendor/macgyver -Ivendor/macgyver/macgyver \
            -Ivendor/gis      -Ivendor/gis/gis \
            $(PKG_CFLAGS)

# Boost components needed by the union of the three libraries.
BOOST_LIBS := -lboost_regex -lboost_serialization -lboost_chrono \
              -lboost_iostreams -lboost_thread

SYSTEM_LIBS := $(PKG_LIBS) $(BOOST_LIBS) -ldouble-conversion -lpthread -lrt

# -- Sources -----------------------------------------------------------------
DRIVER_SRCS := $(wildcard $(SUBNAME)/*.cpp)
DRIVER_OBJS := $(patsubst %.cpp,obj/%.o,$(notdir $(DRIVER_SRCS)))

# Vendored sources: every .cpp in each library's source dir.
# Skip TemplateFormatter.cpp — it pulls in ctpp2, which our driver doesn't
# need and which isn't packaged on every distro.
VENDOR_SRCS := \
  $(wildcard vendor/newbase/newbase/*.cpp) \
  $(filter-out \
      vendor/macgyver/macgyver/TemplateFormatter.cpp \
      vendor/macgyver/macgyver/PostgreSQLConnection.cpp \
      vendor/macgyver/macgyver/PostgreSQLConnectionImpl.cpp, \
      $(wildcard vendor/macgyver/macgyver/*.cpp)) \
  $(wildcard vendor/macgyver/macgyver/date_time/*.cpp) \
  $(wildcard vendor/macgyver/macgyver/date_time/date/*.cpp) \
  $(filter-out \
      vendor/gis/gis/EPSGInfo.cpp, \
      $(wildcard vendor/gis/gis/*.cpp))
# Excluded vendored files and the libraries they would have dragged in:
#   TemplateFormatter.cpp           — libctpp2 (template engine, unused)
#   PostgreSQLConnection*.cpp       — libpqxx (DB pooling, unused)
#   EPSGInfo.cpp                    — libsqlite3 (proj's EPSG db lookup, unused)
VENDOR_OBJS := $(patsubst %.cpp,obj/vendor/%.o,$(VENDOR_SRCS))
VENDOR_LIB  := obj/libvendored_smartmet.a

# -- Install paths -----------------------------------------------------------
PREFIX          ?= /usr
libdir          ?= $(PREFIX)/lib64
GDAL_PLUGIN_DIR ?= $(libdir)/gdalplugins

# -- Top-level targets -------------------------------------------------------
.PHONY: all debug release clean format install test rpm objdir \
        vendor-init vendor-pull vendor-check-version

all: $(VENDOR_SPECS) $(PLUGIN)

debug release: all

# -- Submodule management ----------------------------------------------------
# The .spec from each submodule is the sentinel: if it's missing, initialise
# the submodule. Inside an RPM build chroot the spec is already present in the
# unpacked tarball (the tarball ships the submodule working trees, not the
# .git pointer), so these rules don't fire.

$(NEWBASE_SPEC):
	@echo "==> vendor: initialising newbase submodule"
	git submodule update --init vendor/newbase

$(MACGYVER_SPEC):
	@echo "==> vendor: initialising macgyver submodule"
	git submodule update --init vendor/macgyver

$(GIS_SPEC):
	@echo "==> vendor: initialising gis submodule"
	git submodule update --init vendor/gis

vendor-init: $(VENDOR_SPECS)

# Advance each submodule to the tip of its configured tracking branch
# (set per submodule in .gitmodules via `branch =`). After this, the gitlink
# in the index will differ from HEAD until you `git add vendor/<name>` and
# commit. To pin to a tag instead, check out the tag inside the submodule
# directory manually and commit the gitlink update.
vendor-pull: vendor-init
	git submodule update --remote --merge vendor/newbase vendor/macgyver vendor/gis

# Cross-check that the version recorded in each spec matches the version-like
# tracking branch configured in .gitmodules. Branch names that aren't version
# tags (e.g. master, develop) are skipped.
vendor-check-version: $(VENDOR_SPECS)
	@for pair in \
	    "$(NEWBASE_SPEC)|vendor/newbase|newbase" \
	    "$(MACGYVER_SPEC)|vendor/macgyver|macgyver" \
	    "$(GIS_SPEC)|vendor/gis|gis"; do \
	  spec=$${pair%%|*}; rest=$${pair#*|}; path=$${rest%%|*}; name=$${rest##*|}; \
	  ref=$$(git config -f .gitmodules submodule.$$path.branch 2>/dev/null); \
	  case "$$ref" in \
	    [0-9]*) \
	      ver=$$(grep -E '^Version:' "$$spec" | head -1 | awk '{print $$2}'); \
	      if [ "$$ver" != "$$ref" ]; then \
	        echo "ERROR: $$name pinned at $$ref but vendored copy is $$ver"; \
	        echo "       run 'make vendor-pull' to refresh"; \
	        exit 1; \
	      fi;; \
	    *) echo "==> $$name on branch ref '$$ref' (skipping version check)";; \
	  esac; \
	done

# -- Build rules -------------------------------------------------------------
$(PLUGIN): $(DRIVER_OBJS) $(VENDOR_LIB)
	$(CXX) $(FLAGS) -shared -rdynamic -o $@ $(DRIVER_OBJS) \
	  $(VENDOR_LIB) -Wl,--as-needed $(SYSTEM_LIBS) -Wl,--no-as-needed
	@echo "Checking $@ for unresolved references"
	@if ldd -r $@ 2>&1 | c++filt | grep ^undefined\ symbol | \
	   grep -Pv ':\ __(?:(?:a|t|ub)san_|sanitizer_)'; \
	then rm -v $@; exit 1; fi

# Vendored objects are linked as a normal static archive (no --whole-archive),
# so the linker only pulls in objects that resolve symbols actually used by the
# driver. Translation units that reference excluded symbols (e.g. anything
# transitively touching TemplateFormatter / ctpp2) are dropped silently — which
# is what we want.

$(VENDOR_LIB): $(VENDOR_OBJS)
	$(AR) crs $@ $^

obj/%.o: $(SUBNAME)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $(DEFINES) $(INCLUDES) -c -MD -MF $(@:.o=.d) -o $@ $<

obj/vendor/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $(DEFINES) $(INCLUDES) -c -MD -MF $(@:.o=.d) -o $@ $<

# NFmiEnumConverterInit has a huge static-init expression; -O2 makes it slow
# to compile. Mirror newbase's own Makefile workaround for gcc.
obj/vendor/vendor/newbase/newbase/NFmiEnumConverterInit.o: OPTIMIZE := -O0

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
# Build a source tarball that contains everything rpmbuild needs, including the
# vendored library sources, but excluding things that bloat the tarball without
# being used (Python bindings, test data, .git metadata, build artefacts, the
# vendored libraries' own Makefiles which we never recurse into).
rpm: vendor-check-version clean $(SPEC).spec
	rm -f $(SPEC).tar.gz
	tar -czf $(SPEC).tar.gz \
	    --transform "s,^,$(SPEC)/," \
	    --exclude-vcs \
	    --exclude="vendor/*/python" \
	    --exclude="vendor/*/test" \
	    --exclude="vendor/*/obj" \
	    --exclude="vendor/*/Makefile" \
	    --exclude="vendor/*/Doxyfile" \
	    --exclude="vendor/*/.circleci" \
	    --exclude="vendor/*/.github" \
	    --exclude="vendor/*/CLAUDE.md" \
	    --exclude="vendor/*/README.md" \
	    --exclude="vendor/*/.clang-format" \
	    --exclude="vendor/*/.gitignore" \
	    --exclude="obj" --exclude="plugins" --exclude="*.so" \
		--exclude="vendor/*/smartmet-library-*.spec" \
	    *
	rpmbuild -tb $(SPEC).tar.gz $(RPMBUILD_OPT)
	rm -f $(SPEC).tar.gz

# -- Auto-generated dependency files -----------------------------------------
-include $(DRIVER_OBJS:.o=.d)
-include $(VENDOR_OBJS:.o=.d)
