#include <stdio.h>
#include <unistd.h>
#include "cpu.h"

int cpu_read_raw(cpu_raw_t *raw)
{
    FILE *fd = fopen("/proc/stat", "r");
    if (fd == NULL) {
        perror("fopen /proc/stat");
        return -1;
    }

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    // cpu  123456 789 234567 9876543 12345 0 6789 0 0 0
    int n = fscanf(fd, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
                    &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    fclose(fd);

    if (n != 8) {
        fprintf(stderr, "cpu_read_raw: wrong format in /proc/stat :C\n");
        return -1;
    }

    raw->total = user + nice + system + idle + iowait + irq + softirq + steal;
    raw->idle  = idle + iowait;

    return 0;
}

double cpu_get_usage(void)
{
    cpu_raw_t first, second;

    if (cpu_read_raw(&first) != 0)
        return 0.0;

    sleep(1);

    if (cpu_read_raw(&second) != 0)
        return 0.0;

    unsigned long long delta_total = second.total - first.total;
    unsigned long long delta_idle  = second.idle - first.idle;

    if (delta_total == 0)
        return 0.0;

    double usage_pct = 100.0 * (double)(delta_total - delta_idle) / (double)delta_total;
    return usage_pct;
}

#include "proc.h"

// Measures global CPU usage AND per-process CPU usage in a single
// 1-second window, so we only sleep once instead of once per measurement.
// Returns the global CPU usage percentage, and fills cpu_percent
// for every process in procs.
double measure_all_cpu_usage(process_list_t *procs)
{
    cpu_raw_t cpu_before, cpu_after;
    static unsigned long long proc_before[MAX_PROCESSES];
    static unsigned long long proc_after[MAX_PROCESSES];

    cpu_read_raw(&cpu_before);
    for (int i = 0; i < procs->count; i++)
    {
        if (proc_read_cpu_raw(procs->list[i].pid, &proc_before[i]) != 0)
            proc_before[i] = 0; // process gone, will just show 0%
    }

    sleep(1);

    cpu_read_raw(&cpu_after);
    for (int i = 0; i < procs->count; i++)
    {
        if (proc_read_cpu_raw(procs->list[i].pid, &proc_after[i]) != 0)
            proc_after[i] = proc_before[i]; // exited: delta will be 0
    }

    unsigned long long delta_total = cpu_after.total - cpu_before.total;
    unsigned long long delta_idle = cpu_after.idle - cpu_before.idle;

    double global_usage = 0.0;
    if (delta_total > 0)
    {
        global_usage = 100.0 * (double)(delta_total - delta_idle) / (double)delta_total;
    }

    for (int i = 0; i < procs->count; i++)
    {
        unsigned long long delta_proc = proc_after[i] - proc_before[i];
        procs->list[i].cpu_percent = (delta_total > 0)
            ? 100.0 * (double)delta_proc / (double)delta_total
            : 0.0;
    }

    return global_usage;
}