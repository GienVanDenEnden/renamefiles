all: renamefiles

renamefiles: renamefiles.o
	gcc -o renamefiles renamefiles.o
   
renamefiles.o: renamefiles.c
	gcc -c renamefiles.c 

clean: 
	rm renamefiles.o renamefiles

install:
	cp ./renamefiles /usr/bin/renamefiles
	cp ./renamefiles.1 /usr/share/man/man1/renamefiles.1

uninstall:
	rm /usr/bin/renamefiles
	rm /usr/share/man/man1/renamefiles.1
	