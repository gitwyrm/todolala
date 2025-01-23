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
 * The unfinished tasks have the format:  - [ ] my task
 * The finished tasks have the format:    - [x] my task
 *
 * The code considers "the Nth unfinished todo" by counting only lines
 * that start with "- [ ]".
 *
 * Lines that don't start with "- [ ]" are ignored, so you can use this
 * to edit tasks in any markdown file.
 */

#define TODOS_FILE "todo.md"
#define TEMP_FILE  "temp.md"
#define MAX_LINE_LENGTH 256

/**
 * Print usage instructions to the console.
 */
static void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s \"<task>\"      - Add a new task to todo.md.\n", prog_name);
    printf("  %s list          - List all unfinished tasks.\n", prog_name);
    printf("  %s check <index> - Mark the <index>th unfinished task as finished.\n", prog_name);
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
 * Check (mark done) the Nth unfinished todo, i.e., the line that begins with "- [ ]".
 *
 * @param index The 1-based index of the unfinished task to mark as finished.
 */
static void check_todo(int index) {
    FILE *file = fopen(TODOS_FILE, "r");
    if (!file) {
        printf("No todos found.\n");
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        fclose(file);
        printf("Error creating temporary file.\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int current_unfinished = 1;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        char *trimmed_line = skip_leading_whitespace(line);
        /*
         * Unfinished tasks are recognized by the first 5 characters: "- [ ]".
         *   - [ ] <task>
         * Done tasks are recognized by: "- [x]".
         */
        if (strncmp(trimmed_line, "- [ ]", 5) == 0) {
            // This line is an unfinished task
            if (current_unfinished == index) {
                // Mark it as finished: replace "- [ ]" with "- [x]"
                // Keep the rest of the line after the first 5 characters
                fprintf(temp, "- [x]%s", line + 5);
                found = 1;
            } else {
                // Leave it as is
                fprintf(temp, "%s", line);
            }
            current_unfinished++;
        } else {
            // Copy all other lines (finished tasks, malformed lines, etc.) unchanged
            fprintf(temp, "%s", line);
        }
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        printf("Invalid index: %d\n", index);
        remove(TEMP_FILE);
        return;
    }

    // Overwrite the old file with the updated contents
    remove(TODOS_FILE);
    rename(TEMP_FILE, TODOS_FILE);
}

/**
 * List all unfinished tasks (lines beginning with "- [ ]") from the todo file.
 */
static void list_todos(void) {
    FILE *file = fopen(TODOS_FILE, "r");
    if (!file) {
        printf("No todos found.\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int index = 1;

    while (fgets(line, sizeof(line), file)) {
        char *trimmed_line = skip_leading_whitespace(line);
        // Check if the line starts with the Markdown "unfinished" pattern: "- [ ]"
        if (strncmp(trimmed_line, "- [ ]", 5) == 0) {
            printf("%d) %s", index, line);
            index++;
        }
    }

    fclose(file);
}

/**
 * Append a new task in Markdown format to todos.md.
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        list_todos();
    } else if (strcmp(argv[1], "check") == 0) {
        if (argc < 3) {
            printf("Usage: %s check <index>\n", argv[0]);
            return 1;
        }
        int index = atoi(argv[2]);
        if (index <= 0) {
            printf("Invalid index: %s\n", argv[2]);
            return 1;
        }
        check_todo(index);
    } else {
        // Add a new todo
        add_todo(argv[1]);
    }

    return 0;
}
