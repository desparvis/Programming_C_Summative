/* student_management.c
   Readable beginner-friendly Student Management System
   Features: dynamic array, file save/load, CRUD, search by name, sort by name,
             function pointers for menu, auto-save on changes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 100
#define FILENAME_DEFAULT "students.txt"
#define LINE_BUF 512

/* Student structure */
typedef struct {
    int id;                 // unique id
    char name[NAME_LEN];    // student name
    float grade;            // single numeric grade (0-100)
} Student;

/* Helper: trim newline from end of string */
static void trim_newline(char *s) {
    size_t L = strlen(s);
    if (L == 0) return;
    if (s[L-1] == '\n') s[L-1] = '\0';
}

/* Helper: read a line into buf (fgets) and trim newline */
static int read_line(char *buf, int size) {
    if (!fgets(buf, size, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

/* Helper: parse int safely from string; returns 1 on success */
static int parse_int(const char *s, int *out) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s) return 0;
    *out = (int)v;
    return 1;
}

/* Helper: parse float safely from string; returns 1 on success */
static int parse_float(const char *s, float *out) {
    char *end = NULL;
    float v = strtof(s, &end);
    if (end == s) return 0;
    *out = v;
    return 1;
}

/* Find student index by ID; return -1 if not found */
static int find_by_id(const Student *arr, int n, int id) {
    for (int i = 0; i < n; ++i) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

/* Resize student array to newSize; returns new pointer (or old pointer on failure) */
static Student *resize_students(Student *students, int newSize) {
    if (newSize == 0) {
        /* free and return NULL */
        free(students);
        return NULL;
    }
    Student *tmp = realloc(students, newSize * sizeof(Student));
    if (!tmp) {
        printf("Error: memory allocation failed.\n");
        return students; /* keep old pointer */
    }
    return tmp;
}

/* Save students to file (text format: id|name|grade per line) */
static int save_to_file(const Student *students, int n, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Error: cannot open file '%s' for writing.\n", filename);
        return 0;
    }
    for (int i = 0; i < n; ++i) {
        /* write: id|name|grade\n */
        fprintf(f, "%d|%s|%.2f\n", students[i].id, students[i].name, students[i].grade);
    }
    fclose(f);
    return 1;
}

/* Load students from file saved with save_to_file; returns new array pointer and updates count */
static Student *load_from_file(Student *students, int *count, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        /* Not an error if file missing on startup; caller can decide */
        return students;
    }

    /* free existing array */
    for (int i = 0; i < *count; ++i) { /* nothing to free inside Student here */ }
    free(students);
    students = NULL;
    *count = 0;

    char line[LINE_BUF];
    while (fgets(line, LINE_BUF, f)) {
        trim_newline(line);
        /* parse id|name|grade */
        char *p = strtok(line, "|");
        if (!p) continue;
        int id = atoi(p);

        p = strtok(NULL, "|");
        if (!p) continue;
        char namebuf[NAME_LEN];
        strncpy(namebuf, p, NAME_LEN);
        namebuf[NAME_LEN-1] = '\0';

        p = strtok(NULL, "|");
        if (!p) continue;
        float grade = (float)atof(p);

        /* append to array */
        int newSize = *count + 1;
        Student *tmp = realloc(students, newSize * sizeof(Student));
        if (!tmp) {
            printf("Error: memory allocation failed while loading.\n");
            fclose(f);
            return students;
        }
        students = tmp;
        students[*count].id = id;
        strncpy(students[*count].name, namebuf, NAME_LEN);
        students[*count].name[NAME_LEN-1] = '\0';
        students[*count].grade = grade;
        *count = newSize;
    }
    fclose(f);
    return students;
}

/* AUTO-SAVE after changes, uses default filename */
static void autosave_if_needed(const Student *students, int n) {
    if (n <= 0) {
        /* if no students, write empty file (optional) */
        save_to_file(students, n, FILENAME_DEFAULT);
        return;
    }
    if (!save_to_file(students, n, FILENAME_DEFAULT)) {
        printf("Warning: autosave failed.\n");
    }
}

/* Add student (interactive), returns new array pointer and updates count */
static Student *add_student(Student *students, int *count) {
    char buf[LINE_BUF];

    printf("Add student\n");

    /* read id */
    printf("Enter ID (integer): ");
    if (!read_line(buf, LINE_BUF)) return students;
    int id;
    if (!parse_int(buf, &id)) { printf("Invalid ID.\n"); return students; }

    /* check unique id */
    if (find_by_id(students, *count, id) != -1) {
        printf("Error: ID already exists.\n");
        return students;
    }

    /* read name */
    printf("Enter full name: ");
    if (!read_line(buf, LINE_BUF)) return students;
    char name[NAME_LEN];
    strncpy(name, buf, NAME_LEN);
    name[NAME_LEN-1] = '\0';

    /* read grade */
    printf("Enter grade (0.0 - 100.0): ");
    if (!read_line(buf, LINE_BUF)) return students;
    float grade;
    if (!parse_float(buf, &grade) || grade < 0.0f || grade > 100.0f) {
        printf("Invalid grade.\n");
        return students;
    }

    /* append to array */
    int newSize = *count + 1;
    Student *tmp = realloc(students, newSize * sizeof(Student));
    if (!tmp) {
        printf("Error: memory allocation failed.\n");
        return students;
    }
    students = tmp;

    students[*count].id = id;
    strncpy(students[*count].name, name, NAME_LEN);
    students[*count].name[NAME_LEN-1] = '\0';
    students[*count].grade = grade;

    *count = newSize;

    /* autosave */
    autosave_if_needed(students, *count);

    printf("Student added.\n");
    return students;
}

/* Display all students */
static void display_students(const Student *students, int count) {
    if (count == 0) {
        printf("No students to display.\n");
        return;
    }

    printf("\n--- Students (%d) ---\n", count);
    for (int i = 0; i < count; ++i) {
        printf("[%d] ID:%d | Name:%s | Grade: %.2f\n",
               i, students[i].id, students[i].name, students[i].grade);
    }
}

/* Update student (by ID) */
static void update_student(Student *students, int count) {
    if (count == 0) {
        printf("No students available.\n");
        return;
    }

    char buf[LINE_BUF];
    printf("Enter ID of student to update: ");
    if (!read_line(buf, LINE_BUF)) return;
    int id;
    if (!parse_int(buf, &id)) { printf("Invalid ID.\n"); return; }

    int idx = find_by_id(students, count, id);
    if (idx == -1) { printf("Student not found.\n"); return; }

    printf("Updating student ID %d (%s)\n", students[idx].id, students[idx].name);

    /* new name */
    printf("Enter new name (or press Enter to keep current): ");
    if (!read_line(buf, LINE_BUF)) return;
    if (buf[0] != '\0') {
        strncpy(students[idx].name, buf, NAME_LEN);
        students[idx].name[NAME_LEN-1] = '\0';
    }

    /* new grade */
    printf("Enter new grade (or press Enter to keep current): ");
    if (!read_line(buf, LINE_BUF)) return;
    if (buf[0] != '\0') {
        float g;
        if (!parse_float(buf, &g) || g < 0.0f || g > 100.0f) {
            printf("Invalid grade, keeping old value.\n");
        } else {
            students[idx].grade = g;
        }
    }

    /* autosave */
    autosave_if_needed(students, count);

    printf("Student updated.\n");
}

/* Delete student by ID */
static Student *delete_student(Student *students, int *count) {
    if (*count == 0) {
        printf("No students to delete.\n");
        return students;
    }

    char buf[LINE_BUF];
    printf("Enter ID of student to delete: ");
    if (!read_line(buf, LINE_BUF)) return students;
    int id;
    if (!parse_int(buf, &id)) { printf("Invalid ID.\n"); return students; }

    int idx = find_by_id(students, *count, id);
    if (idx == -1) { printf("Student not found.\n"); return students; }

    /* shift left */
    for (int i = idx; i < *count - 1; ++i) {
        students[i] = students[i+1];
    }

    int newSize = *count - 1;
    if (newSize == 0) {
        free(students);
        students = NULL;
        *count = 0;
        /* autosave (writes empty file) */
        autosave_if_needed(students, *count);
        printf("Student deleted. No students left.\n");
        return students;
    }

    Student *tmp = realloc(students, newSize * sizeof(Student));
    if (tmp) students = tmp; /* if realloc fails, keep old pointer but adjust count */
    *count = newSize;

    /* autosave */
    autosave_if_needed(students, *count);

    printf("Student deleted.\n");
    return students;
}

/* Convert string to lowercase copy (for case-insensitive search) */
static void strtolower_copy(const char *src, char *dst, int dstSize) {
    int i;
    for (i = 0; i < dstSize-1 && src[i]; ++i) {
        dst[i] = (char)tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

/* Search by name (substring, case-insensitive) */
static void search_by_name(const Student *students, int count) {
    if (count == 0) { printf("No students.\n"); return; }

    char buf[LINE_BUF];
    printf("Enter name or substring to search: ");
    if (!read_line(buf, LINE_BUF)) return;

    char key[LINE_BUF];
    strtolower_copy(buf, key, LINE_BUF);

    int found = 0;
    for (int i = 0; i < count; ++i) {
        char name_l[NAME_LEN];
        strtolower_copy(students[i].name, name_l, NAME_LEN);
        if (strstr(name_l, key) != NULL) {
            printf("Found: ID:%d | Name:%s | Grade: %.2f\n",
                   students[i].id, students[i].name, students[i].grade);
            found = 1;
        }
    }
    if (!found) printf("No matches found.\n");
}

/* Sort by name (ascending) using bubble sort */
static void sort_by_name(Student *students, int count) {
    if (count < 2) { printf("Not enough students to sort.\n"); return; }

    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - 1 - i; ++j) {
            if (strcmp(students[j].name, students[j+1].name) > 0) {
                Student tmp = students[j];
                students[j] = students[j+1];
                students[j+1] = tmp;
            }
        }
    }
    /* autosave after changing order */
    autosave_if_needed(students, count);
    printf("Sorted by name.\n");
}

/* Manual save (ask filename) */
static void manual_save(const Student *students, int count) {
    char fname[LINE_BUF];
    printf("Enter filename to save to (or press Enter for default '%s'): ", FILENAME_DEFAULT);
    if (!read_line(fname, LINE_BUF)) return;
    const char *use = fname[0] == '\0' ? FILENAME_DEFAULT : fname;
    if (save_to_file(students, count, use)) {
        printf("Saved to '%s'.\n", use);
    } else {
        printf("Save failed.\n");
    }
}

/* Manual load (ask filename) */
static Student *manual_load(Student *students, int *count) {
    char fname[LINE_BUF];
    printf("Enter filename to load from (or press Enter for default '%s'): ", FILENAME_DEFAULT);
    if (!read_line(fname, LINE_BUF)) return students;
    const char *use = fname[0] == '\0' ? FILENAME_DEFAULT : fname;
    Student *newArr = load_from_file(students, count, use);
    printf("Load complete. %d records loaded.\n", *count);
    return newArr;
}

/* Menu function pointer type (uniform signature) */
typedef void (*MenuAction)(Student **, int *);

/* Wrapper functions matching MenuAction signature */

/* wrapper for add (matches MenuAction) */
static void menu_add(Student **students_ptr, int *count_ptr) {
    *students_ptr = add_student(*students_ptr, count_ptr);
}

/* wrapper for delete */
static void menu_delete(Student **students_ptr, int *count_ptr) {
    *students_ptr = delete_student(*students_ptr, count_ptr);
}

/* wrapper for update */
static void menu_update(Student **students_ptr, int *count_ptr) {
    (void)students_ptr; (void)count_ptr; /* unused params for signature */
    update_student(*students_ptr, *count_ptr);
}

/* wrapper for display */
static void menu_display(Student **students_ptr, int *count_ptr) {
    (void)students_ptr;
    display_students(*students_ptr, *count_ptr);
}

/* wrapper for search */
static void menu_search(Student **students_ptr, int *count_ptr) {
    (void)students_ptr;
    search_by_name(*students_ptr, *count_ptr);
}

/* wrapper for sort */
static void menu_sort(Student **students_ptr, int *count_ptr) {
    (void)students_ptr;
    sort_by_name(*students_ptr, *count_ptr);
}

/* wrapper for manual save */
static void menu_save(Student **students_ptr, int *count_ptr) {
    (void)students_ptr;
    manual_save(*students_ptr, *count_ptr);
}

/* wrapper for manual load */
static void menu_load(Student **students_ptr, int *count_ptr) {
    *students_ptr = manual_load(*students_ptr, count_ptr);
}

/* Main program */
int main(void) {
    Student *students = NULL;
    int count = 0;

    /* load at start from default file if exists */
    students = load_from_file(students, &count, FILENAME_DEFAULT);
    if (count > 0) {
        printf("Loaded %d records from %s on start.\n", count, FILENAME_DEFAULT);
    }

    /* prepare menu actions (1..9 mapped to array indexes 0..8) */
    MenuAction actions[] = {
        menu_add,      /* 1 */
        menu_delete,   /* 2 */
        menu_update,   /* 3 */
        menu_display,  /* 4 */
        menu_search,   /* 5 */
        menu_sort,     /* 6 */
        menu_save,     /* 7 */
        menu_load      /* 8 */
    };
    const int ACTION_COUNT = (int)(sizeof(actions) / sizeof(actions[0]));

    char line[LINE_BUF];

    while (1) {
        printf("\n=== STUDENT MANAGEMENT MENU ===\n");
        printf("1 Add student\n");          /* add + autosave */
        printf("2 Delete student\n");       /* delete + autosave */
        printf("3 Update student\n");       /* update + autosave */
        printf("4 Display students\n");
        printf("5 Search by name\n");
        printf("6 Sort by name\n");        /* sort + autosave */
        printf("7 Save to file (manual)\n");
        printf("8 Load from file (manual)\n");
        printf("9 Exit\n");
        printf("Choice: ");

        if (!read_line(line, LINE_BUF)) break;
        int choice;
        if (!parse_int(line, &choice)) {
            printf("Invalid input.\n");
            continue;
        }

        if (choice >= 1 && choice <= ACTION_COUNT) {
            /* call menu action (array index is choice-1) */
            actions[choice - 1](&students, &count);

            /* auto-save after update operation (#3) is handled inside update_student
               and add/delete already call autosave; sort_by_name also autosaves */
            /* No extra autosave needed here for those actions */
            continue;
        }

        if (choice == 9) {
            printf("Exiting program.\n");
            break;
        }

        printf("Invalid choice.\n");
    }

    /* final save before exit */
    autosave_if_needed(students, count);

    free(students);
    return 0;
}
