file_system: file_system.c file_system.h common.h
	make List.o
	make node.o
	make file_system.o
	gcc -g -o file_system List.o node.o file_system.o -ggdb
	make clean
clean:
	rm -rf *.o
	rm -rf *.gch
List.o: List.c List.h
	gcc -c -g List.c -ggdb
node.o: node.c node.h
	gcc -c -g node.c -ggdb
file_system.o: file_system.c file_system.h
	gcc -c -g file_system.c -ggdb