#ifndef PROC_H
#define PROC_H

#define MAX_PROCESSES 1024
#define PROC_NAME_LEN 64

typedef struct {
    int pid;
    char name[PROC_NAME_LEN];
    char state;        // R, S, Z, etc.
    long vmrss_kb;      // physical memory used (RSS)
} process_t;

typedef struct {
    process_t list[MAX_PROCESSES];
    int count;
} process_list_t;

// scans /proc, fills procs with every running process found
// returns 0 on success, -1 on error
int proc_list_get(process_list_t *procs);

// prints the process list (basic table)
void proc_list_print(const process_list_t *procs);

void proc_list_sort_by_rss(process_list_t *procs);
void proc_list_print_top(const process_list_t *procs, int top_n);

#endif