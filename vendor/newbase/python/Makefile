REQUIRES =

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

LIBFILE = newbase.so

PYTHON ?= python3

PYTHON_SITE_ARCH=$(shell $(PYTHON) -Ic "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")
PYTHON_INCLUDE_DIR=$(shell python3 -c "from sysconfig import get_paths as gp; print(gp()['include'])")

INCLUDES += -I../newbase
INCLUDES += -I$(PYTHON_INCLUDE_DIR)

# Silence warbubfs related to missing field initializers and cast function type (otherwise it is too noisy)
CFLAGS += -Wno-missing-field-initializers -Wno-cast-function-type

LIBS += \
	-L.. -lsmartmet-newbase \
	-lsmartmet-macgyver \
	-lboost_iostreams \
	-lboost_thread \
	$(REQUIRED_LIBS) \
	-lpthread -lrt

all: newbase.so load-test

# Simple load test for verifying that the module can be loaded
load-test: newbase.so
	@LD_LIBRARY_PATH=.. $(PYTHON) -c "import newbase; print(newbase.__file__)" >/dev/null

test:
	LD_LIBRARY_PATH=.. $(PYTHON) test_newbase.py

install:
	# NOTE: installs for current Python3 version only
	install -D -m 644 newbase.so $(datadir)/smartmet/python/newbase.so
	install -d $(prefix)/..$(PYTHON_SITE_ARCH)
	ln -srf $(datadir)/smartmet/python/newbase.so $(prefix)/..$(PYTHON_SITE_ARCH)/newbase.so

clean:
	rm -f newbase-binding.cpp *.o *.d $(LIBFILE)

$(LIBFILE): newbase-binding.o ../libsmartmet-newbase.so
	$(CXX) -shared -Wl,--retain-symbols-file,newbase-binding.sym -o $@ newbase-binding.o $(LIBS)
	@echo "Checking $(LIBFILE) for unresolved references (newbase ones only)"
	@if LD_LIBRARY_PATH=.. ldd -r $(LIBFILE) 2>&1 | c++filt | grep ^undefined\ symbol |\
			grep -P ':\ NFmi'; \
	then \
		rm -v $(LIBFILE); \
		exit 1; \
	fi

newbase-binding.cpp: newbase-binding.py ../newbase/NFmiGlobals.h ../newbase/NFmiLevelType.h ../newbase/NFmiDataIdent.h ../newbase/NFmiParameterName.h
	$(PYTHON) newbase-binding.py > $@
	@if test $$(wc -l $@ | awk '{print $$1}') -lt 100; then \
		echo "Generated binding file is too short"; \
		exit 1; \
	fi

%.o : %.cpp
	@mkdir -p $(objdir)
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst %.o, %.d, $@) -MT $@ -o $@ $<

-include $(wildcard *.d)
