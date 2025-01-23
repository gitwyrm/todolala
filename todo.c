#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * This program uses a Markdown-based "todo" format, where each line is:
 *   - [ ] task
 * or
 *   - [x] task
 *
 * Usage:
 *
 *   1) Add a new todo:
 *       todo "my new task"
 *
 *   2) List unfinished todos:
 *       todo list
 *
 *   3) Check (mark done) the Nth unfinished todo:
 *       todo check 3
 *
 *   4) Remove the Nth unfinished todo:
 *       todo remove 2
 *
 *   5) Remove all finished tasks:
 *       todo clean
 *
 * The unfinished tasks have the format:  - [ ] my task
 * The finished tasks have the format:    - [x] my task
 *
 * The code considers "the Nth unfinished todo" by counting only lines
 * that start with "- [ ]".
 *
 * Lines that don't start with "- [ ]" or "- [x]" are ignored for indexing,
 * so you can have extra text in your todo.md.
 */

#define TODOS_FILE "todo.md"
#define MAX_LINE_LENGTH 256

/**
 * Global pointer holding all lines from TODO file.
 */
static char **todo_lines = NULL;

/**
 * Print usage instructions to the console.
 */
static void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s \"<task>\"      - Add a new task to %s.\n", prog_name, TODOS_FILE);
    printf("  %s list          - List all unfinished tasks.\n", prog_name);
    printf("  %s check <index> - Mark the <index>th unfinished task as finished.\n", prog_name);
    printf("  %s remove <index> - Remove the <index>th unfinished task.\n", prog_name);
    printf("  %s clean         - Remove all finished tasks.\n", prog_name);
}

/**
 * Skip leading whitespace in a string and return a pointer to the first
 * non-whitespace character.
 */
static char *skip_leading_whitespace(char *str) {
    while (isspace((unsigned char)*str)) {
        str++;
    }
    return str;
}

/**
 * Read all lines from the todo file into a NULL-terminated array of strings.
 * Returns NULL if the file doesn't exist or is empty.
 */
static char **get_all_lines(void) {
    FILE *file = fopen(TODOS_FILE, "r");
    if (!file) {
        // "No todos found" is not necessarily an error; could just be an empty file.
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    char **lines = NULL;
    int num_lines = 0;

    while (fgets(line, sizeof(line), file)) {
        num_lines++;
        lines = realloc(lines, num_lines * sizeof(char *));
        if (!lines) {
            perror("realloc");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        lines[num_lines - 1] = strdup(line);
    }
    fclose(file);

    if (num_lines == 0) {
        free(lines);
        return NULL;
    }

    // Null-terminate the array
    lines = realloc(lines, (num_lines + 1) * sizeof(char *));
    lines[num_lines] = NULL;

    return lines;
}

/**
 * Return the line numbers of all unfinished tasks (those starting with "- [ ]"),
 * along with a count of how many there are.
 *
 * @param count_out Output parameter for number of unfinished tasks.
 * @return Pointer to a dynamically allocated int array (line indices).
 */
static int *get_unfinished_tasks(int *count_out) {
    int *unfinished_tasks = NULL;
    int num_unfinished = 0;

    if (!todo_lines) {
        *count_out = 0;
        return NULL;
    }

    for (int i = 0; todo_lines[i]; i++) {
        char *trimmed_line = skip_leading_whitespace(todo_lines[i]);
        if (strncmp(trimmed_line, "- [ ]", 5) == 0) {
            num_unfinished++;
            unfinished_tasks = realloc(unfinished_tasks, num_unfinished * sizeof(int));
            if (!unfinished_tasks) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            unfinished_tasks[num_unfinished - 1] = i;
        }
    }

    *count_out = num_unfinished;
    return unfinished_tasks;
}

/**
 * Return the line numbers of all finished tasks (those starting with "- [x]"),
 * along with a count of how many there are.
 *
 * @param count_out Output parameter for number of finished tasks.
 * @return Pointer to a dynamically allocated int array (line indices).
 */
static int *get_finished_tasks(int *count_out) {
    int *finished_tasks = NULL;
    int num_finished = 0;

    if (!todo_lines) {
        *count_out = 0;
        return NULL;
    }

    for (int i = 0; todo_lines[i]; i++) {
        char *trimmed_line = skip_leading_whitespace(todo_lines[i]);
        if (strncmp(trimmed_line, "- [x]", 5) == 0) {
            num_finished++;
            finished_tasks = realloc(finished_tasks, num_finished * sizeof(int));
            if (!finished_tasks) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            finished_tasks[num_finished - 1] = i;
        }
    }

    *count_out = num_finished;
    return finished_tasks;
}

/**
 * Delete a line from todo_lines (shift everything up).
 *
 * @param line_index Index in todo_lines to delete.
 */
static void delete_line(int line_index) {
    free(todo_lines[line_index]);
    // Shift lines down
    for (int i = line_index; todo_lines[i]; i++) {
        todo_lines[i] = todo_lines[i + 1];
    }
}

/**
 * Remove all finished tasks from todo_lines.
 *
 * We remove from last to first so indices are not messed up after each removal.
 */
static void remove_finished_tasks(void) {
    int count = 0;
    int *finished_tasks = get_finished_tasks(&count);
    if (!finished_tasks || count == 0) {
        printf("No finished tasks found.\n");
        free(finished_tasks);
        return;
    }

    // Remove from bottom to top
    for (int i = count - 1; i >= 0; i--) {
        delete_line(finished_tasks[i]);
    }
    free(finished_tasks);
}

/**
 * Remove the Nth unfinished task (1-based index).
 */
static void remove_task(int index) {
    if (index <= 0) {
        printf("Invalid index: %d\n", index);
        return;
    }

    int count = 0;
    int *unfinished_tasks = get_unfinished_tasks(&count);
    if (!unfinished_tasks || count == 0) {
        printf("No unfinished tasks found.\n");
        free(unfinished_tasks);
        return;
    }

    if (index > count) {
        printf("Invalid index: %d (only %d unfinished tasks)\n", index, count);
        free(unfinished_tasks);
        return;
    }

    // Convert the user's 1-based index to the line index in todo_lines
    int line_index = unfinished_tasks[index - 1];
    delete_line(line_index);

    free(unfinished_tasks);
}

/**
 * Check (mark done) the Nth unfinished task, i.e., the line that begins with "- [ ]".
 *
 * @param index The 1-based index of the unfinished task to mark as finished.
 */
static void check_todo(int index) {
    if (index <= 0) {
        printf("Invalid index: %d\n", index);
        return;
    }

    int count = 0;
    int *unfinished_tasks = get_unfinished_tasks(&count);
    if (!unfinished_tasks || count == 0) {
        printf("No unfinished tasks found.\n");
        free(unfinished_tasks);
        return;
    }

    if (index > count) {
        printf("Invalid index: %d (only %d unfinished tasks)\n", index, count);
        free(unfinished_tasks);
        return;
    }

    int line_index = unfinished_tasks[index - 1];
    char *trimmed_line = skip_leading_whitespace(todo_lines[line_index]);

    // If it's indeed "- [ ]", replace with "- [x]"
    if (strncmp(trimmed_line, "- [ ]", 5) == 0) {
        // Overwrite the bracket portion
        strncpy(trimmed_line, "- [x]", 5);
    }

    free(unfinished_tasks);
}

/**
 * List all unfinished tasks (lines beginning with "- [ ]") from todo_lines
 * with their 1-based indices.
 */
static void list_todos(void) {
    int count = 0;
    int *unfinished_tasks = get_unfinished_tasks(&count);
    if (!unfinished_tasks || count == 0) {
        printf("No unfinished tasks found.\n");
        free(unfinished_tasks);
        return;
    }

    for (int i = 0; i < count; i++) {
        // Show the original line, skipping leading spaces if you want
        int line_index = unfinished_tasks[i];
        char *trimmed_line = skip_leading_whitespace(todo_lines[line_index]);
        // Print as "1: - [ ] something"
        printf("%d: %s", i + 1, trimmed_line);
    }

    free(unfinished_tasks);
}

/**
 * Append a new task in Markdown format ("- [ ] <task>") to todo.md.
 *
 * @param task The text of the task to add.
 */
static void add_todo(const char *task) {
    FILE *file = fopen(TODOS_FILE, "a");
    if (!file) {
        printf("Error opening %s for writing.\n", TODOS_FILE);
        return;
    }

    fprintf(file, "- [ ] %s\n", task);
    fclose(file);
}

/**
 * Save todo_lines to the todo file (overwrite).
 */
static void save_todos(void) {
    // If we have never read any lines, there's nothing to save
    if (!todo_lines) return;

    FILE *file = fopen(TODOS_FILE, "w");
    if (!file) {
        printf("Error opening %s for writing.\n", TODOS_FILE);
        return;
    }

    for (int i = 0; todo_lines[i]; i++) {
        fprintf(file, "%s", todo_lines[i]);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Load lines from file (this populates todo_lines globally)
    todo_lines = get_all_lines();

    if (strcmp(argv[1], "list") == 0) {
        list_todos();
    }
    else if (strcmp(argv[1], "check") == 0) {
        if (argc < 3) {
            printf("Usage: %s check <index>\n", argv[0]);
            return 1;
        }
        int index = atoi(argv[2]);
        check_todo(index);
        save_todos();
    }
    else if (strcmp(argv[1], "remove") == 0) {
        if (argc < 3) {
            printf("Usage: %s remove <index>\n", argv[0]);
            return 1;
        }
        int index = atoi(argv[2]);
        remove_task(index);
        save_todos();
    }
    else if (strcmp(argv[1], "clean") == 0) {
        remove_finished_tasks();
        save_todos();
    }
    else {
        // Assume the argument is a new task to add
        add_todo(argv[1]);
    }

    return 0;
}