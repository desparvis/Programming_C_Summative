#include <stdio.h>
#include <stdlib.h>

/* function pointer types */
typedef int (*intOperation)(int*, int);
typedef float (*floatOperation)(int*, int);

/* sum of all elements */
int sum(int *data, int size) {
    int s = 0;
    for (int i = 0; i < size; i++) {
        s += data[i];
    }
    return s;
}

/* average of elements */
float average(int *data, int size) {
    if (size == 0) return 0.0f;
    return (float)sum(data, size) / (float)size;
}

/* minimum value */
int minimum(int *data, int size) {
    int m = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] < m) {
            m = data[i];
        }
    }
    return m;
}

/* maximum value */
int maximum(int *data, int size) {
    int m = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] > m) {
            m = data[i];
        }
    }
    return m;
}

/* sort ascending */
void sortAsc(int *data, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                int t = data[j];
                data[j] = data[j + 1];
                data[j + 1] = t;
            }
        }
    }
}

/* sort descending */
void sortDesc(int *data, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] < data[j + 1]) {
                int t = data[j];
                data[j] = data[j + 1];
                data[j + 1] = t;
            }
        }
    }
}

/* search for value */
int searchVal(int *data, int size, int v) {
    for (int i = 0; i < size; i++) {
        if (data[i] == v) {
            return i;
        }
    }
    return -1;
}

/* save dataset */
void saveFile(int *data, int size) {
    char name[260];
    printf("Enter filename to save: ");
    if (scanf("%259s", name) != 1) return;
    FILE *f = fopen(name, "w");
    if (!f) {
        printf("Cannot open file.\n");
        return;
    }
    for (int i = 0; i < size; i++) {
        fprintf(f, "%d\n", data[i]);
    }
    fclose(f);
    printf("Saved %d values.\n", size);
}

/* load dataset */
int* loadFile(int *data, int *size) {
    char name[260];
    printf("Enter filename to load: ");
    if (scanf("%259s", name) != 1) return data;
    FILE *f = fopen(name, "r");
    if (!f) {
        printf("Cannot open file.\n");
        return data;
    }

    free(data);
    data = NULL;
    *size = 0;

    int v;
    while (fscanf(f, "%d", &v) == 1) {
        int newSize = *size + 1;
        int *tmp = realloc(data, newSize * sizeof(int));
        if (!tmp) {
            printf("Memory allocation error.\n");
            fclose(f);
            free(data);
            *size = 0;
            return NULL;
        }
        data = tmp;
        data[newSize - 1] = v;
        *size = newSize;
    }
    fclose(f);
    printf("Loaded %d values.\n", *size);
    return data;
}

/* run operation using function pointers */
void runOp(int *data, int size) {
    if (size == 0) {
        printf("Dataset is empty.\n");
        return;
    }
    int c;
    printf("\nChoose operation: 1 sum 2 average 3 min 4 max\nChoice: ");
    if (scanf("%d", &c) != 1) return;

    if (c == 1) {
        intOperation op = sum;
        printf("Sum = %d\n", op(data, size));
    } else if (c == 2) {
        floatOperation op = average;
        printf("Average = %.2f\n", op(data, size));
    } else if (c == 3) {
        intOperation op = minimum;
        printf("Minimum = %d\n", op(data, size));
    } else if (c == 4) {
        intOperation op = maximum;
        printf("Maximum = %d\n", op(data, size));
    } else {
        printf("Invalid option.\n");
    }
}

/* add a number */
int* addNum(int *data, int *size) {
    int v;
    printf("Enter number: ");
    if (scanf("%d", &v) != 1) return data;

    int newSize = *size + 1;
    int *tmp = realloc(data, newSize * sizeof(int));
    if (tmp) {
        data = tmp;
        data[newSize - 1] = v;
        *size = newSize;
    }
    return data;
}

/* delete a number by index */
int* delNum(int *data, int *size) {
    if (*size == 0) {
        printf("Dataset is empty.\n");
        return data;
    }
    int idx;
    printf("Enter index to delete: ");
    if (scanf("%d", &idx) != 1) return data;
    if (idx < 0 || idx >= *size) {
        printf("Invalid index.\n");
        return data;
    }

    for (int i = idx; i < *size - 1; i++) {
        data[i] = data[i + 1];
    }

    int newSize = *size - 1;
    if (newSize == 0) {
        free(data);
        *size = 0;
        return NULL;
    }
    int *tmp = realloc(data, newSize * sizeof(int));
    if (tmp) data = tmp;
    *size = newSize;
    return data;
}

/* update a number by index */
void updateNum(int *data, int size) {
    if (size == 0) {
        printf("Dataset is empty.\n");
        return;
    }
    int idx, val;
    printf("Enter index to update: ");
    if (scanf("%d", &idx) != 1) return;
    if (idx < 0 || idx >= size) {
        printf("Invalid index.\n");
        return;
    }
    printf("Enter new value: ");
    if (scanf("%d", &val) != 1) return;
    data[idx] = val;
    printf("Updated index %d successfully.\n", idx);
}

/* display dataset */
void show(int *data, int size) {
    if (size == 0) {
        printf("Dataset is empty.\n");
        return;
    }
    printf("Current dataset: ");
    for (int i = 0; i < size; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
}

int main() {
    int *data = NULL;
    int size = 0;
    int c;

    while (1) {
        printf("\n=== DYNAMIC DATA ENGINE ===\n");
        printf("1 Add number\n");
        printf("2 Delete number\n");
        printf("3 Update number\n");
        printf("4 Show dataset\n");
        printf("5 Run operation (sum/avg/min/max)\n");
        printf("6 Sort ascending\n");
        printf("7 Sort descending\n");
        printf("8 Search value\n");
        printf("9 Save to file\n");
        printf("10 Load from file\n");
        printf("11 Exit\n");
        printf("Choice: ");

        if (scanf("%d", &c) != 1) {
            printf("Invalid input.\n");
            break;
        }

        if (c == 1) data = addNum(data, &size);
        else if (c == 2) data = delNum(data, &size);
        else if (c == 3) updateNum(data, size);
        else if (c == 4) show(data, size);
        else if (c == 5) runOp(data, size);
        else if (c == 6) {
            if (size > 1) {
                sortAsc(data, size);
                printf("Dataset sorted ascending.\n");
            } else printf("Not enough elements to sort.\n");
        }
        else if (c == 7) {
            if (size > 1) {
                sortDesc(data, size);
                printf("Dataset sorted descending.\n");
            } else printf("Not enough elements to sort.\n");
        }
        else if (c == 8) {
            if (size == 0) printf("Dataset empty.\n");
            else {
                int val;
                printf("Enter value to search: ");
                if (scanf("%d", &val) == 1) {
                    int idx = searchVal(data, size, val);
                    if (idx == -1) printf("Value not found.\n");
                    else printf("Value found at index %d.\n", idx);
                }
            }
        }
        else if (c == 9) saveFile(data, size);
        else if (c == 10) data = loadFile(data, &size);
        else if (c == 11) break;
        else printf("Invalid choice.\n");
    }

    free(data);
    return 0;
}
