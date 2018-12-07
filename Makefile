all: main list tree tree_handler
	gcc -o prog main.o list.o tree.o tree_handler.o

main: main.c
	gcc -c main.c

list: list.c
	gcc -c list.c

tree: tree.c
	gcc -c tree.c

tree_handler: tree_handler.c
	gcc -c tree_handler.c

clear:
	rm *.o
