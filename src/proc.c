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

    snprintf(line, sizeof(line), "%-8s %-12.12s %-5s %-10s", "PID", "NAME", "CPU%", "RSS (Mo)");
    print_bordered(line, WINDOW_WIDTH);

    int limit = (top_n < procs->count) ? top_n : procs->count;

    for (int i = 0; i < limit; i++)
    {
        const process_t *p = &procs->list[i];
        double rss_mo = p->vmrss_kb / 1024.0;

            snprintf(line, sizeof(line), "%-8d %-12.12s %-5.1f %-10.1f",
                p->pid, p->name, p->cpu_percent, rss_mo);
        print_bordered(line, WINDOW_WIDTH);
    }
}

int proc_read_cpu_raw(int pid, unsigned long long *cpu_time)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) 
        return -1; // process may have exited

    char buf[512];
    if (fgets(buf, sizeof(buf), fp) == NULL) 
    {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    // the process name (field 2) is in parentheses and can contain
    // spaces or even parentheses itself, so we must not just split
    // on spaces blindly. We find the LAST ')' to safely skip past
    // the name, then parse the fixed-position fields that follow.
    char *after_name = strrchr(buf, ')');
    if (after_name == NULL)
        return -1;
    after_name++; // move past the ')'

    unsigned long long utime, stime;

    // after the name, fields in order are:
    // state ppid pgrp session tty_nr tpgid flags minflt cminflt majflt cmajflt utime stime
    // that's 11 fields to skip (state through cmajflt), then utime and stime
    int n = sscanf(after_name,
        "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %llu %llu",
        &utime, &stime);

    if (n != 2) {
        return -1;
    }

    *cpu_time = utime + stime;
    return 0;
}

// Sorts by cpu_percent descending (biggest CPU usage first).
static int compare_by_cpu(const void *a, const void *b)
{
    const process_t *pa = (const process_t *)a;
    const process_t *pb = (const process_t *)b;

    // cpu_percent is a double, so we can't just subtract and cast to int
    // like we did for RSS (a double can hold values like 0.3 that would
    // round to 0 and make the sort incorrect). We compare directly instead.
    if (pb->cpu_percent > pa->cpu_percent) return 1;
    if (pb->cpu_percent < pa->cpu_percent) return -1;
    return 0;
}

void proc_list_sort_by_cpu(process_list_t *procs)
{
    qsort(procs->list, procs->count, sizeof(process_t), compare_by_cpu);
}