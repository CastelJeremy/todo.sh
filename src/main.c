#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

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
int read_tasks(char* filename, Task* tasks, int* tasks_size) {
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

int print_tasks(Task* tasks, int tasks_size) {
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

int main() {
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

        return EXIT_FAILURE;
    }

    print_tasks(tasks, tasks_size);

    return EXIT_SUCCESS;
}
