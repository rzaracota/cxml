all: static shared test-rebuild
static: objects
	ar rcs lib/libCXML.a.0.1 objects/document/document.o objects/load/load.o objects/save/save.o objects/cxml.o
	ln -fsv ./libCXML.a.0.1 lib/libCXML.a
shared: static
	gcc -shared -Wl,-soname,libCXML.so.0.1 objects/document/document.o objects/load/load.o -o lib/libCXML.so.0.1
	ln -fsv ./libCXML.so.0.1 lib/libCXML.so
objects: document load save cxml.c
	gcc -std=c99 -Iinclude cxml.c -g -c -o objects/cxml.o
document: document/document.c
	gcc -std=c99 -Iinclude document/document.c -fpic -g -c -o objects/document/document.o
load: load/load.c
	gcc -std=c99 -Iinclude load/load.c -fpic -g -c -o objects/load/load.o
save: save/save.c
	gcc -std=c99 -Iinclude save/save.c -fpic -g -c -o objects/save/save.o
test: objects static
	gcc -std=c99 -Iinclude -g test/test.c -o bin/test -Llib/ -static -lCXML
clean-all: clean-objects clean-static clean-shared
clean-objects:
	find objects -name "*.o" -exec rm -v {} \;
clean-static:
	find lib -name "*.a*" -exec rm -v {} \;
clean-shared:
	find lib -name "*.so*" -exec rm -v {} \;
test-rebuild:
	make -B objects test
