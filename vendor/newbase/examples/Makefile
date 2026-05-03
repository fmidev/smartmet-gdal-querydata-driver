# The files to be compiled

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
PROGS = $(SRCS:%.cpp=%)

-include $(HOME)/.smartmet.mk
GCC_DIAG_COLOR ?= always
CXX_STD ?= c++11

# The rules

all: $(PROGS)

$(PROGS): % : %.cpp
	g++ -std=$(CXX_STD) -DUNIX -Wall -g -O0 -o $@ $@.cpp -I ../include -L .. -lnewbase

clean:
	rm -f $(PROGS) $(OBJS) *~

