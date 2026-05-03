#! /bin/sh

supported_requires="cairo configpp ctpp2 fmt gdal icu-i18n jsoncpp libpqxx librsvg mysql xerces-c"

for req in $supported_requires; do
    echo "--- $req"
    echo "REQUIRES := $req" >test.mk
    echo "include ../makefile.inc" >>test.mk
    echo "" >>test.mk
    echo "test:" >>test.mk
    echo "	@echo REQUIRES_LEFT=\$(REQUIRES_LEFT)" >>test.mk
    echo "	@echo INCLUDES=\$(INCLUDES)" >>test.mk
    echo "	@echo REQUIRED_LIBS=\$(REQUIRED_LIBS)" >>test.mk
    make -f test.mk test
    rm test.mk
done
