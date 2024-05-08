kais_shell: Process_Props.o List.o commands.o node.o
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
	make parser.o
	make formatDISK.o
	gcc -o kais_shell built_in_commands.o parser.o Process_Props.o node.o List.o List_Extras.o sighandlers.o kais_shell.o string_extras.o file_system.o formatDISK.o -ggdb -lreadline -lpthread -lm 
	rm -rf *.gch
clean: 
	rm -rf *.o 
	rm -rf *.gch
	rm -rf kais_shell
formatDISK.o: formatDISK.c
	gcc -c -g formatDISK.c
node.o: node.h
	gcc -c -g node.c
List.o: node.h List.h
	gcc -c -g List.c
List_Extras.o: node.h List_Extras.h
	gcc -c -g List_Extras.c
Process_Props.o: common.h Process_Props.h
	gcc -c -g Process_Props.c
commands.o: List.h Process_Props.h node.h
	gcc -c -g commands.c
built_in_commands.o: built_in_commands.c 
	gcc -c -g built_in_commands.c 
sighandlers.o: sighandlers.h
	gcc -c -g sighandlers.c
string_extras.o: string_extras.h
	gcc -c -g string_extras.c 
parser.o: parser.h
	gcc -c -g parser.c
kais_shell.o: common.h Process_Props.h List.h commands.h sighandlers.h string_extras.h
	gcc -c -g kais_shell.c

