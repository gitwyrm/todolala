#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "todo.h"

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
 * It is also possible to use multiple indexes for the check and remove commands,
 * and most of the commands have single letter abbreviations.
 * 
 * If you want to use a different file, supply it as the first argument:
 *
 *   todo README.md "my task"
 *   todo README.md list
 *   todo some_other.md check 2
 *
 * etc. If no filename is given, the default is todo.md.
 */

const char *todos_filename = NULL;
char **todo_lines = NULL;

/**
 * Print usage instructions to the console.
 */
void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s [<file.md>] \"<task>\"           - Add a new task (default file: todo.md).\n", prog_name);
    printf("  %s [<file.md>] l(ist)             - List all unfinished tasks.\n", prog_name);
    printf("  %s [<file.md>] c(heck) <index>    - Mark the <index>th unfinished task as finished.\n", prog_name);
    printf("  %s [<file.md>] r(emove) <index>   - Remove the <index>th unfinished task.\n", prog_name);
    printf("  %s [<file.md>] clean              - Remove all finished tasks.\n", prog_name);
    printf("\n");
    printf("You can also use multiple <index>es for check and remove commands, i.e. todo check 1 2 3.\n");
}

/**
 * Skip leading whitespace in a string and return a pointer to the first
 * non-whitespace character.
 */
char *skip_leading_whitespace(char *str) {
    while (isspace((unsigned char)*str)) {
        str++;
    }
    return str;
}

/**
 * Read all lines from the todo file into a NULL-terminated array of strings.
 * Returns NULL if the file doesn't exist or is empty.
 */
char **get_all_lines(void) {
    FILE *file = fopen(todos_filename, "r");
    if (!file) {
        // It's not necessarily an error if the file doesn't exist;
        // we may be creating a new one.
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
int *get_unfinished_tasks(int *count_out) {
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
int *get_finished_tasks(int *count_out) {
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
void delete_line(int line_index) {
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
void remove_finished_tasks(void) {
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
void remove_task(int index) {
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
void check_todo(int index) {
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
void list_todos(void) {
    int count = 0;
    int *unfinished_tasks = get_unfinished_tasks(&count);
    if (!unfinished_tasks || count == 0) {
        printf("No unfinished tasks found.\n");
        free(unfinished_tasks);
        return;
    }

    for (int i = 0; i < count; i++) {
        int line_index = unfinished_tasks[i];
        char *trimmed_line = skip_leading_whitespace(todo_lines[line_index]);
        // Print as "1) something"
        printf("%d) %s", i + 1, trimmed_line + 6);
    }

    free(unfinished_tasks);
}

/**
 * Append a new task in Markdown format ("- [ ] <task>") to the current file.
 * 
 * If the last line in the file has no newline at the end, adds a newline before the new task.
 *
 * @param task The text of the task to add.
 */
void add_todo(const char *task) {
    FILE *file = fopen(todos_filename, "a");
    if (!file) {
        printf("Error opening %s for appending.\n", todos_filename);
        return;
    }

    if (todo_lines) {
        // Find the last line in the array
        int i = 0;
        while (todo_lines[i + 1] != NULL) {
            i++;
        }
        char *last_line = todo_lines[i];
        
        // Check if last_line ends with a newline
        size_t len = strlen(last_line);
        if (len > 0 && last_line[len - 1] != '\n') {
            fprintf(file, "\n");
        }
    }

    fprintf(file, "- [ ] %s\n", task);
    fclose(file);
}

/**
 * Save todo_lines to the current file (overwrite).
 */
void save_todos(void) {
    // If we have never read any lines, there's nothing to save
    if (!todo_lines) return;

    FILE *file = fopen(todos_filename, "w");
    if (!file) {
        printf("Error opening %s for writing.\n", todos_filename);
        return;
    }

    for (int i = 0; todo_lines[i]; i++) {
        fprintf(file, "%s", todo_lines[i]);
    }

    fclose(file);
}

/**
 * Comparison function for descending order of two ints.
 * Used by qsort() when we want to process bigger indexes first.
 */
int compare_int_desc(const void *a, const void *b) {
    int ai = *(const int *)a;
    int bi = *(const int *)b;
    // Return negative if bi < ai so that bigger numbers come first
    return (bi - ai);
}

/**
 * Calls function pointer for each index in the given arguments.
 * 
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @param index Index of the first index argument.
 * @param fn Function pointer to a void function that takes an int.
 * @return 0 if successful, 1 if there was an error.
 */
int call_fn_with_indexes(int argc, char *argv[], int index, void (*fn)(int)) {
    // Collect all indexes into an array
    int numIndexes = 0;
    int capacity = argc - index;
    int *indexes = malloc(capacity * sizeof(int));
    if (!indexes) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Parse all remaining arguments as indexes
    for (int i = index; i < argc; i++) {
        int idx = atoi(argv[i]);
        if (idx <= 0) {
            printf("Skipping invalid index: %s\n", argv[i]);
            continue;
        }
        indexes[numIndexes++] = idx;
    }

    // Sort the indexes in descending order so we handle highest first
    qsort(indexes, numIndexes, sizeof(int), compare_int_desc);

    // Call the fn on each requested index
    for (int i = 0; i < numIndexes; i++) {
        fn(indexes[i]);
    }

    free(indexes);
    return 0;
}

#ifndef TESTING
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    todos_filename = "todo.md";
    todo_lines = NULL;

    /*
     * Check if the first argument ends with ".md". If yes, treat it as a filename
     * and shift our parsing index so the next argument is the command/task.
     */
    int argIndex = 1;
    size_t len = strlen(argv[1]);
    if (len > 3 && strcmp(argv[1] + (len - 3), ".md") == 0) {
        // Use the given file as our todos_filename
        todos_filename = argv[1];
        argIndex = 2; // Next argument is the command/task
    }

    // If we consumed the filename, but there are no more args, print usage
    if (argIndex > (argc - 1)) {
        print_usage(argv[0]);
        return 1;
    }

    // Load lines from the selected file
    todo_lines = get_all_lines();

    /*
     * Now parse the next argument. If it's "list", "check", "remove", or "clean",
     * do the corresponding operation; otherwise, assume it's a new task.
     * 
     * There are also one letter shortcuts for almost every command, so l for list,
     * c for check and so on.
     */
    if (strcmp(argv[argIndex], "list") == 0 || strcmp(argv[argIndex], "l") == 0) {
        list_todos();
    }
    else if (strcmp(argv[argIndex], "check") == 0 || strcmp(argv[argIndex], "c") == 0) {
        if (argIndex + 1 >= argc) {
            printf("Usage: %s [<file.md>] check <index>\n", argv[0]);
            return 1;
        }

        call_fn_with_indexes(argc, argv, argIndex + 1, check_todo);

        save_todos();
    }
    else if (strcmp(argv[argIndex], "remove") == 0 || strcmp(argv[argIndex], "r") == 0) {
        if (argIndex + 1 >= argc) {
            printf("Usage: %s [<file.md>] remove <index>\n", argv[0]);
            return 1;
        }

        call_fn_with_indexes(argc, argv, argIndex + 1, remove_task);

        save_todos();
    }
    else if (strcmp(argv[argIndex], "clean") == 0) {
        remove_finished_tasks();
        save_todos();
    }
    else {
        // Assume the argument is a new task to add
        // (If there are multiple arguments, you might want to join them)
        add_todo(argv[argIndex]);
    }

    return 0;
}
#endif
