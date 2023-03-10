#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#define PROGRAM_NAME "todo"

#define STYLE_RESET  "\e[0m"
#define COLOR_CYAN   "\e[49;38;5;81m"
#define FONT_STRIKE  "\e[9m"

#define LABEL_LIMIT 17
#define LINE_LIMIT  LABEL_LIMIT + 1
#define TASKS_LIMIT 64

typedef struct task {
    bool check;
    char label[LABEL_LIMIT];
} Task;

/**
 * @return 
 *  0  on success
 *  -1 on file stream error (check errno)
 *  -2 on file format error
 *  -3 on file format error
 */
static int
read_tasks(char* filename, Task* tasks, int* tasks_size) {
    FILE *f;

    f = fopen(filename, "r");

    if (f == NULL) {
        return -1;
    }

    char* line = calloc(LINE_LIMIT, sizeof(char));

    int c;
    while ((c = fgetc(f)) != EOF) {
        // Line-break indicates a new task
        if (c == '\n') {
            // Initialize a task
            Task task;

            // Line must start with 0 or 1
            if (line[0] == '0') {
                task.check = false;
            } else if (line[0] == '1') {
                task.check = true;
            } else {
                return -2;
            }

            // Copy label
            strncpy(task.label, line + 1, LABEL_LIMIT - 1);

            // Add the task
            tasks[(*tasks_size)++] = task;

            // Reset line
            free(line);
            line = calloc(LINE_LIMIT, sizeof(char));
        } else if (strlen(line) < LINE_LIMIT - 1) {
            line[strlen(line)] = c;
        } else {
            return -3;
        }
    }

    return 0;
}

static int
print_tasks(Task* tasks, int tasks_size) {
    for (int i = 0; i < tasks_size; i++) {
        printf(COLOR_CYAN);

        if (i < 9) {
            printf("0");
        }

        printf("%i "  STYLE_RESET, i + 1);

        if (tasks[i].check == 1) {
            printf(FONT_STRIKE);
        }

        printf("%s\n" STYLE_RESET, tasks[i].label);
    }

    return 0;
}

static inline int
write_tasks(char* filename, Task* tasks, int tasks_size)
{
    FILE* f;

    f = fopen(filename, "w");

    for (int i = 0; i < tasks_size; i++) {
        fprintf(f, "%i%s\n", tasks[i].check, tasks[i].label);
    }

    fclose(f);

    return (0);
}

static inline void
do_add (char* label, Task* tasks, int* tasks_size) {
    Task task;
    task.check = false;
    strncpy(task.label, label, LABEL_LIMIT - 1);

    tasks[(*tasks_size)++] = task;
}

static inline void
do_update (int index, Task* tasks) {
    tasks[index - 1].check = !tasks[index - 1].check;
}

static inline void
do_delete (int index, Task* tasks, int* tasks_size) {
    if (index == *tasks_size) {
        (*tasks_size)--;
    } else {
        for (int i = index; i < *tasks_size; i++) {
            tasks[i - 1] = tasks[i];
        }

        (*tasks_size)--;
    }
}

// From coreutils/system.h
static inline void
emit_try_help (void)
{
  fprintf (stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
}

void
error (char* msg)
{
    fprintf (stderr, "%s: %s\n", PROGRAM_NAME, msg);
}

void
dir (char* msg)
{
    error (msg);
    exit (EXIT_FAILURE);
}

void
usage (int status)
{
    if (status != EXIT_SUCCESS) {
        emit_try_help ();
    } else {
        printf ("\
Usage: %s [OPTION]...\n\
", PROGRAM_NAME);
    }

    exit (status);
}

struct
todo_options {
    char** add_list;
    int    add_list_size;
    int*   update_index_list;
    int    update_index_list_size;
    int*   delete_index_list;
    int    delete_index_list_size;
    int*   mv_src_index_list;
    int    mv_src_index_list_size;
    int*   mv_dest_index_list;
    int    mv_dest_index_list_size;
};

static inline void
todo_options_init (struct todo_options *x)
{
    x->add_list = malloc(32);
    x->add_list_size = 0;
    x->update_index_list = malloc(32 * sizeof(int));
    x->update_index_list_size = 0;
    x->delete_index_list = malloc(32 * sizeof(int));
    x->delete_index_list_size = 0;
}

int
main(int argc, char** argv) {
    Task tasks[TASKS_LIMIT];
    int tasks_size = 0;

    int r = read_tasks("/tmp/todo.txt", tasks, &tasks_size);

    // Error handling
    if (r != 0) {
        if (r == -1) {
            printf("cannot stat '' : %s\n", strerror(errno));
        } else if (r == -2) {
            printf("invalid file '' : Line does not start with 0 or 1\n");
        } else if (r == -3) {
            printf("invalid file '' : Line limit reached");
        } else {
            printf("Unknown error %i\n", r);
        }

        exit (EXIT_FAILURE);
    }

    struct todo_options x;

    todo_options_init (&x);

    struct option options[] = {
        { "add",    required_argument, NULL, 'a' },
        { "update", required_argument, NULL, 'u' },
        { "delete", required_argument, NULL, 'd' },
        { "help",   no_argument,       NULL, 'h' }
    };

    char opt;
    while ((opt = getopt_long(argc, argv, "a:u:d:h", options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                if (x.add_list_size + 1 <= 32) {
                    if (strlen(optarg) < LABEL_LIMIT) {
                        x.add_list[x.add_list_size++] = optarg;
                    } else {
                        error ("argument exceeds the limit");
                        usage (EXIT_FAILURE);
                    }
                } else {
                    error ("too many options");
                    usage (EXIT_FAILURE);
                }
                break;
            case 'u':
                if (x.update_index_list_size + 1 <= 32) {
                    x.update_index_list[x.update_index_list_size++] = atoi(optarg);
                } else {
                    error ("too many options");
                    usage (EXIT_FAILURE);
                }
                break;
            case 'd':
                if (x.delete_index_list_size + 1 <= 32) {
                    x.delete_index_list[x.delete_index_list_size++] = atoi(optarg);
                } else {
                    error ("too many options");
                    usage (EXIT_FAILURE);
                }
                break;
            case 'h':
                usage (EXIT_SUCCESS);
                break;
            case ':':
            case '?':
                usage (EXIT_FAILURE);
                break;
        }
    }

    for (int i = 0; i < x.add_list_size; i++) {
        do_add (x.add_list[i], tasks, &tasks_size);
    }

    for (int i = 0; i < x.update_index_list_size; i++) {
        do_update (x.update_index_list[i], tasks);
    }

    for (int i = 0; i < x.delete_index_list_size; i++) {
        do_delete (x.delete_index_list[i], tasks, &tasks_size);
    }

    print_tasks(tasks, tasks_size);

    write_tasks("/tmp/todo.txt", tasks, tasks_size);

    exit (EXIT_SUCCESS);
}
