clean: 
	rm -rf *.o 
	rm -rf *.gch
	rm -rf kais_shell
node.o: node.h
	gcc -c node.c
List.o: node.h List.h
	gcc -c List.c
List_Extras.o: node.h List_Extras.h
	gcc -c List_Extras.c
Process_Props.o: common.h Process_Props.h
	gcc -c Process_Props.c
commands.o: List.h Process_Props.h node.h
	gcc -c commands.c
built_in_commands.o: built_in_commands.c 
	gcc -c built_in_commands.c 
sighandlers.o: sighandlers.h
	gcc -c sighandlers.c
string_extras.o: string_extras.h
	gcc -c string_extras.c 
kais_shell.o: common.h Process_Props.h List.h commands.h sighandlers.h string_extras.h
	gcc -c kais_shell.c
kais_shell: Process_Props.o List.o commands.o node.o
	rm -rf *.o 
	rm -rf *.gch
	rm -rf kais_shell
	make node.o
	make List.o
	make Process_Props.o
	make kais_shell.o
	make sighandlers.o
	make List_Extras.o
	make string_extras.o	
	make built_in_commands.o
	gcc -o kais_shell built_in_commands.o Process_Props.o node.o List.o List_Extras.o sighandlers.o kais_shell.o string_extras.o -lreadline -lpthread 
	rm -rf *.o 
	rm -rf *.gch

