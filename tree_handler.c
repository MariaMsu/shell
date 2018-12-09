#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "tree.h"

int tree_handler(struct cmd_inf *unit);

// настраивается ввод / вывод в фаилы
void stream_adjustment(struct cmd_inf *unit) {
    if (unit->outfile != NULL) {
        char mode[2];
        if (unit->append == 1)
            strcpy(mode, "a");
        else
            strcpy(mode, "w");
        freopen(unit->outfile, mode, stdout);
    }

    if (unit->infile)
        freopen(unit->infile, "r", stdin);
}

// перематывает древовидную структуру к концу сабшелла
struct cmd_inf *rewind_pipe(struct cmd_inf *unit) {
    if ((unit->pipe == NULL) && (unit->psubcmd == NULL))
        return unit;

    if (unit->pipe != NULL)
        return rewind_pipe(unit->pipe);

    if (unit->psubcmd != NULL)
        return rewind_pipe(unit->psubcmd);
}

// запускает команды или рекурсивно вызывает сабшелл
int run_shell_or_subshell(struct cmd_inf *unit) {
//    printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);
    int returned_value = 0;

    if (unit->argv[0] == NULL) {

//        printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);
        returned_value = tree_handler(unit->psubcmd);
    } else {

//        printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);
        // нет, я не устраиваю здесь зомби-апокалипсис
        if (execvp(unit->argv[0], unit->argv) == -1){
//	будет выдавать ошибки на cd и false
//            printf("error in line %d, file tree_handler.c\n", __LINE__);
            return -1;
        }
    }

//    printf("run_shell_or_subshell-------------------    %d\n", returned_value);
    return returned_value;
}

// в цикле обрабатывает пайп от начала до конца
int pipe_handler(struct cmd_inf *unit) {
    int pid;
    int status;
    int fd[2];

    while (unit != NULL) {
//        printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);

        pipe(fd);
        pid = fork();

        // сын
        if (pid == 0) {
            signal(SIGINT, SIG_DFL);

            stream_adjustment(unit);

            if (unit->pipe != NULL) {
                dup2(fd[1], 1);
            }
            close(fd[1]);
            close(fd[0]);

            if (run_shell_or_subshell(unit) == -1) {
                printf("error in line %d, file tree_handler.c\n", __LINE__);
                return -1;
            }

            exit(0);
        }
        dup2(fd[0], 0);
        close(fd[1]);
        close(fd[0]);

        unit = unit->pipe;
    }

    while (wait(&status) != -1);
    return status;
}

// главная функция - обработчик дерева
int tree_handler(struct cmd_inf *unit) {
    if (unit == NULL)
        return 0;

    int status = 0;
//    printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);

    // обрабатывается cd (смена директории)
    if ((unit->argv[0] != NULL) && (strcmp(unit->argv[0], "cd") == 0)) {

//    printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);
        chdir(unit->argv[1]);

        status = tree_handler(unit->psubcmd);

        if (unit->next != NULL)
            if ((unit->type == NXT) ||
                ((unit->type == AND) && (status == 0)) ||
                ((unit->type == OR) && (status != 0)))
                status = tree_handler(unit->next);

        return status;
    }

    // pipe
    if (unit->pipe != NULL) {
        int in = dup(0); //  чудо - штучка
        status = pipe_handler(unit);
        dup2(in, 0);
        unit = rewind_pipe(unit);

        if (unit == NULL)
            return status;

//        printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);
	    
//         status = tree_handler(unit->psubcmd);

        if (unit->next != NULL)
            if ((unit->type == NXT) ||
                ((unit->type == AND) && (status == 0)) ||
                ((unit->type == OR) && (status != 0)))
                status = tree_handler(unit->next);

        return status;
    }

    // Дочерний процесс
    if (fork() == 0) {
        stream_adjustment(unit);

        // фон
        if (unit->backgrnd == 1) {
            signal(SIGINT, SIG_IGN);

            // внук
            if (fork() == 0) {

                run_shell_or_subshell(unit);
                exit(0);

            } else

                // kill father
		// фоновый процесс берет на себя ос
                kill(getpid(), SIGKILL);

            // не фон
        } else {
//            printf("UNIT %s in line %d\n", unit->argv[0], __LINE__);
            status = run_shell_or_subshell(unit);
//            printf("i returnsed %d\n", status);
            if (status != 0)
                exit(1);
            else
                exit(0);
        }

        // father
    } else {
        wait(&status);
        status = WEXITSTATUS(status);

        if (unit->next != NULL)
            if ((unit->type == NXT) ||
                ((unit->type == AND) && (status == 0)) ||
                ((unit->type == OR) && (status != 0)))
                status = tree_handler(unit->next);

        return status;
    }
}
