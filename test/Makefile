PROG = $(patsubst %.cpp,%,$(wildcard *Test.cpp))

REQUIRES = gdal

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

FLAGS = -std=$(CXX_STD) -Wall -W -Wno-unused-parameter

ifeq ($(TSAN), yes)
  FLAGS += -fsanitize=thread
endif
ifeq ($(ASAN), yes)
  FLAGS += -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -fsanitize-address-use-after-scope
endif

CFLAGS = -DUNIX -O0 -g $(FLAGS)

INCLUDES += \
	-I../newbase \
	-I/usr/include/smartmet \
	-I.

LIBS +=  \
     ../libsmartmet-newbase.so \
	$(PREFIX_LDFLAGS) \
	-lsmartmet-macgyver \
	-lboost_iostreams \
	-lboost_thread \
	$(REQUIRED_LIBS) \
	-lz -lbz2 \
	-lpthread -lrt

all: $(PROG)
clean:
	rm -f $(PROG) *~

test: $(PROG)
	@echo Running tests:
	@rm -f *.err
	@for prog in $(PROG); do \
	  ( ./$$prog || touch $$prog.err ) ; \
	done
	@test `find . -name \*.err | wc -l` = "0" || ( echo ; echo "The following tests have errors:" ; \
		for i in *.err ; do echo `basename $$i .err`; done ; rm -f *.err ; false )

$(PROG) : % : %.cpp ../libsmartmet-newbase.so
	$(CXX) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)


ifeq ($(USE_CLANG), yes)
NFmiStringToolsTest: CFLAGS += -Wno-invalid-source-encoding
endif
