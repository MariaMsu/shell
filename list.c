#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"


//return size of array
static int size_of_mas(string_list mas) {
    int size = 0;
    while (*(mas++) != NULL)
        size++;
    return size;
}

static char *get_next_string(struct iterator *iter) {
    int number = iter->current_string++;
    if (number >= iter->mas_size) {
        return NULL;
    }
    return iter->mas[number];
}

extern struct iterator *iterator_constructor(string_list mas) {
    struct iterator *new_iter = malloc(sizeof(struct iterator));
    new_iter->mas = mas;
    new_iter->mas_size = size_of_mas(mas);
    new_iter->current_string = 0;
    new_iter->next_string = get_next_string;
    return new_iter;
}

//swap
static void swap(string_list first, string_list second) {
    void *temporary = *first;
    *first = *second;
    *second = temporary;

}

// append string to list
extern void append_string(string_list *mas, char *word) {
    if (*mas == NULL) {
        *mas = malloc(2 * sizeof(char *));
        (*mas)[0] = word;
        (*mas)[1] = NULL;
        return;
    }
    int length = size_of_mas(*mas);
    *mas = realloc(*mas, (length + 2) * sizeof(char *));
    (*mas)[length] = word;
    (*mas)[length + 1] = NULL;
}

// sorts list
extern void sort_list(string_list mas) {
    if (mas == 0)
        return;
    int length = size_of_mas(mas);
    for (int i = 1; i < length; i++)
        for (int j = 0; j < length - i; j++)
            if (strcmp(mas[j], mas[j + 1]) > 0)
                swap(&mas[j], &mas[j + 1]);
}

//prints list
extern void print_list(string_list mas) {
    if (mas == 0) {
        printf("list is empty\n");
        return;
    }
    int length = size_of_mas(mas);
    printf("length of list is %d\n", length);
    for (int i = 0; i < length; i++) {
        printf("%s\n", mas[i]);
    }
}

//deletes list, free memory
extern void del_list(string_list mas) {
    if (mas == 0)
        return;
    int length = size_of_mas(mas);
    for (int i = 0; i <= length; i++)
        free((mas)[i]);
    free(mas);
}

// 0 for right sequence
extern int check_brace(string_list mas){
    int brace = 0;
    for (int i = 0; mas[i]!=NULL; ++i){
        if (strcmp(mas[i], "(")==0)
            ++brace;
        else if(strcmp(mas[i], ")") == 0)
            if(--brace < 0)
                return 1;
    }
    return brace;
}