file_system.o: file_system.c file_system.h
	gcc -c file_system.c
file_system: file_system.c file_system.h common.h
	make List.o
	make node.o
	make file_system.o
	make -B fresh_disk
	gcc -g -o file_system List.o node.o file_system.o string_extras.o -ggdb
	make clean
clean:
	rm -rf *.o
	rm -rf *.gch
	rm -rf *.out
List.o: List.c List.h
	gcc -c -g List.c -ggdb
node.o: node.c node.h
	gcc -c -g node.c -ggdb
fresh_disk: format.c
	gcc -g format.c 
	./a.out
	rm -rf *.out