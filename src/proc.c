#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "proc.h"
#include "ui.h"

// Checks if a string contains only digits (so it can be a PID).
static int is_pid(const char *name)
{
    if (*name == '\0')
        return 0; // empty string is not a valid pid

    for (const char *p = name; *p != '\0'; p++) 
    {
        if (!isdigit((unsigned char)*p))
            return 0; 
    }

    return 1;
}

// reads /proc/[pid]/status and fills name, statem vmrss_kb
// return 0 if success and -1 if error
// (process may have exited between the readdir() and this call)
static int read_process_status(int pid, process_t *proc)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *fd = fopen(path, "r");
    if (fd == NULL)
        return -1;

    proc->pid = pid;
    proc->name[0] = '\0';
    proc->state = '?';
    proc->vmrss_kb = 0;

    char line[256];
    while (fgets(line, sizeof(line), fd) != NULL)
    {
        if (strncmp(line, "Name:", 5) == 0) 
            sscanf(line, "Name: %63s", proc->name);
        else if (strncmp(line, "State:", 6) == 0) 
            sscanf(line, "State: %c", &proc->state);
        else if (strncmp(line, "VmRSS:", 6) == 0) 
            sscanf(line, "VmRSS: %ld kB", &proc->vmrss_kb);
    }

    fclose(fd);
    return 0;
}

int proc_list_get(process_list_t *procs)
{
    procs->count = 0;

    DIR *dir = opendir("/proc");
    if (dir == NULL)
    {
        perror("opendir /proc");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!is_pid(entry->d_name))
            continue; // not a process folder (ex: "net", "self", "stat"...)

        if (procs->count >= MAX_PROCESSES)
            break; // fixed-size array is full, stop here for now

        int pid = atoi(entry->d_name);
        process_t proc;

        // The process may have exited since readdir() listed it,
        // so a failed read is not a fatal error, just skip it.
        if (read_process_status(pid, &proc) == 0)
        {
            procs->list[procs->count] = proc;
            procs->count++;
        }
    }

    closedir(dir);
    return 0;
}

void proc_list_print(const process_list_t *procs) {
    printf("%-8s %-20s %-6s %10s\n", "PID", "NAME", "STATE", "RSS (kB)");

    for (int i = 0; i < procs->count; i++) {
        const process_t *p = &procs->list[i];
        printf("%-8d %-20s %-6c %10ld\n", p->pid, p->name, p->state, p->vmrss_kb);
    }
}

// Comparison function used by qsort.
// Sorts by vmrss_kb descending (biggest memory usage first).
static int compare_by_rss(const void *a, const void *b)
{
    const process_t *pa = (const process_t *)a;
    const process_t *pb = (const process_t *)b;

    // descending order: if pb uses more RAM, it should come first
    return (int)(pb->vmrss_kb - pa->vmrss_kb);
}

void proc_list_sort_by_rss(process_list_t *procs)
{
    qsort(procs->list, procs->count, sizeof(process_t), compare_by_rss);
}

void proc_list_print_top(const process_list_t *procs, int top_n)
{
    char line[128];

    snprintf(line, sizeof(line), "%-8s %-12.12s %-6s %-8s", "PID", "NAME", "STATE", "RSS (Mo)");
    print_bordered(line, WINDOW_WIDTH);

    int limit = (top_n < procs->count) ? top_n : procs->count;

    for (int i = 0; i < limit; i++)
    {
        const process_t *p = &procs->list[i];
        double rss_mo = p->vmrss_kb / 1024.0;

        snprintf(line, sizeof(line), "%-8d %-12.12s %-6c %-8.1f",
                 p->pid, p->name, p->state, rss_mo);
        print_bordered(line, WINDOW_WIDTH);
    }
}
