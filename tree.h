#ifndef PRAK_TREE_H
#define PRAK_TREE_H

#include "list.h"

enum type_of_next{
    NXT, AND, OR   // виды связей соседних команд в списке команд
};


struct cmd_inf {
    int argc;
    char **argv; // список из имени команды и аргументов
    char *infile; // переназначенный файл стандартного ввода
    char *outfile; // переназначенный файл стандартного вывода
    int append;
    int backgrnd; // =1, если команда подлежит выполнению в фоновом режиме
    struct cmd_inf *psubcmd; // команды для запуска в дочернем shell
    struct cmd_inf *pipe; // следующая команда после "|"
    struct cmd_inf *next; // следующая после ";" (или после "&")
    enum type_of_next type;// связь со следующей командой через ; или && или ||
};

struct cmd_inf* list_to_tree(string_list list);
struct cmd_inf *cmd_inf_constructor();
void print_tree(struct cmd_inf* tree);
void del_tree(struct cmd_inf *tree);
#endif //PRAK_TREE_H
