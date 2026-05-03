REQUIRES = libpqxx

include $(shell smartbuildcfg --prefix)/share/smartmet/devel/makefile.inc

LIBS = -L../.. -lsmartmet-macgyver -lpqxx -lboost_chrono -lboost_thread -lpthread

all: torture-test

clean:
	rm -f torture-test

torture-test: torture-test.cpp
	$(CXX) -I ../../macgyver $(CFLAGS) -o $@ $< $(INCLUDES) $(LIBS)
