REQUIRES := mysql

include ../../makefile.inc

all test test-installed release debug profile rpm:
	@echo MAKECMDGOALS=$(MAKECMDGOALS)
	@echo INCLUDES=$(INCLUDES)
	@echo MYSQL_LIBS=$(MYSQL_LIBS)
