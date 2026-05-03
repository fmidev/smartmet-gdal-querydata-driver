REQUIRES := mariadb

include ../../makefile.inc

all debug profile rpm clean test test-installed:
	@echo MAKECMDGOALS=$(MAKECMDGOALS)
	@echo INCLUDES=$(INCLUDES)
	@echo MARIADB_LIBS_LIBS=$(MARIADB_LIBS)

