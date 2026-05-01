SUBNAME = qd_driver
DRIVER  = querydata
PLUGIN  = gdal_$(DRIVER).so
SPEC    = smartmet-gdal-querydata-driver

REQUIRES = gdal fmt

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

RPMBUILD_OPT ?=

DEFINES = -DUNIX -D_REENTRANT

# Headers from newbase live under /usr/include/smartmet/newbase, and
# the GDAL plugin search path is the same dir layout as for normal libs.
GDAL_PLUGIN_DIR ?= $(libdir)/gdalplugins

INCLUDES += -I$(includedir) -I$(includedir)/smartmet

LIBS += \
	$(PREFIX_LDFLAGS) \
	-lsmartmet-newbase \
	-lsmartmet-macgyver \
	-lsmartmet-gis \
	$(REQUIRED_LIBS) \
	$(PREFIX_LDFLAGS)

SRCS = $(wildcard $(SUBNAME)/*.cpp)
HDRS = $(wildcard $(SUBNAME)/*.h)
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

vpath %.cpp $(SUBNAME)
vpath %.h $(SUBNAME)

.PHONY: all debug release clean format install test rpm objdir

all: objdir $(PLUGIN)
debug: all
release: all

$(PLUGIN): $(OBJS)
	$(CXX) $(CFLAGS) -shared -rdynamic -o $(PLUGIN) $(OBJS) $(LIBS)
	@echo Checking $(PLUGIN) for unresolved references
	@if ldd -r $(PLUGIN) 2>&1 | c++filt | grep ^undefined\ symbol |\
			grep -Pv ':\ __(?:(?:a|t|ub)san_|sanitizer_)'; \
	then \
		rm -v $(PLUGIN); \
		exit 1; \
	fi

clean:
	rm -f $(PLUGIN) *~ $(SUBNAME)/*~
	rm -rf $(objdir)
	rm -rf test/output

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp

install: $(PLUGIN)
	@mkdir -p $(GDAL_PLUGIN_DIR)
	$(INSTALL_PROG) $(PLUGIN) $(GDAL_PLUGIN_DIR)/$(PLUGIN)

test: $(PLUGIN)
	$(MAKE) -C test test

rpm: clean $(SPEC).spec
	rm -f $(SPEC).tar.gz
	tar -czvf $(SPEC).tar.gz --exclude-vcs --transform "s,^,$(SPEC)/," *
	rpmbuild -tb $(SPEC).tar.gz $(RPMBUILD_OPT)
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp

objdir:
	@mkdir -p $(objdir)

obj/%.o : %.cpp
	@mkdir -p $(objdir)
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d, $@) -MT $@ -o $@ $<

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
