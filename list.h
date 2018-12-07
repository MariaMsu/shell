#ifndef PRAK_LIST_H
#define PRAK_LIST_H

typedef char **string_list;

struct iterator{
    string_list mas;
    int current_string;
    int mas_size;
    char * (*next_string)(struct iterator*);
};

struct iterator* iterator_constructor(string_list mas);

void del_list(string_list mas);

void print_list(string_list mas);

void sort_list(string_list mas);

void append_string(string_list *mas, char *word);

extern int check_brace(string_list mas);

#endif //PRAK_LIST_H


