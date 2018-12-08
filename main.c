#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <zconf.h>
#include <signal.h>
#include "list.h"
#include "tree.h"
#include "tree_handler.h"

#define DEBUG 0

#define BUFFER_SIZE 1
#define f_ORDINARY_WORK 0
#define f_END_OF_FILE 1
#define f_END_OF_LINE 2
#define f_INCORRECT_INPUT 3

#define MAX_PATH 32


#if DEBUG == 1
FILE *f;
#define PATH f
#else
#define PATH stdin
#endif

char special_symbols[] = {'|', '&', ';', '>', '<', '(', ')'};

//check entry of symbol in mas
int in(char symbol, char *mas) {
    for (int i = 0; i < strlen(mas); i++) {
        if (symbol == mas[i])
            return 1;
    }
    return 0;
}

//append char to string
void add_char_to_end(char symbol, char *word) {
    size_t length = strlen(word);
    word[length] = symbol;
    word[length + 1] = '\0';
}

char *get_string(short *const flag) {
    static char *buf;
    static int i = BUFFER_SIZE;
    static char buf_initialized = 0;
    static int end_of_sequence = BUFFER_SIZE;

    static char flag_of_EOF = 0;
    if (flag_of_EOF == 1) {
        flag_of_EOF = 0;
        free(buf);
        buf_initialized = 0;
        i = BUFFER_SIZE;
        end_of_sequence = BUFFER_SIZE;
        *flag = f_END_OF_FILE;
        return NULL;
    }

    if (!buf_initialized) {
        buf = malloc(BUFFER_SIZE * sizeof(char) + 1); //for '\0'
        buf_initialized = 1;
    }
    size_t word_size = BUFFER_SIZE * sizeof(char);
    char *word = malloc(word_size + 1); //for '\0'
    word[0] = '\0';
    int quote_flag = 0;
    int slash_flag = 0;

    for (;; i++) {

        if (i >= BUFFER_SIZE) {
            buf[0] = '\0';
            end_of_sequence = (int) fread(buf, sizeof(char), BUFFER_SIZE, PATH);
            if (end_of_sequence < BUFFER_SIZE) {
                buf[end_of_sequence] = '\0';
                add_char_to_end('\n', buf);
                *flag = f_END_OF_FILE;
            }
            i = 0;
        }


        size_t w_length = strlen(word);
        if (w_length == word_size) {
            word = realloc(word, word_size + BUFFER_SIZE * sizeof(char) + 1);//for '\0'
            word_size += BUFFER_SIZE * sizeof(char);
        }

        if (slash_flag == 1) {
            add_char_to_end(buf[i], word);
            slash_flag = 0;
            continue;
        }

        switch (buf[i]) {

            case '"':
                if (quote_flag == 1) {
                    i++;
                    return word;
                }
                quote_flag = 1;
                continue;

            case '\\':
                slash_flag = 1;
                continue;

            case '|':
            case '&':
            case '>':
                if ((w_length == 0) || ((word[w_length - 1] == buf[i]) && (w_length == 1)))
                    add_char_to_end(buf[i], word);
                else {
                    if (quote_flag == 1)
                        continue;
                    *flag = f_ORDINARY_WORK;
                    return word;
                }
                break;
            case ';':
            case '<':
            case '(':
            case ')':
                if (w_length == 0)
                    add_char_to_end(buf[i], word);
                else {
                    if (quote_flag == 1)
                        continue;
                    *flag = f_ORDINARY_WORK;
                    return word;
                }
                break;
            case 'a'...'z':
            case 'A'...'Z':
            case '0'...'9':
            case ':':
            case '_':
            case '$':
            case '/':
            case '.':
            case '-': // new option
                if ((w_length == 0) || (in(word[w_length - 1], special_symbols) == 0))
                    add_char_to_end(buf[i], word);
                else {
                    if (quote_flag == 1)
                        continue;
                    *flag = f_ORDINARY_WORK;
                    return word;
                }
                break;
            case ' ':
            case '\t':
                if (w_length == 0)
                    continue;
                else {
                    if (quote_flag == 1) {
                        add_char_to_end(buf[i], word);
                        continue;
                    }
                    *flag = f_ORDINARY_WORK;
                    return word;
                }
            case '\n':
                if (quote_flag != 1) {
                    if (*flag == f_END_OF_FILE) {
                        free(buf);
                        buf_initialized = 0;
                        i = BUFFER_SIZE;
                        end_of_sequence = BUFFER_SIZE;
                    } else {
                        *flag = f_END_OF_LINE;
                        i++;
                    }
                    if (w_length == 0) {
                        free(word);
                        return NULL;
                    }
                    return word;
                }

            case '#':
#if DEBUG == 0
                if (quote_flag != 1) {
                while ((buf[i] != '\n') && (i != end_of_sequence))
                        i++;
                    if (i == end_of_sequence) {
                        int c;
                        do {
                            c = fgetc(stdin);
                        } while ((c != '\n') && (c != EOF));
                        if (c == EOF)
                            flag_of_EOF = 1;
                    }
                    *flag = f_END_OF_LINE;
                    return word;
                    }
#else
                printf("comment was met\n");
#endif
                // error
            default:

#if DEBUG == 0
                while ((buf[i] != '\n') && (i != end_of_sequence))
                    i++;
                if (i == end_of_sequence) {
                    int c;
                    do {
                        c = fgetc(stdin);
                    } while ((c != '\n') && (c != EOF));
                    if (c == EOF)
                        flag_of_EOF = 1;
                }
                *flag = f_INCORRECT_INPUT;
#else
                printf("error\n");
                *flag = f_END_OF_FILE;
#endif

                free(word);
                return NULL;

                // буквы, цифры, подчеркивание, пробельные символы (в том числе символы табуляции '\t' и перевода строки '\n'
                // |, ||, &, &&, ; , >, >>, <, (, ).
                // (12345);|||67\
                // (12345);|||67<112211>>3(;)1
        }
    }
}

char *dollar_sign_handler(char *string) {
    if (string[0] != '$')
        return string;

    if (!strcmp(string, "$HOME")) {
        string = realloc(string, MAX_PATH);
        strcpy(string, getenv("HOME"));
    } else if (!strcmp(string, "$SHELL")) {
        string = realloc(string, MAX_PATH);
        getcwd(string, PATH_MAX + 1);
    } else if (!strcmp(string, "$USER")) {
        string = realloc(string, MAX_PATH);
        strcpy(string, getenv("LOGNAME"));
    } else if (!strcmp(string, "$EUID")) {
        string = realloc(string, MAX_PATH);
        sprintf(string, "%d", getuid());
    }
    return string;
}

void handler(string_list unit) {
    if (unit == 0)
        return;
    if (check_brace(unit)) {
        printf("incorrect braces\n");
        return;
    }
    struct cmd_inf *tree = list_to_tree(unit);
//    print_tree(tree);

    tree_handler(tree);

    del_tree(tree);
    del_list(unit);
}

void invitation() {
//    printf("\x1b[30;5m"); // здесь изменяется цвет на зеленый
//    printf("\033[32;41;5;1;3;61m"); // здесь изменяется цвет на зеленый
    printf("\033[32;41;1;3;61m"); // здесь то же саоме, но не моргает
    char s[100];
    getcwd(s, 100);
    printf("%s", s);
    printf("\x1b[0m");
    printf("\x1b[32m");
    printf("\u2765 ");
    printf("\x1b[0m");
}

int main() {

#if DEBUG == 1
    f = fopen("/home/maria/Desktop/prak/file.txt", "r");
#endif
    signal(SIGINT, SIG_IGN);

    short flag = f_ORDINARY_WORK;
    while (flag != f_END_OF_FILE) {
        invitation();
#if DEBUG == 1
        printf("\n");
#endif
        string_list array = 0;
        char *string;

        while (1) {
            string = get_string(&flag);
            if (string != NULL)
                append_string(&array, dollar_sign_handler(string));
            if (flag == f_END_OF_LINE) {
                handler(array);
                break;
            } else if (flag == f_END_OF_FILE) {
                handler(array);
                break;
            } else if (flag == f_INCORRECT_INPUT) {
                printf("Incorrect input :( \nPlease check your sequence of symbols and try again\n");
                del_list(array);
                break;
            }
        }
    }
#if DEBUG == 1
    fclose(f);
#endif
}
