#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "tree.h"

jmp_buf begin;

void error(char *err, int line) {
    printf("error: %s\nin line %d in file %s\n", err, line, __FILE__);
    longjmp(begin, 1);
}

static void add_argv(struct cmd_inf *unit, char *string) {
    int size = ++unit->argc;
    unit->argv = realloc(unit->argv, (size + 1) * sizeof(char *));
    unit->argv[size - 1] = string;
    unit->argv[size] = NULL;
}

char is_name(const char *string) {
    for (int i = 0; i < strlen((string)); ++i) {
        switch (string[i]) {
            case '>':
            case '<':
            case '&':
            case '#':
            case '(':
            case ')':
            case '|':
            case ';':
                return 0;
            default:
                continue;

        }
    }
    return 1;
}

static struct cmd_inf *create_unit(struct iterator *iter) {
    struct cmd_inf *unit = cmd_inf_constructor();
    int unit_filled = 0;
    for (const char *lexeme = iter->next_string(iter); lexeme != NULL; lexeme = iter->next_string(iter)) {

        if (strcmp(lexeme, ">") == 0) {
            if (unit->outfile != NULL) {
                del_tree(unit);
                error("two input files", __LINE__);
            }
            if (!is_name(lexeme = iter->next_string(iter))) {
                del_tree(unit);
                error("parse error near `>'", __LINE__);
            }
            unit->outfile = (char *) lexeme;
            unit->append = 0;

        } else if (strcmp(lexeme, ">>") == 0) {
            if (unit->outfile != NULL) {
                del_tree(unit);
                error("two input files", __LINE__);
            }
            if (!is_name(lexeme = iter->next_string(iter))) {
                del_tree(unit);
                error("parse error near `>>'", __LINE__);
            }
            unit->outfile = (char *) lexeme;
            unit->append = 1;

        } else if (strcmp(lexeme, "<") == 0) {
            if (unit->infile != NULL) {
                del_tree(unit);
                error("two output files", __LINE__);
            }
            if (!is_name(lexeme = iter->next_string(iter))) {
                del_tree(unit);
                error("parse error near `<'", __LINE__);
            }
            unit->infile = (char *) lexeme;

        } else if (strcmp(lexeme, ";") == 0) {
            if (!unit_filled) {
                del_tree(unit);
                error("parse error near `;;'", __LINE__);
            }
            unit->type = NXT;
            unit->next = create_unit(iter);
            break;

        } else if (strcmp(lexeme, "&&") == 0) {
            unit->type = AND;
            unit->next = create_unit(iter);
            break;

        } else if (strcmp(lexeme, "||") == 0) {
            unit->type = OR;
            unit->next = create_unit(iter);
            break;

        } else if (strcmp(lexeme, "&") == 0) {
            unit->backgrnd = 1;
            unit->next = create_unit(iter);
            break;

        } else if (strcmp(lexeme, "|") == 0) {
            unit->pipe = create_unit(iter);
            break;

        } else if (strcmp(lexeme, "(") == 0) {
            if (unit->argc != 0) {
                del_tree(unit);
                error("unexpected arg before '('", __LINE__);
            }
            unit->psubcmd = create_unit(iter);

        } else if (strcmp(lexeme, ")") == 0) {
            if (!unit_filled) {
                del_tree(unit);
                return NULL;
            } else
                return unit;

        } else {
            char *string = malloc(sizeof(char) * (strlen(lexeme) + 1));
            strcpy(string, lexeme);
            add_argv(unit, string);
        }
        unit_filled = 1;
    }
    if (!unit_filled) {
        del_tree(unit);
        return NULL;
    } else
        return unit;
}

extern struct cmd_inf *list_to_tree(string_list list) {
    struct iterator *iter = iterator_constructor(list);
    struct cmd_inf *root;
    if (!setjmp(begin)) {
        root = create_unit(iter);
    } else {
        printf("sintaksis error, pleas try again\n");
        root = NULL;
    }
    free(iter);
    return root;
}

extern void del_tree(struct cmd_inf *tree) {
    if (tree == NULL)
        return;
    del_tree(tree->psubcmd);
    del_tree(tree->pipe);
    del_tree(tree->next);
    for (int i = 0; tree->argv[i] != NULL; free(tree->argv[i]), ++i);
    free(tree->argv);
    free(tree);
}

extern void print_tree(struct cmd_inf *tree) {
    if (tree == NULL)
        return;

    printf("element (%s): ", tree->argv[0]);
    printf("\n=============\n");
    printf("argv: ");
    for (int i = 0; tree->argv[i] != NULL; printf("%s, ", tree->argv[i]), ++i);
    printf("\n=============\n");
    printf("infile: %s", tree->infile);
    printf("\n=============\n");
    printf("outfile: %s", tree->outfile);
    printf("\n=============\n");
    printf("backgrnd: %d", tree->backgrnd);
    printf("\n=============\n");
    if (tree->psubcmd)
        printf("exist psubcmd (%s)\n", tree->psubcmd->argv[0]);
    if (tree->pipe)
        printf("exist pipe (%s)\n", tree->pipe->argv[0]);
    if (tree->next)
        printf("exist next (%s)\n", tree->next->argv[0]);
    printf("\n");

    print_tree(tree->psubcmd);
    print_tree(tree->pipe);
    print_tree(tree->next);
}

struct cmd_inf *cmd_inf_constructor() {
    struct cmd_inf *unit = malloc(sizeof(struct cmd_inf));
    unit->argc = 0;
    unit->argv = malloc(sizeof(char *));
    unit->argv[0] = NULL;
    unit->infile = NULL;
    unit->outfile = NULL;
    unit->append = 0;
    unit->backgrnd = 0;
    unit->psubcmd = NULL;
    unit->pipe = NULL;
    unit->next = NULL;
    unit->type = NXT;
    return unit;
}

