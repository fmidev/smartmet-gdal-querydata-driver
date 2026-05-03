PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))

include $(shell smartbuildcfg --prefix)/share/smartmet/devel/makefile.inc

INCLUDES += -I../macgyver \
        -I/usr/include/smartmet

LIBS += -L.. -lsmartmet-macgyver \
	-lboost_regex \
        -lboost_filesystem \
        -lboost_iostreams \
	-lboost_thread \
	-lpthread

all: $(PROG) examples
clean:
	rm -f $(PROG) *~

examples: $(PROG)
	@for prog in $(PROG); do \
	echo; \
	echo Running executable: ./$$prog; \
	./$$prog; \
	done

$(PROG) : % : %.cpp ../libsmartmet-macgyver.so
	$(CXX) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)
