c++ = g++

object_directory = objects

objects = $(object_directory)/document/document.o $(object_directory)/load/load.o $(object_directory)/save/save.o $(object_directory)/cxml.o

all: static shared test-rebuild
static: objects
	ar rcs lib/libCXML.a.1.0 objects/document/document.o objects/load/load.o objects/save/save.o objects/cxml.o
	ln -fsv ./libCXML.a.1.0 lib/libCXML.a
shared: static
	gcc -shared -Wl,-soname,libCXML.so.1.0 objects/document/document.o objects/load/load.o -o lib/libCXML.so.1.0
	ln -fsv ./libCXML.so.1.0 lib/libCXML.so
objects: dirs $(objects) dirs cxml.c
$(object_directory)/cxml.o: cxml.c
	gcc -std=c99 -Iinclude cxml.c -fpic -g -c -o objects/cxml.o
$(object_directory)/document/document.o: document/document.c
	gcc -std=c99 -Iinclude document/document.c -fpic -g -c -o $@
$(object_directory)/load/load.o: load/load.c
	gcc -std=c99 -Iinclude load/load.c -fpic -g -c -o $@
$(object_directory)/save/save.o: save/save.c
	gcc -std=c99 -Iinclude save/save.c -fpic -g -c -o $@
test: objects static
	gcc -std=c99 -Iinclude -g test/test.c -o bin/test -Llib/ -static -lCXML
clean: clean-all
clean-all: clean-objects clean-static clean-shared
clean-objects:
	find objects -name "*.o" -exec rm -v {} \;
	rm -rv objects
clean-static:
	find lib -name "*.a*" -exec rm -v {} \;
clean-shared:
	find lib -name "*.so*" -exec rm -v {} \;
test-rebuild:
	make -B objects test
dirs:
	mkdir -p $(object_directory)
	mkdir -p lib
	mkdir -p bin
	mkdir -p $(object_directory)/document
	mkdir -p $(object_directory)/load
	mkdir -p $(object_directory)/save
.PHONY: static shared objects test-rebuild clean all dirs
