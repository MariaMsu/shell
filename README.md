# Task5
hell с конвейером и сабшеллом  

## правильная работа:

/home/maria/Desktop/task5❥ yes "kafedra_A_YA" | head  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  
kafedra_A_YA  

/home/maria/Desktop/task5❥ ls | (cat -n)  
     1	f.txt  
     2	list.c  
     3	list.h  
     4	main.c  
     5	Makefile  
     6	prog  
     7	README.md  
     8	t  
     9	tree.c  
    10	tree.h  
    11	tree_handler.c  
    12	tree_handler.h  

/home/maria/Desktop/task5❥ ls && pwd  
f.txt	list.h	Makefile  README.md  tree.c  tree_handler.c  
list.c	main.c	prog	  t	     tree.h  tree_handler.h  
/home/maria/Desktop/task5  


## неправильная работа:

ошибочный синтаксис:  
/home/maria/Desktop/task5❥ echo ~  
Incorrect input :(  
Please check your sequence of symbols and try again  

ошибочные параметы:  
/home/maria/Desktop/task5❥ ls -z  
ls: invalid option -- 'z'  
Try 'ls --help' for more information.  

несуществующая команда:  
/home/maria/Desktop/task5❥ kafedra_A_YA  
error in line 49, file tree_handler.c  

## структуры:
main.c - выводит приглашение ко вводу, запускает шелл, считывает строку, передает ее в обработчики  
list.c - созраняет строку в список (не заню, зачем. совершенно бесполезный модуль, имхо). А еще! А еще в этом модуле есть функция сортировки списка! Вот  
tree.c - строит из списка древовидную структуру, содержащую распарсенные команды, аргументы, метаинформацию  
tree_handler.c - выполняет команды шелла, распарсенные в дерево  

## описание графов, которых нет
символы в слова очень органично собираются с момощью switch-case (main.c), слова в древовидную структуру не менее органично собираются с помощью серии if'ов, стилизованной под switch-case (tree.c)

## ps
на команду: echo "  
шелл выдает два приглашения ко вводу. Если почините - дайте знать  

У вас может не быть библиотеки zconf.h. Или скачайте, или погуглите замену. Она чем-то обыденным прекрасно заменяется
